# MemoMind — Quiz culture (quiz)

Application **appairée** MemoMind : un quiz de culture générale joué sans les mains sur les lunettes, piloté depuis le téléphone.

## Composition

| Côté | Dossier | Artefact |
| --- | --- | --- |
| Lunettes (plugin natif C) | `glass/quiz/` | `builds/quiz.gmp` |
| Téléphone (plugin Web) | `phone/quiz/` | `builds/quiz-1.0.0.mmpkg` |

## Fonctionnement

- Le téléphone contient une banque de 8 questions de culture générale par langue (6 langues). Chaque partie mélange les questions.
- Les lunettes affichent une question à choix multiple (4 options) : tête haut/bas pour choisir la réponse, appui court sur le bouton pour valider. Un bandeau « Correct / Faux » s'affiche 2 s, puis la question suivante arrive.
- Le téléphone suit la progression et le score, puis met à jour les statistiques : nombre de parties jouées et meilleur score.
- Traduit en 6 langues (FR/EN/ES/IT/DE/ZH), banque de questions incluse, côté lunettes et côté téléphone.

## Protocole binaire

`QUIZ_CHANNEL = 0x515A` ('QZ', téléphone → lunettes), `EVENT_CHANNEL = 0x5155` ('QE', lunettes → téléphone), version 1.

- `COMMAND_QUESTION` = `[version, cmd, index_lo, index_hi, total_lo, total_hi, correct, qlen, question..., option_count, (olen, option...)*]`
- `COMMAND_SESSION_END` = `[version, cmd, 0, 0]`
- `EVENT_ANSWER` = `[version, event, chosen, correct, 0]`

## Build

Nécessite le SDK officiel MemoMind (`memomind-open/plugin-open-platform`). Déposer `glass/quiz/` dans `GlassSDK/examples/` et `phone/quiz/` dans `PhoneSDK/examples/`, puis lancer `python3 build.py`.

- Lunettes : `python3 build.py glass` → `.gmp`
- Téléphone : `python3 build.py web` → `.mmpkg`
