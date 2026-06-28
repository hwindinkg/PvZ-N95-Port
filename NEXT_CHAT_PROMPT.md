# PROMPT FOR NEXT CHAT SESSION — PvZ N95 Port (1:1 Full Game)

## Copy-paste this prompt at the start of the next chat:

---

Ты продолжаешь работу над **ПОЛНЫМ ПОРТОМ 1:1** игры Plants vs. Zombies для Nokia N95 (Symbian S60 3rd FP1). Цель — не "похоже", а точный порт всей игры: меню, геймплей, растения, зомби, уровни, звуки — всё как в оригинале.

## Репозитории
- **Порт (текущий код)**: https://github.com/hwindinkg/PvZ-N95-Port (ветка main, push напрямую)
- **Upstream (оригинальный движок)**: https://github.com/wszqkzqk/PvZ-Portable
- **Whisk3D (эталон EGL/Symbian)**: https://github.com/Dante-Leoncini/Whisk3D/tree/symbian

## Главная задача
**Порт 1:1 всей игры PvZ** на Symbian S60 3rd FP1 (Nokia N95). Не аппроксимация, не "похоже" — точный порт. Сейчас сделано (session 5, commit f94ef93):
- Engine boot (EGL/GLES, landscape, 30 FPS)
- PAK VFS (3198 файлов, XOR 0xF7)
- Loading screen (progress bar + Click to Start) — **progress bar рендерится криво (полоской), починить**
- **ReanimLoader XML парсер ИСПРАВЛЕН** (session 4) — FindElement парсит ВСЕ ~48 треков
- **ReanimPlayer runtime** (session 4) — интерполяция + Draw, меню анимируется
- **IMAGE_REANIM_* mapping ИСПРАВЛЕН** (session 4) — спрайты меню грузятся
- **SystemFont ')' glyph ИСПРАВЛЕН** (session 4)
- **Purple screen + crash ИСПРАВЛЕНЫ** (session 5) — state machine re-entry + lazy image loading + TRAP + NULL checks. Было: LoadingCompleted() вызывался 36× (state=3 ставился ПОСЛЕ вызова, Leave на OOM → state не advancing → 35× GameSelector ctor → heap exhaustion → KERN-EXEC 3). Теперь: state=3 ДО вызова + TRAPD + images lazy-load в Draw вместо 14× ICL decode в ctor.
- **Lawn bg always drawn** (session 5) — IMAGE_BACKGROUND1 рисуется ПЕРВЫМ, reanim поверх. Нет фиолетового экрана.
- Заглушечное меню (лужайка + 10 rect кнопок) — **всё ещё ЗАМЕНИТЬ на 1:1** (reanim рендерится ПОД кнопками). gs_log.txt теперь содержит ВСЕ track names для маппинга.
- miniz (zlib) + d-pad input (курсор + клики)

## Что нужно портировать (ВЕСЬ upstream движок):

### Этап 1: Главное меню (1:1)
1. **Исправить ReanimLoader** — XML парсер `src/Sexy.TodLib/ReanimLoader.cpp` парсит только 1 трек (баг в FindTag: `</track>` находится как `<track>`). Должно найти ~48 треков.
2. **Загрузить reanim images** — треки с `<i>IMAGE_REANIM_SELECTORSCREEN_BG</i>` должны загрузить спрайты через ResourceManager (уже ищет `reanim/` prefix).
3. **Рендерить SelectorScreen** — GameSelector::Draw рисует все треки reanim (фон, надгробие, деревянные знаки, кнопки-спрайты, облака, цветы). Координаты 800x600 → 400x300 (×0.5).
4. **Убрать заглушечное меню** — удалить 10 rect кнопок, заменить на reanim спрайты с hit-testing.
5. **Порт upstream GameSelector.cpp 1:1** — конструктор (NewLawnButton со спрайтами), Draw (reanim), ButtonDepress (Adventure → zombie hand → PreNewGame).
6. **Порт upstream TitleScreen.cpp 1:1** — state machine (PopCap logo → partner logo → screen), SODROLLCAP анимация, "Click to Start" hyperlink.

### Этап 2: Геймплей (1:1)
7. **Порт Board.cpp 1:1** — игровое поле, сетка 9x5, солнце, растения, зомби, газонокосилки. Upstream: 6364 строки.
8. **Порт Plant.cpp 1:1** — поведение растений (стрельба, посадка, анимация). Upstream: все растения.
9. **Порт Zombie.cpp 1:1** — поведение зомби (движение, атака, смерть). Upstream: все типы зомби.
10. **Порт Reanimator.cpp 1:1** — система анимации (interpolation transforms over time). Upstream: 1501 строка. Нужна для анимации растений/зомби/UI.
11. **Порт EffectSystem.cpp 1:1** — менеджер reanimations + particles. Upstream: 541 строка.
12. **Порт TodParticle.cpp 1:1** — система частиц (эффекты стрельбы, взрывы, искры). Upstream: 1290 строк.
13. **Порт SeedChooserScreen.cpp 1:1** — выбор семян перед уровнем. Upstream: 1158 строк.
14. **Порт Challenge.cpp 1:1** — мини-игры, пазлы, выживание. Upstream: весь файл.
15. **Порт CutScene.cpp 1:1** — катсцены (начало уровня, окончание). 
16. **Порт Coin.cpp, Projectile.cpp, LawnMower.cpp, GridItem.cpp 1:1** — игровые объекты.

### Этап 3: Диалоги и подэкраны (1:1)
17. **Порт StoreScreen.cpp 1:1** — магазин (1187 строк upstream).
18. **Порт AlmanacDialog.cpp 1:1** — альманах растений/зомби (718 строк).
19. **Порт ZenGarden.cpp 1:1** — дзен-сад.
20. **Порт NewOptionsDialog.cpp 1:1** — настройки.
21. **Порт AwardScreen.cpp 1:1** — экран наград.
22. **Порт CreditScreen.cpp 1:1** — титры.

### Этап 4: Системы (1:1)
23. **Порт ImageFont.cpp 1:1** — настоящие PvZ шрифты (загрузка .dat font description files). Upstream: 1748 строк. Заменит SystemFont.
24. **Порт SaveGame.cpp 1:1** — сохранения (RFile serialization).
25. **Порт ProfileMgr.cpp 1:1** — профили игроков.
26. **Порт Music.cpp 1:1** — музыка (CMdaAudioPlayerUtility или stub).
27. **Порт SoundManager 1:1** — звуки (Symbian audio или stub на первое время).

### Этап 5: Полировка
28. **IMAGE_BACKGROUND1 tiling** — lawn background для gameplay (1400x600 → ≤2048 текстуры).
29. **149 REANIM_* assets** — исправить naming/packing mismatch.
30. **51 NULL IMAGE_/FONT_/SOUND_ symbols** — загрузить недостающие ресурсы.

## PAK файл (main.pak, 45MB, 3198 файлов)
Содержит ВСЕ нужные assets:
- `reanim/*.reanim` — 290 XML файлов анимаций (architecture-independent)
- `reanim/SelectorScreen_*.png/jpg` — спрайты меню
- `reanim/SelectorScreen.reanim` — XML меню (328KB, 48 tracks)
- `images/background1.jpg` — фон газона (171KB)
- `images/PvZ_Logo_.png` — логотип с альфа
- `images/LoadBar_*.png` — полоса загрузки
- `images/SelectorScreen_*.png` — кнопки опций/помощи/выхода
- `.reanim.compiled` файлы — НЕ ИСПОЛЬЗОВАТЬ (64-bit struct sizes, несовместимы с 32-bit ARM)
- compiled/particles/*.xml.compiled — определения частиц (нужен Definition.cpp парсер)

## Текущие блокеры (по приоритету) — обновлено session 5:
1. ~~ReanimLoader XML парсит 1 трек вместо 48~~ — **ИСПРАВЛЕНО** (session 4, FindElement)
2. ~~Reanim images не загружаются~~ — **ИСПРАВЛЕНО** (session 4, IMAGE_REANIM_ mapping)
3. **Меню — 10 rect кнопок рисуются ПОВЕРХ reanim** — reanim грузится+анимируется+lazy-load работает, но stub GameButton'ы всё ещё сверху. Нужно: убрать rect кнопки, использовать reanim button-sprite track'и для hit-testing. **ПРЕДВАРИТЕЛЬНО**: gs_log.txt содержит ВСЕ track names — прочитай их с устройства, замапь на кнопки.
4. ~~SystemFont глифы кривые~~ — **ИСПРАВЛЕНО** (session 4, ')' glyph + скобки)
5. **Loading bar рендерится криво (полоской)** — TitleScreen.cpp grass-bar clip/scale логика. Пользователь видит "кривое разворачивание поля как полоски загрузки". Нужно починить DrawImage с srcRect (сейчас stub) или использовать DrawImage scaled.
6. ~~Purple screen + crash~~ — **ИСПРАВЛЕНО** (session 5, state machine + lazy image loading)
7. **Upstream код не портирован** — Reanimator.cpp (1501), Board.cpp (6364), Plant.cpp, Zombie.cpp, EffectSystem.cpp (541), TodParticle.cpp (1290), SeedChooserScreen.cpp (1158) и т.д.

## Технические ограничения Symbian GCCE 3.4.3:
- C++03 (NO auto, nullptr, range-for, constexpr, std::filesystem, std::atomic, char32_t)
- No libc (malloc/free/realloc/strncpy/atof — stubs in clib_stubs.cpp)
- e32def.h: TInt32 = `long int` (NOT `int`) — watch for typedef conflicts
- No <stdint.h> (shim in src/engine/stdint.h)
- No <map>/<list> (stl_stubs/ directory has shims)
- GCCE 3.4.3 has RTTI bugs — avoid dynamic_cast
- PowerVR MBX: GL_MAX_TEXTURE_SIZE=2048, NPOT textures need POT padding
- glTexSubImage2DOES broken on MBX — use full-POT glTexImage2D fallback

## Ключевые файлы порта:
- `src/Sexy.TodLib/ReanimLoader.h/.cpp` — XML парсер .reanim + **ReanimPlayer runtime** (ИСПРАВЛЕНО session 4)
- `src/Lawn/Widget/GameSelector.h/.cpp` — меню (reanim грузится+анимируется, но rect кнопки сверху — убрать)
- `src/Lawn/Widget/GameButton.h/.cpp` — кнопки (переписать на sprite buttons с polygon hit-test)
- `src/Lawn/Widget/TitleScreen.h/.cpp` — loading screen (порт 1:1 state machine)
- `src/engine/Graphics.cpp` — DrawString → mFont->DrawString
- `src/engine/SystemFont.h/.cpp` — 8x8 bitmap font (')' glyph ИСПРАВЛЕН session 4)
- `src/engine/ResourceManager.cpp` — LoadImageByResName (IMAGE_REANIM_ mapping ИСПРАВЛЕН session 4)
- `src/engine/PvZVfs.cpp` — PAK reader (XOR 0xF7, case-insensitive)
- `src/engine/clib_stubs.cpp` — malloc/free/realloc/strncpy/atof stubs
- `src/engine/miniz.c + miniz_tinfl.c + miniz_tdef.c` — zlib
- `src/engine/stdint.h` — shim для GCCE
- `group/PvZ_N95.mmp` — build file (SOURCE list)
- `src/platform/symbian/PvZAppUi.cpp` — loading state machine + d-pad input
- `src/platform/symbian/PvZGameView.cpp` — EGL/GLES render loop
- `src/lawnapp.cpp` — LawnApp (game flow, ShowGameSelector, PreNewGame)
- `src/Resources.cpp/.h` — все IMAGE_*/FONT_*/SOUND_* globals
- `src/engine/Stubs.h` — typedefs, stub functions, IMAGE_* macros

## Важно:
- НЕ делай "похоже" — делай 1:1 порт
- Сравнивай с upstream кодом: https://github.com/wszqkzqk/PvZ-Portable
- Push напрямую в main (не создавай PR)
- Регулярно обновляй README.md с прогрессом
- НЕ удаляй старые заметки в README — добавляй новые секции
- Сначала почини сборку, потом тестируй на устройстве
- Звук/музыку можно оставить stub до последнего этапа
