#include "gm_plugin_lvgl_api.h"
#include "gm_plugin_libc.h"

/* Paired with PhoneSDK/examples/quiz. The phone pushes a multiple-choice
 * question; this plugin renders it with a carousel of answers. The wearer
 * navigates with head up/down and confirms with the primary button, then a
 * short correct/wrong banner is shown and the chosen index is sent back. */

#define QUIZ_CHANNEL UINT16_C(0x515A)  /* 'QZ': phone -> glasses */
#define EVENT_CHANNEL UINT16_C(0x5155) /* 'QE': glasses -> phone */
#define PROTOCOL_VERSION 1U

#define COMMAND_QUESTION 1U
#define COMMAND_SESSION_END 2U

#define EVENT_ANSWER UINT8_C(1)

#define MAX_QUESTION_BYTES 140U
#define MAX_OPTION_BYTES 40U
#define MAX_OPTIONS 4U
#define VISIBLE_ROWS 4U
#define CENTER_SLOT 1U

#define SCREEN_MARGIN 20
#define HINT_HEIGHT 28
#define QUESTION_HEIGHT 64

#define FEEDBACK_DURATION_MS 2000U

typedef struct {
    const gm_plugin_host_api_t *host;
    const gm_plugin_lvgl_api_t *ui;
    const gm_plugin_libc_extension_api_t *libc;
    gm_plugin_lvgl_obj_t *screen;
    gm_plugin_lvgl_obj_t *question_label;
    gm_plugin_lvgl_obj_t *selection_panel;
    gm_plugin_lvgl_obj_t *row_label[VISIBLE_ROWS];
    gm_plugin_lvgl_obj_t *hint_label;
    char question[MAX_QUESTION_BYTES + 1U];
    char options[MAX_OPTIONS][MAX_OPTION_BYTES + 1U];
    uint16_t option_count;
    uint16_t question_index;
    uint16_t question_total;
    uint8_t correct_index;
    uint16_t selected_index;
    bool has_question;
    bool feedback_active;
    bool feedback_correct;
    uint32_t feedback_elapsed_ms;
    int32_t row_height;
    uint8_t language; /* 0=fr 1=en 2=es 3=it 4=de 5=zh-CN */
} quiz_t;

/* Picks the string for the detected UI locale. */
#define L(fr_str, en_str, es_str, it_str, de_str, zh_str) \
    (quiz.language == 1U ? (en_str) : quiz.language == 2U ? (es_str) : \
     quiz.language == 3U ? (it_str) : quiz.language == 4U ? (de_str) : \
     quiz.language == 5U ? (zh_str) : (fr_str))

static quiz_t quiz;

static uint8_t quiz_detect_language(const char *locale)
{
    if (locale[0] == 'e' && locale[1] == 'n') return 1U;
    if (locale[0] == 'e' && locale[1] == 's') return 2U;
    if (locale[0] == 'i' && locale[1] == 't') return 3U;
    if (locale[0] == 'd' && locale[1] == 'e') return 4U;
    if (locale[0] == 'z' && locale[1] == 'h') return 5U;
    return 0U;
}

#define number gm_plugin_lvgl_style_number
#define color gm_plugin_lvgl_style_color

static void set_style(gm_plugin_lvgl_obj_t *object,
                       gm_plugin_lvgl_style_prop_t property,
                       gm_plugin_lvgl_style_value_t value)
{
    quiz.ui->style_set(object, property, value, GM_PLUGIN_LVGL_SELECTOR_MAIN);
}

static void style_panel(gm_plugin_lvgl_obj_t *object, uint8_t fill,
                         uint8_t border, uint8_t radius)
{
    set_style(object, GM_PLUGIN_LVGL_STYLE_BG_COLOR, color(fill));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BG_OPA, number(255));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_COLOR, color(0xA0));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_OPA, number(255));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_WIDTH, number(border));
    set_style(object, GM_PLUGIN_LVGL_STYLE_RADIUS, number(radius));
    quiz.ui->obj_clear_flag(object, GM_PLUGIN_LVGL_FLAG_SCROLLABLE);
}

static gm_plugin_lvgl_obj_t *make_label(gm_plugin_lvgl_obj_t *parent,
                                         const char *text, uint8_t shade)
{
    gm_plugin_lvgl_obj_t *label = quiz.ui->label_create(parent);
    if (label == 0) return 0;
    quiz.ui->label_set_text(label, text);
    quiz.ui->label_set_long_mode(label, GM_PLUGIN_LVGL_LABEL_WRAP);
    set_style(label, GM_PLUGIN_LVGL_STYLE_TEXT_COLOR, color(shade));
    set_style(label, GM_PLUGIN_LVGL_STYLE_TEXT_ALIGN,
              number(GM_PLUGIN_LVGL_TEXT_ALIGN_CENTER));
    return label;
}

static uint8_t opacity_for_distance(uint32_t distance)
{
    switch (distance) {
    case 0: return GM_PLUGIN_LVGL_OPA_COVER;
    case 1: return GM_PLUGIN_LVGL_OPA_70;
    default: return GM_PLUGIN_LVGL_OPA_30;
    }
}

static int32_t zoom_for_distance(uint32_t distance)
{
    switch (distance) {
    case 0: return 256;
    case 1: return 205;
    default: return 154;
    }
}

static void refresh_row(uint32_t slot, uint16_t option)
{
    bool is_center = (slot == CENTER_SLOT);
    uint32_t distance = is_center ? 0U
        : (slot > CENTER_SLOT ? slot - CENTER_SLOT : CENTER_SLOT - slot);
    gm_plugin_lvgl_style_value_t font_value = {0};

    quiz.ui->label_set_text(quiz.row_label[slot], quiz.options[option]);
    font_value.ptr = is_center ? quiz.ui->font_large : quiz.ui->font_default;
    set_style(quiz.row_label[slot], GM_PLUGIN_LVGL_STYLE_TEXT_FONT, font_value);
    set_style(quiz.row_label[slot], GM_PLUGIN_LVGL_STYLE_TEXT_COLOR, color(0xF0));
    set_style(quiz.row_label[slot], GM_PLUGIN_LVGL_STYLE_TEXT_OPA,
              number(opacity_for_distance(distance)));
    set_style(quiz.row_label[slot], GM_PLUGIN_LVGL_STYLE_TRANSFORM_ZOOM,
              number(zoom_for_distance(distance)));
}

static void format_progress(char *buffer, uint32_t size)
{
    /* "i/N" without printf, in the current locale-neutral form. */
    char number_buf[8];
    char total_buf[8];
    uint32_t i = 0U;
    uint32_t n = 0U;
    uint16_t value = (uint16_t)(quiz.question_index + 1U);
    uint16_t total = quiz.question_total;
    uint32_t pos = 0U;

    if (value == 0U) number_buf[i++] = '0';
    while (value > 0U && i < sizeof(number_buf) - 1U) {
        number_buf[i++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    if (total == 0U) total_buf[n++] = '0';
    while (total > 0U && n < sizeof(total_buf) - 1U) {
        total_buf[n++] = (char)('0' + (total % 10U));
        total /= 10U;
    }
    pos = 0U;
    while (i > 0U && pos < size - 1U) buffer[pos++] = number_buf[--i];
    if (pos < size - 1U) buffer[pos++] = '/';
    while (n > 0U && pos < size - 1U) buffer[pos++] = total_buf[--n];
    buffer[pos] = '\0';
}

static void refresh_display(void)
{
    uint32_t slot;
    char progress[16];
    char question_line[MAX_QUESTION_BYTES + 16U];
    uint32_t qlen;
    uint32_t pos;

    if (!quiz.has_question) {
        quiz.ui->label_set_text(quiz.question_label,
            L("Quiz", "Quiz", "Quiz", "Quiz", "Quiz", "问答"));
        for (slot = 0U; slot < VISIBLE_ROWS; slot++)
            quiz.ui->label_set_text(quiz.row_label[slot], "");
        quiz.ui->label_set_text(quiz.hint_label,
            L("En attente du telephone...", "Waiting for the phone...", "Esperando al teléfono...", "In attesa del telefono...", "Warte auf das Telefon...", "正在等待手机..."));
        return;
    }

    if (quiz.feedback_active) {
        quiz.ui->label_set_text(quiz.question_label,
            quiz.feedback_correct
                ? L("Correct !", "Correct!", "¡Correcto!", "Corretto!", "Richtig!", "正确！")
                : L("Faux", "Wrong", "Incorrecto", "Sbagliato", "Falsch", "错误"));
        quiz.ui->label_set_text(quiz.row_label[CENTER_SLOT],
                                quiz.options[quiz.correct_index]);
        for (slot = 0U; slot < VISIBLE_ROWS; slot++) {
            if (slot == CENTER_SLOT) continue;
            quiz.ui->label_set_text(quiz.row_label[slot], "");
        }
        quiz.ui->label_set_text(quiz.hint_label, "");
        return;
    }

    format_progress(progress, sizeof(progress));
    qlen = 0U;
    while (quiz.question[qlen] != '\0' && qlen < MAX_QUESTION_BYTES) qlen++;
    pos = 0U;
    while (progress[pos] != '\0' && pos < sizeof(question_line) - 3U) {
        question_line[pos] = progress[pos];
        pos++;
    }
    if (pos < sizeof(question_line) - 3U) question_line[pos++] = '\n';
    for (slot = 0U; slot < qlen && pos < sizeof(question_line) - 1U; slot++)
        question_line[pos++] = quiz.question[slot];
    question_line[pos] = '\0';
    quiz.ui->label_set_text(quiz.question_label, question_line);

    for (slot = 0U; slot < VISIBLE_ROWS; slot++) {
        int32_t slot_offset = (int32_t)slot - (int32_t)CENTER_SLOT;
        int32_t option = (int32_t)quiz.selected_index + slot_offset;
        if (option < 0 || option >= (int32_t)quiz.option_count) {
            quiz.ui->label_set_text(quiz.row_label[slot], "");
            continue;
        }
        refresh_row(slot, (uint16_t)option);
    }
    quiz.ui->label_set_text(quiz.hint_label,
        L("Tete: choisir   Bouton: valider",
          "Head: choose   Button: confirm",
          "Cabeza: elegir   Botón: confirmar",
          "Testa: scegli   Pulsante: conferma",
          "Kopf: wählen   Taste: bestätigen",
          "抬头/低头：选择   按钮：确认"));
}

static void move_selection(int32_t delta)
{
    int32_t next = (int32_t)quiz.selected_index + delta;
    if (next < 0) next = 0;
    if (next >= (int32_t)quiz.option_count) next = (int32_t)quiz.option_count - 1;
    quiz.selected_index = (uint16_t)next;
    refresh_display();
}

static void confirm_selection(void)
{
    uint8_t payload[5];
    if (!quiz.has_question || quiz.feedback_active) return;
    payload[0] = PROTOCOL_VERSION;
    payload[1] = EVENT_ANSWER;
    payload[2] = (uint8_t)quiz.selected_index;
    payload[3] = (quiz.selected_index == quiz.correct_index) ? 1U : 0U;
    payload[4] = 0U;
    (void)quiz.host->bt_send(EVENT_CHANNEL, payload, sizeof(payload));
    quiz.feedback_active = true;
    quiz.feedback_correct = (quiz.selected_index == quiz.correct_index);
    quiz.feedback_elapsed_ms = 0U;
    refresh_display();
}

static gm_plugin_result_t quiz_start(void *context)
{
    gm_plugin_display_info_t display;
    gm_plugin_lvgl_obj_t *root;
    char locale[GM_PLUGIN_LOCALE_TAG_MAX];
    int32_t list_area_height;
    uint32_t slot;
    (void)context;

    quiz.language = 0U;
    quiz.has_question = false;
    quiz.feedback_active = false;

    if (quiz.host->locale_get != 0 &&
        quiz.host->locale_get(locale) == GM_PLUGIN_OK)
        quiz.language = quiz_detect_language(locale);

    if (quiz.host->display_get_info(&display) != GM_PLUGIN_OK ||
        display.width <= SCREEN_MARGIN * 2U)
        return GM_PLUGIN_ESTATE;

    list_area_height = (int32_t)display.height - HINT_HEIGHT - QUESTION_HEIGHT - SCREEN_MARGIN;
    quiz.row_height = list_area_height / (int32_t)VISIBLE_ROWS;
    if (quiz.row_height < 22) return GM_PLUGIN_ESTATE;

    root = quiz.ui->root_get();
    if (root == 0) return GM_PLUGIN_ESTATE;
    quiz.ui->obj_clean(root);

    quiz.screen = quiz.ui->obj_create(root);
    if (quiz.screen == 0) return GM_PLUGIN_ENOMEM;
    quiz.ui->obj_set_size(quiz.screen, display.width, display.height);
    quiz.ui->obj_align(quiz.screen, GM_PLUGIN_LVGL_ALIGN_CENTER, 0, 0);
    style_panel(quiz.screen, 0x00, 0, 0);

    quiz.question_label = make_label(quiz.screen, "", 0xD0);
    if (quiz.question_label == 0) goto no_memory;
    quiz.ui->obj_set_size(quiz.question_label, display.width - SCREEN_MARGIN * 2U,
                           QUESTION_HEIGHT);
    quiz.ui->obj_align(quiz.question_label, GM_PLUGIN_LVGL_ALIGN_TOP_MID, 0, 4);

    quiz.selection_panel = quiz.ui->obj_create(quiz.screen);
    if (quiz.selection_panel == 0) goto no_memory;
    quiz.ui->obj_set_size(quiz.selection_panel, display.width - SCREEN_MARGIN,
                           quiz.row_height);
    quiz.ui->obj_align(quiz.selection_panel, GM_PLUGIN_LVGL_ALIGN_CENTER, 0,
                       QUESTION_HEIGHT / 2 - HINT_HEIGHT / 2);
    style_panel(quiz.selection_panel, 0x10, 2, 10);

    for (slot = 0U; slot < VISIBLE_ROWS; slot++) {
        int32_t offset_y = ((int32_t)slot - (int32_t)CENTER_SLOT) * quiz.row_height
                            + (QUESTION_HEIGHT / 2 - HINT_HEIGHT / 2);
        quiz.row_label[slot] = make_label(quiz.screen, "", 0x80);
        if (quiz.row_label[slot] == 0) goto no_memory;
        quiz.ui->label_set_long_mode(quiz.row_label[slot], GM_PLUGIN_LVGL_LABEL_DOT);
        quiz.ui->obj_set_size(quiz.row_label[slot], display.width - SCREEN_MARGIN * 2U,
                               quiz.row_height - 4);
        quiz.ui->obj_align(quiz.row_label[slot], GM_PLUGIN_LVGL_ALIGN_CENTER, 0, offset_y);
    }

    quiz.hint_label = make_label(quiz.screen, "", 0x70);
    if (quiz.hint_label == 0) goto no_memory;
    quiz.ui->obj_set_size(quiz.hint_label, display.width - SCREEN_MARGIN * 2U, HINT_HEIGHT);
    quiz.ui->obj_align(quiz.hint_label, GM_PLUGIN_LVGL_ALIGN_BOTTOM_MID, 0, -6);

    refresh_display();

    if (quiz.host->imu_enable(GM_PLUGIN_IMU_ENABLE_GESTURES) != GM_PLUGIN_OK)
        goto no_memory;
    return GM_PLUGIN_OK;

no_memory:
    quiz.ui->obj_clean(root);
    quiz.screen = 0;
    quiz.question_label = 0;
    quiz.selection_panel = 0;
    for (slot = 0U; slot < VISIBLE_ROWS; slot++) quiz.row_label[slot] = 0;
    quiz.hint_label = 0;
    return GM_PLUGIN_ENOMEM;
}

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static void handle_quiz_message(const uint8_t *data, uint32_t length)
{
    uint32_t offset;
    uint8_t qlen;
    uint8_t option_count;
    uint8_t i;

    if (length < 4U || data[0] != PROTOCOL_VERSION) return;

    if (data[1] == COMMAND_SESSION_END) {
        quiz.has_question = false;
        quiz.feedback_active = false;
        refresh_display();
        return;
    }
    if (data[1] != COMMAND_QUESTION) return;
    if (length < 8U) return;

    quiz.question_index = read_u16(&data[2]);
    quiz.question_total = read_u16(&data[4]);
    quiz.correct_index = data[6];
    qlen = data[7];
    offset = 8U;
    if (qlen > MAX_QUESTION_BYTES || offset + qlen > length) return;
    quiz.libc->memcpy(quiz.question, &data[offset], qlen);
    quiz.question[qlen] = '\0';
    offset += qlen;

    if (offset >= length) return;
    option_count = data[offset++];
    if (option_count > MAX_OPTIONS) option_count = MAX_OPTIONS;
    for (i = 0U; i < option_count; i++) {
        uint8_t olen;
        if (offset >= length) break;
        olen = data[offset++];
        if (olen > MAX_OPTION_BYTES || offset + olen > length) break;
        quiz.libc->memcpy(quiz.options[i], &data[offset], olen);
        quiz.options[i][olen] = '\0';
        offset += olen;
    }
    quiz.option_count = i;
    if (quiz.correct_index >= quiz.option_count) quiz.correct_index = 0U;
    quiz.selected_index = 0U;
    quiz.has_question = quiz.option_count > 0U;
    quiz.feedback_active = false;
    refresh_display();
}

static bool quiz_event(void *context, const gm_plugin_event_t *event)
{
    (void)context;
    if (event == 0) return false;

    if (event->type == GM_PLUGIN_EVENT_CONNECTION) {
        quiz.host->log("quiz: phone connected=%u",
                       (unsigned int)event->data.connection.connected);
        return true;
    }

    if (event->type == GM_PLUGIN_EVENT_BT_MESSAGE) {
        if (event->data.bt.channel != QUIZ_CHANNEL || event->data.bt.data == 0)
            return false;
        handle_quiz_message(event->data.bt.data, event->data.bt.length);
        return true;
    }

    if (!quiz.has_question || quiz.feedback_active) return false;

    if (event->type == GM_PLUGIN_EVENT_BUTTON &&
        event->data.button.button == GM_PLUGIN_BUTTON_PRIMARY &&
        event->data.button.action == GM_PLUGIN_BUTTON_ACTION_SINGLE) {
        confirm_selection();
        return true;
    }

    if (event->type == GM_PLUGIN_EVENT_IMU_GESTURE &&
        event->data.imu_gesture.active) {
        switch (event->data.imu_gesture.gesture) {
        case GM_PLUGIN_IMU_GESTURE_HEAD_RAISE:
            move_selection(-1);
            return true;
        case GM_PLUGIN_IMU_GESTURE_HEAD_LOWER:
            move_selection(1);
            return true;
        default:
            break;
        }
    }
    return false;
}

static void quiz_loop(void *context, uint32_t elapsed_ms)
{
    (void)context;
    if (!quiz.feedback_active) return;
    quiz.feedback_elapsed_ms += elapsed_ms;
    if (quiz.feedback_elapsed_ms >= FEEDBACK_DURATION_MS) {
        quiz.feedback_active = false;
        refresh_display();
    }
}

static void quiz_stop(void *context)
{
    gm_plugin_lvgl_obj_t *root;
    uint32_t slot;
    (void)context;
    root = quiz.ui->root_get();
    (void)quiz.host->imu_enable(GM_PLUGIN_IMU_ENABLE_NONE);
    if (root != 0) quiz.ui->obj_clean(root);
    quiz.screen = 0;
    quiz.question_label = 0;
    quiz.selection_panel = 0;
    for (slot = 0U; slot < VISIBLE_ROWS; slot++) quiz.row_label[slot] = 0;
    quiz.hint_label = 0;
    quiz.has_question = false;
    quiz.feedback_active = false;
}

gm_plugin_result_t gm_plugin_entry(const gm_plugin_host_api_t *host,
                                    gm_plugin_descriptor_t *plugin)
{
    const gm_plugin_capabilities_t required =
        GM_PLUGIN_CAP_BLUETOOTH | GM_PLUGIN_CAP_BUTTON | GM_PLUGIN_CAP_IMU_EVENTS;

    if (host == 0 || plugin == 0 || host->log == 0 || host->bt_send == 0 ||
        host->display_get_info == 0 || host->graphics.lvgl == 0 ||
        host->imu_enable == 0 ||
        !GM_PLUGIN_VERSION_COMPATIBLE(host->abi_version,
                                      GM_PLUGIN_ABI_MIN_VERSION) ||
        host->struct_size < GM_PLUGIN_HOST_API_MIN_SIZE ||
        plugin->struct_size < GM_PLUGIN_DESCRIPTOR_MIN_SIZE ||
        (host->capabilities & required) != required)
        return GM_PLUGIN_ENOTSUP;

    quiz.host = host;
    quiz.ui = host->graphics.lvgl;
    if (gm_plugin_libc_get(host, &quiz.libc) != GM_PLUGIN_OK)
        return GM_PLUGIN_ENOTSUP;
    if (quiz.ui->struct_size < GM_PLUGIN_LVGL_API_MIN_SIZE ||
        !GM_PLUGIN_VERSION_COMPATIBLE(quiz.ui->api_version,
                                      GM_PLUGIN_LVGL_API_MIN_VERSION))
        return GM_PLUGIN_EVERSION;

    plugin->abi_version = GM_PLUGIN_ABI_MIN_VERSION;
    plugin->context = &quiz;
    plugin->on_start = quiz_start;
    plugin->on_event = quiz_event;
    plugin->on_loop = quiz_loop;
    plugin->on_stop = quiz_stop;
    return GM_PLUGIN_OK;
}
