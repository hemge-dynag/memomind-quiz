// UI strings + built-in general-knowledge question bank for the quiz phone
// plugin. Locale follows the system language (navigator.language) — French is
// the default. Supported: fr, en, es, it, de, zh-CN.

const QUESTIONS = {
  fr: [
    { q: 'Quelle est la capitale de l’Australie ?', options: ['Sydney', 'Canberra', 'Melbourne', 'Perth'], correct: 1 },
    { q: 'Combien de continents y a-t-il sur Terre ?', options: ['5', '6', '7', '8'], correct: 2 },
    { q: 'Quel est le plus grand océan du monde ?', options: ['Atlantique', 'Indien', 'Arctique', 'Pacifique'], correct: 3 },
    { q: 'Qui a peint la Joconde ?', options: ['Van Gogh', 'Léonard de Vinci', 'Picasso', 'Monet'], correct: 1 },
    { q: 'Quel élément a le symbole chimique O ?', options: ['Or', 'Oxygène', 'Osmium', 'Argent'], correct: 1 },
    { q: 'En quelle année l’homme a-t-il marché sur la Lune ?', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: 'Quelle planète est la plus proche du Soleil ?', options: ['Vénus', 'Terre', 'Mercure', 'Mars'], correct: 2 },
    { q: 'Combien de cordes a une guitare classique ?', options: ['4', '5', '6', '7'], correct: 2 },
  ],
  en: [
    { q: 'What is the capital of Australia?', options: ['Sydney', 'Canberra', 'Melbourne', 'Perth'], correct: 1 },
    { q: 'How many continents are there on Earth?', options: ['5', '6', '7', '8'], correct: 2 },
    { q: 'What is the largest ocean in the world?', options: ['Atlantic', 'Indian', 'Arctic', 'Pacific'], correct: 3 },
    { q: 'Who painted the Mona Lisa?', options: ['Van Gogh', 'Leonardo da Vinci', 'Picasso', 'Monet'], correct: 1 },
    { q: 'Which element has the chemical symbol O?', options: ['Gold', 'Oxygen', 'Osmium', 'Silver'], correct: 1 },
    { q: 'In what year did humans first walk on the Moon?', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: 'Which planet is closest to the Sun?', options: ['Venus', 'Earth', 'Mercury', 'Mars'], correct: 2 },
    { q: 'How many strings does a classical guitar have?', options: ['4', '5', '6', '7'], correct: 2 },
  ],
  es: [
    { q: '¿Cuál es la capital de Australia?', options: ['Sídney', 'Canberra', 'Melbourne', 'Perth'], correct: 1 },
    { q: '¿Cuántos continentes hay en la Tierra?', options: ['5', '6', '7', '8'], correct: 2 },
    { q: '¿Cuál es el océano más grande del mundo?', options: ['Atlántico', 'Índico', 'Ártico', 'Pacífico'], correct: 3 },
    { q: '¿Quién pintó la Mona Lisa?', options: ['Van Gogh', 'Leonardo da Vinci', 'Picasso', 'Monet'], correct: 1 },
    { q: '¿Qué elemento tiene el símbolo químico O?', options: ['Oro', 'Oxígeno', 'Osmio', 'Plata'], correct: 1 },
    { q: '¿En qué año caminó el ser humano por primera vez en la Luna?', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: '¿Qué planeta está más cerca del Sol?', options: ['Venus', 'Tierra', 'Mercurio', 'Marte'], correct: 2 },
    { q: '¿Cuántas cuerdas tiene una guitarra clásica?', options: ['4', '5', '6', '7'], correct: 2 },
  ],
  it: [
    { q: 'Qual è la capitale dell’Australia?', options: ['Sydney', 'Canberra', 'Melbourne', 'Perth'], correct: 1 },
    { q: 'Quanti continenti ci sono sulla Terra?', options: ['5', '6', '7', '8'], correct: 2 },
    { q: 'Qual è l’oceano più grande del mondo?', options: ['Atlantico', 'Indiano', 'Artico', 'Pacifico'], correct: 3 },
    { q: 'Chi ha dipinto la Gioconda?', options: ['Van Gogh', 'Leonardo da Vinci', 'Picasso', 'Monet'], correct: 1 },
    { q: 'Quale elemento ha il simbolo chimico O?', options: ['Oro', 'Ossigeno', 'Osmio', 'Argento'], correct: 1 },
    { q: 'In che anno l’uomo camminò sulla Luna?', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: 'Quale pianeta è il più vicino al Sole?', options: ['Venere', 'Terra', 'Mercurio', 'Marte'], correct: 2 },
    { q: 'Quante corde ha una chitarra classica?', options: ['4', '5', '6', '7'], correct: 2 },
  ],
  de: [
    { q: 'Was ist die Hauptstadt von Australien?', options: ['Sydney', 'Canberra', 'Melbourne', 'Perth'], correct: 1 },
    { q: 'Wie viele Kontinente gibt es auf der Erde?', options: ['5', '6', '7', '8'], correct: 2 },
    { q: 'Welcher ist der größte Ozean der Welt?', options: ['Atlantik', 'Indischer Ozean', 'Arktischer Ozean', 'Pazifik'], correct: 3 },
    { q: 'Wer malte die Mona Lisa?', options: ['Van Gogh', 'Leonardo da Vinci', 'Picasso', 'Monet'], correct: 1 },
    { q: 'Welches Element hat das chemische Symbol O?', options: ['Gold', 'Sauerstoff', 'Osmium', 'Silber'], correct: 1 },
    { q: 'In welchem Jahr betrat der Mensch erstmals den Mond?', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: 'Welcher Planet ist der Sonne am nächsten?', options: ['Venus', 'Erde', 'Merkur', 'Mars'], correct: 2 },
    { q: 'Wie viele Saiten hat eine klassische Gitarre?', options: ['4', '5', '6', '7'], correct: 2 },
  ],
  zh: [
    { q: '澳大利亚的首都是哪里？', options: ['悉尼', '堪培拉', '墨尔本', '珀斯'], correct: 1 },
    { q: '地球上有几个大洲？', options: ['5', '6', '7', '8'], correct: 2 },
    { q: '世界上最大的海洋是？', options: ['大西洋', '印度洋', '北冰洋', '太平洋'], correct: 3 },
    { q: '谁画了《蒙娜丽莎》？', options: ['梵高', '列奥纳多·达·芬奇', '毕加索', '莫奈'], correct: 1 },
    { q: '哪个元素的化学符号是 O？', options: ['金', '氧', '锇', '银'], correct: 1 },
    { q: '人类哪一年首次登上月球？', options: ['1965', '1969', '1972', '1958'], correct: 1 },
    { q: '哪颗行星离太阳最近？', options: ['金星', '地球', '水星', '火星'], correct: 2 },
    { q: '古典吉他有几根弦？', options: ['4', '5', '6', '7'], correct: 2 },
  ],
};

const STRINGS = {
  fr: {
    appTitle: 'Quiz culture',
    statusStarting: 'Démarrage…',
    statusConnected: 'Lunettes connectées',
    statusDisconnected: 'Lunettes déconnectées',
    start: 'Démarrer le quiz',
    stop: 'Arrêter',
    glassesHint: 'Sur les lunettes : tête haut/bas = choisir une réponse, bouton = valider.',
    progress: (i, total) => `Question ${i} / ${total}`,
    scoreLine: (score, total) => `Score : ${score} / ${total}`,
    finished: (score, total) => `Terminé ! ${score} / ${total}`,
    statsGames: 'Parties jouées',
    statsBest: 'Meilleur score',
    noStats: 'Aucune partie pour l’instant.',
  },
  en: {
    appTitle: 'Trivia quiz',
    statusStarting: 'Starting…',
    statusConnected: 'Glasses connected',
    statusDisconnected: 'Glasses disconnected',
    start: 'Start quiz',
    stop: 'Stop',
    glassesHint: 'On the glasses: head up/down = choose an answer, button = confirm.',
    progress: (i, total) => `Question ${i} / ${total}`,
    scoreLine: (score, total) => `Score: ${score} / ${total}`,
    finished: (score, total) => `Done! ${score} / ${total}`,
    statsGames: 'Games played',
    statsBest: 'Best score',
    noStats: 'No game yet.',
  },
  es: {
    appTitle: 'Quiz de cultura',
    statusStarting: 'Iniciando…',
    statusConnected: 'Gafas conectadas',
    statusDisconnected: 'Gafas desconectadas',
    start: 'Empezar el quiz',
    stop: 'Detener',
    glassesHint: 'En las gafas: cabeza arriba/abajo = elegir respuesta, botón = confirmar.',
    progress: (i, total) => `Pregunta ${i} / ${total}`,
    scoreLine: (score, total) => `Puntuación: ${score} / ${total}`,
    finished: (score, total) => `¡Terminado! ${score} / ${total}`,
    statsGames: 'Partidas jugadas',
    statsBest: 'Mejor puntuación',
    noStats: 'Aún no hay partidas.',
  },
  it: {
    appTitle: 'Quiz di cultura',
    statusStarting: 'Avvio…',
    statusConnected: 'Occhiali connessi',
    statusDisconnected: 'Occhiali disconnessi',
    start: 'Inizia il quiz',
    stop: 'Ferma',
    glassesHint: 'Sugli occhiali: testa su/giù = scegli una risposta, pulsante = conferma.',
    progress: (i, total) => `Domanda ${i} / ${total}`,
    scoreLine: (score, total) => `Punteggio: ${score} / ${total}`,
    finished: (score, total) => `Finito! ${score} / ${total}`,
    statsGames: 'Partite giocate',
    statsBest: 'Miglior punteggio',
    noStats: 'Nessuna partita per ora.',
  },
  de: {
    appTitle: 'Allgemeinwissen-Quiz',
    statusStarting: 'Startet…',
    statusConnected: 'Brille verbunden',
    statusDisconnected: 'Brille getrennt',
    start: 'Quiz starten',
    stop: 'Stoppen',
    glassesHint: 'Auf der Brille: Kopf hoch/runter = Antwort wählen, Taste = bestätigen.',
    progress: (i, total) => `Frage ${i} / ${total}`,
    scoreLine: (score, total) => `Punktzahl: ${score} / ${total}`,
    finished: (score, total) => `Fertig! ${score} / ${total}`,
    statsGames: 'Gespielte Runden',
    statsBest: 'Bestwert',
    noStats: 'Noch keine Runde.',
  },
  zh: {
    appTitle: '知识问答',
    statusStarting: '启动中…',
    statusConnected: '眼镜已连接',
    statusDisconnected: '眼镜未连接',
    start: '开始问答',
    stop: '停止',
    glassesHint: '在眼镜上：抬头/低头 = 选择答案，按钮 = 确认。',
    progress: (i, total) => `第 ${i} / ${total} 题`,
    scoreLine: (score, total) => `得分：${score} / ${total}`,
    finished: (score, total) => `结束！${score} / ${total}`,
    statsGames: '已玩局数',
    statsBest: '最佳成绩',
    noStats: '暂无记录。',
  },
};

export function detectLocale() {
  const tag = (navigator.language || 'fr').toLowerCase();
  if (tag.startsWith('en')) return 'en';
  if (tag.startsWith('es')) return 'es';
  if (tag.startsWith('it')) return 'it';
  if (tag.startsWith('de')) return 'de';
  if (tag.startsWith('zh')) return 'zh';
  return 'fr';
}

export function getStrings(locale) {
  return STRINGS[locale] || STRINGS.fr;
}

export function getQuestions(locale) {
  return QUESTIONS[locale] || QUESTIONS.fr;
}
