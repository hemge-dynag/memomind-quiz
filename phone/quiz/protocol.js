// Binary wire protocol shared with GlassSDK/examples/quiz/quiz.c.
// Keep both sides in sync if you change any layout or constant here.

export const QUIZ_CHANNEL = 0x515a; // 'QZ', phone -> glasses
export const EVENT_CHANNEL = 0x5155; // 'QE', glasses -> phone
export const PROTOCOL_VERSION = 1;

export const COMMAND_QUESTION = 1;
export const COMMAND_SESSION_END = 2;

export const EVENT_ANSWER = 1;

export const MAX_QUESTION_BYTES = 140;
export const MAX_OPTION_BYTES = 40;
export const MAX_OPTIONS = 4;

const encoder = new TextEncoder();
const decoder = new TextDecoder();

function toBoundedUtf8(text, maxBytes) {
  let bytes = encoder.encode(text);
  while (bytes.length > maxBytes) {
    bytes = encoder.encode([...decoder.decode(bytes)].slice(0, -1).join(''));
  }
  return bytes;
}

// [version, COMMAND_QUESTION, index_lo, index_hi, total_lo, total_hi, correct,
//  qlen, question..., option_count, (olen, option...)*]
export function encodeQuestion({ index, total, correct, question, options }) {
  const qBytes = toBoundedUtf8(question, MAX_QUESTION_BYTES);
  const limitedOptions = options.slice(0, MAX_OPTIONS);
  const optBytes = limitedOptions.map((opt) => toBoundedUtf8(opt, MAX_OPTION_BYTES));
  const optTotal = optBytes.reduce((sum, bytes) => sum + 1 + bytes.length, 0);
  const payload = new Uint8Array(8 + qBytes.length + 1 + optTotal);
  let offset = 0;
  payload[offset++] = PROTOCOL_VERSION;
  payload[offset++] = COMMAND_QUESTION;
  payload[offset++] = index & 0xff;
  payload[offset++] = (index >> 8) & 0xff;
  payload[offset++] = total & 0xff;
  payload[offset++] = (total >> 8) & 0xff;
  payload[offset++] = correct & 0xff;
  payload[offset++] = qBytes.length;
  payload.set(qBytes, offset);
  offset += qBytes.length;
  payload[offset++] = limitedOptions.length;
  optBytes.forEach((bytes) => {
    payload[offset++] = bytes.length;
    payload.set(bytes, offset);
    offset += bytes.length;
  });
  return payload;
}

export function encodeSessionEnd() {
  return new Uint8Array([PROTOCOL_VERSION, COMMAND_SESSION_END, 0, 0]);
}

// [version, EVENT_ANSWER, chosen, correct, 0]
export function decodeEvent(data) {
  if (!data || data.length < 5 || data[0] !== PROTOCOL_VERSION) return null;
  if (data[1] !== EVENT_ANSWER) return null;
  return { chosen: data[2], correct: data[3] === 1 };
}
