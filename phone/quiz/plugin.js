import { createGMPlugin } from './vendor/gm-plugin-web-sdk.esm.js';
import {
  EVENT_CHANNEL,
  QUIZ_CHANNEL,
  decodeEvent,
  encodeQuestion,
  encodeSessionEnd,
} from './protocol.js';
import { detectLocale, getQuestions, getStrings } from './translations.js';

const LOCALE = detectLocale();
const S = getStrings(LOCALE);
const BANK = getQuestions(LOCALE);

const STORAGE_KEY = 'quiz';

const gm = createGMPlugin();

const statusEl = document.querySelector('#status');
const startButton = document.querySelector('#start-button');
const stopButton = document.querySelector('#stop-button');
const sessionPanel = document.querySelector('#session-panel');
const progressEl = document.querySelector('#progress');
const scoreEl = document.querySelector('#score');
const noStatsEl = document.querySelector('#no-stats');

let connected = false;
let sessionActive = false;
let order = [];
let questionPos = 0;
let score = 0;
let correctSoFar = 0;
let advanceTimer = null;

function setStatus(text, state = '') {
  statusEl.textContent = text;
  statusEl.className = `status ${state}`.trim();
}

function applyStrings() {
  document.documentElement.lang = LOCALE;
  document.querySelector('#app-title').textContent = S.appTitle;
  startButton.textContent = S.start;
  stopButton.textContent = S.stop;
  document.querySelector('#glasses-hint').textContent = S.glassesHint;
  document.querySelector('#stat-games-label').textContent = S.statsGames;
  document.querySelector('#stat-best-label').textContent = S.statsBest;
  noStatsEl.textContent = S.noStats;
  setStatus(S.statusStarting);
}

function shuffle(array) {
  const copy = [...array];
  for (let i = copy.length - 1; i > 0; i -= 1) {
    const j = Math.floor(Math.random() * (i + 1));
    [copy[i], copy[j]] = [copy[j], copy[i]];
  }
  return copy;
}

async function loadStats() {
  const result = await gm.storage.get(STORAGE_KEY);
  return result.value || { games: 0, best: 0 };
}

async function renderStats() {
  const stats = await loadStats();
  document.querySelector('#stat-games').textContent = String(stats.games);
  document.querySelector('#stat-best').textContent = String(stats.best);
  noStatsEl.hidden = stats.games > 0;
}

async function saveResult(finalScore) {
  const stats = await loadStats();
  stats.games += 1;
  stats.best = Math.max(stats.best, finalScore);
  await gm.storage.set(STORAGE_KEY, stats);
  await renderStats();
}

function updateSession() {
  progressEl.textContent = S.progress(Math.min(questionPos + 1, order.length), order.length);
  scoreEl.textContent = S.scoreLine(correctSoFar, order.length);
}

async function sendQuestion() {
  const entry = order[questionPos];
  if (!entry) return;
  const question = BANK[entry % BANK.length];
  updateSession();
  try {
    await gm.plugin.sendMessage(QUIZ_CHANNEL, encodeQuestion({
      index: questionPos,
      total: order.length,
      correct: question.correct,
      question: question.q,
      options: question.options,
    }));
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
}

async function startQuiz() {
  if (BANK.length === 0) return;
  sessionActive = true;
  order = shuffle(BANK.map((_, i) => i));
  questionPos = 0;
  score = 0;
  correctSoFar = 0;
  startButton.hidden = true;
  stopButton.hidden = false;
  sessionPanel.hidden = false;
  setStatus(S.progress(1, order.length), 'ready');
  await sendQuestion();
}

async function stopQuiz() {
  sessionActive = false;
  if (advanceTimer) {
    clearTimeout(advanceTimer);
    advanceTimer = null;
  }
  try {
    await gm.plugin.sendMessage(QUIZ_CHANNEL, encodeSessionEnd());
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
  startButton.hidden = false;
  stopButton.hidden = true;
  sessionPanel.hidden = true;
  startButton.disabled = !connected;
  setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
}

async function onAnswer(decoded) {
  if (!sessionActive) return;
  if (decoded.correct) correctSoFar += 1;
  scoreEl.textContent = S.scoreLine(correctSoFar, order.length);
  questionPos += 1;

  if (questionPos >= order.length) {
    const finalScore = correctSoFar;
    await saveResult(finalScore);
    await gm.plugin.sendMessage(QUIZ_CHANNEL, encodeSessionEnd());
    setStatus(S.finished(finalScore, order.length), 'ready');
    sessionActive = false;
    startButton.hidden = false;
    stopButton.hidden = true;
    startButton.disabled = !connected;
    // Keep the session panel visible for a moment with the final score.
    progressEl.textContent = '';
    scoreEl.textContent = S.finished(finalScore, order.length);
    return;
  }

  updateSession();
  // Give the glasses time to show their 2s correct/wrong banner first.
  if (advanceTimer) clearTimeout(advanceTimer);
  advanceTimer = setTimeout(() => {
    advanceTimer = null;
    if (sessionActive) void sendQuestion();
  }, 2300);
}

startButton.addEventListener('click', () => void startQuiz());
stopButton.addEventListener('click', () => void stopQuiz());

const offMessages = gm.plugin.onMessage((message) => {
  if (message.channel !== EVENT_CHANNEL) return;
  const decoded = decodeEvent(message.data);
  if (!decoded) return;
  void onAnswer(decoded);
});

async function start() {
  try {
    await gm.ready();
    applyStrings();
    await renderStats();
    const info = await gm.device.getInfo();
    connected = Boolean(info.connected);
    setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
    startButton.disabled = !connected;
    await gm.device.subscribeEvents(['connection']);
    gm.device.onConnection((event) => {
      connected = Boolean(event.connected);
      setStatus(connected ? S.statusConnected : S.statusDisconnected, connected ? 'ready' : 'error');
      if (!connected && sessionActive) void stopQuiz();
      startButton.disabled = !connected || sessionActive;
    });
  } catch (error) {
    setStatus(error.message || String(error), 'error');
  }
}

window.addEventListener('pagehide', () => {
  offMessages();
  gm.close();
});

void start();
