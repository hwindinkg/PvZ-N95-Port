# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Порт **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) на Symbian S60 3rd FP1 (Nokia N95), сборка через **GCCE 3.4.3 + libgcc**.

## Текущее состояние (session 14)

### Что работает
- **Engine boot**: EGL/GLES 1.1, landscape 320×240, 30 FPS
- **PAK VFS**: main.pak (45MB, 3198 файлов, XOR 0xF7)
- **Image decode**: ICL (PNG + JPEG) с alpha mask композицией
- **Alpha mask composition**: поиск `_<stem>` и `<stem>_` файлов для alpha channel (как в upstream ImageLib)
- **PopCap logo**: fade in/out (3 фазы: 30f in, 60f hold, 30f out)
- **Loading screen**: IMAGE_TITLESCREEN bg + PvZ logo slide-in + grass unroll + SODROLLCAP zombie head
- **ReanimLoader**: XML парсер .reanim файлов (48 треков, наследование трансформов)
- **ReanimatorRuntime**: полная система Reanimation (interpolation, render groups, DrawTrack)
- **Главное меню**: reanim с anim_open анимацией, BG + надгробие + дерево + трава + кнопки
- **Цветы в горшках** (session 14): 3 child-reanim играют `anim_flower1/2/3` (PLAY_ONCE_AND_HOLD, rate 0 = сидят на горшках). Клик в радиусе 20px от центра цветка → `mAnimRate=24` → цветок отваливается и падает. Звук `FOLEY_LIMBS_POP`.
- **Листья** (session 14): child-reanim `anim_grass` LOOP, позиция следует за `SelectorScreen_BG_Right`, периодическое шевеление каждые 200-400 кадров.
- **AssignRenderGroupToPrefix** (session 14): исправлен баг — теперь использует `strncasecmp` (prefix match) вместо `StrCaseCmp` (full compare). Раньше `AssignRenderGroupToPrefix("flower", -1)` НЕ скрывал трек `SelectorScreen_Flower1`.
- **Прозрачность**: GL_BLEND + vertex alpha (mAlpha из reanim трансформов)
- **d-pad input**: курсор + клики
- **Texture cache**: POT padding + eviction с glDeleteTextures

### Что не работает / в разработке
- **NewUserDialog**: окно ввода ника (нужен порт EditWidget)
- **Hover выделение кнопок**: ImageOverride на button tracks
- **Заблокированные кнопки**: скрыты через render group -1 (клик не попадает — HitTestButton проверяет mRenderGroup == -1)
- **Profile save**: сохранение в `C:\Data\PvZ\`
- **Геймплей**: Board.cpp, Plant.cpp, Zombie.cpp — не портированы
- **Звук/музыка**: stub
- **Шрифты**: SystemFont (8×8 bitmap), не настоящие PvZ шрифты

### Архитектура

#### Ключевые файлы
| Файл | Назначение |
|------|-----------|
| `src/Sexy.TodLib/ReanimLoader.h/.cpp` | XML парсер .reanim (FindElement, наследование трансформов) |
| `src/Sexy.TodLib/ReanimatorRuntime.h/.cpp` | Полная Reanimation система (Reanim2, ReanimHolder2) |
| `src/Lawn/Widget/GameSelector.cpp` | Главное меню (Reanimation + hit-testing) |
| `src/Lawn/Widget/TitleScreen.cpp` | Экран загрузки (PopCap logo + SODROLLCAP + grass unroll) |
| `src/engine/ResourceManager.cpp` | Загрузка изображений + alpha mask composition |
| `src/engine/Graphics.cpp` | GL rendering (DrawImage, DrawImageScaledSrcRect, texture cache) |
| `src/engine/GLInterface.cpp` | EGL/GLES init, GL state (GL_BLEND, no GL_ALPHA_TEST) |
| `src/platform/symbian/PvZAppUi.cpp` | State machine (loading → menu), d-pad input |
| `src/platform/symbian/PvZGameView.cpp` | EGL render loop, glClearColor(black) |

#### Reanimation система
- `.reanim` XML → `ReanimDefinition` (48 треков × 706 трансформов)
- `Reanim2` класс: `PlayReanim`, `GetCurrentTransform`, `DrawTrack`, `DrawRenderGroup`
- Наследование трансформов: tf[j] наследует числовые поля от tf[j-1] (не строки!)
- `ScanForImage`: сканирует назад для поиска изображения
- Render groups: group 1 = BG (рисуется первым), group 0 = кнопки, group -1 = скрытые

#### Координаты
- Reanim space: 800×600, top-left origin (0,0 = верхний левый угол)
- Canvas: 400×300 (×0.5 масштаб)
- Position = (mX + transX) × 0.5, (mY + transY) × 0.5
- mX=0, mY=0 (no offset — upstream AddReanimation(0.5, 0.5) ≈ (0,0))

#### Alpha mask composition
- Upstream ImageLib ищет `_<filename>` или `<filename>_` как alpha mask
- Alpha mask's blue channel → alpha channel основного изображения
- Реализовано в `ResourceManager::LoadImageByResName`

## Правила разработки (что можно и нельзя)

### Можно
- Использовать `new`/`delete` (port's operator new = User::Alloc, returns NULL on OOM)
- Использовать `malloc`/`free` (clib_stubs.cpp)
- Использовать `memcpy`, `memset`, `strncmp`, `strcmp`, `strlen`
- Использовать `atof` (реализован в clib_stubs.cpp)
- Использовать `<math.h>` (cos, sin, sqrt, floor — но НЕ floorf!)
- Использовать Symbian RFile/RFs для логов

### Нельзя
- **`floorf`** — нет в Symbian GCCE math.h. Использовать `(float)(int)val`
- **`strncasecmp`** — нет в Symbian GCCE. Использовать локальный `StrnCaseCmp` (см. ReanimatorRuntime.cpp). `AssignRenderGroupToPrefix` ДОЛЖЕН использовать prefix-match, не full-compare.
- **`nullptr`** — C++03. Использовать `NULL`
- **`auto`** — C++03
- **`range-for`** — C++03
- **`constexpr`** — C++03
- **`std::filesystem`** — нет
- **`std::map`/`std::list`** — нет (stl_stubs/ имеет shims)
- **`dynamic_cast`** — RTTI ненадёжен в GCCE 3.4.3
- **`delete[]` на статические литералы** — `ParseString` возвращает `""` (литерал) если тег не найден. НЕ вызывать `delete[]` на пустые строки.
- **Копировать `const char*` указатели между трансформами** — double-free в деструкторе. Устанавливать в `""` (литерал), `ScanForImage` найдёт изображение.
- **`glTexSubImage2DOES`** — сломан на MBX. Использовать full-POT `glTexImage2D`.
- **`SetRequiredDisplayMode(EColor64K)`** — ломает EGL surface. НЕ использовать.
- **`GL_ALPHA_TEST`** — upstream не использует. Только `GL_BLEND`.

### Безопасные паттерны
- `TRAPD(err, func())` — ловит Symbian Leaves (но НЕ KERN-EXEC 3)
- `User::Alloc` / `User::Free` — Symbian heap allocation
- `TBuf8<N>` / `TBuf<N>` — Symbian descriptors (8-bit / 16-bit)
- `_L8("text")` / `_L("text")` — Symbian literals (8-bit / 16-bit)

## Roadmap

### Stage 1: Главное меню (ТЕКУЩИЙ — ~90% готово)
- [x] PopCap logo fade
- [x] Loading screen (grass unroll + SODROLLCAP)
- [x] Reanimation runtime
- [x] Menu BG + tombstone + tree + buttons
- [x] anim_open animation
- [x] Alpha mask transparency
- [x] Locked buttons hidden (render group -1 + click cull)
- [x] **Potted flowers click-to-fall** (1:1 upstream, session 14)
- [x] **Leaf rustle** (1:1 upstream, session 14)
- [x] **AssignRenderGroupToPrefix prefix-match fix** (session 14)
- [ ] NewUserDialog (nickname entry)
- [ ] Profile save/load
- [ ] Button hover highlight
- [ ] Clickable buttons (Adventure → PreNewGame)

### Stage 2: Геймплей (НЕ НАЧАТ)
- Board.cpp (6364 lines) — игровое поле
- Plant.cpp — растения
- Zombie.cpp — зомби
- TodParticle.cpp (1290) — частицы
- SeedChooserScreen.cpp (1158) — выбор семян
- CutScene.cpp — катсцены

### Stage 3: Диалоги (НЕ НАЧАТ)
- StoreScreen.cpp (1187)
- AlmanacDialog.cpp (718)
- ZenGarden.cpp
- NewOptionsDialog.cpp

### Stage 4: Системы (ЧАСТИЧНО)
- ImageFont.cpp (1748) — настоящие PvZ шрифты
- SaveGame.cpp — сохранения
- Music.cpp — музыка
- SoundManager — звуки

## Сборка и тестирование

```bash
# Windows + Symbian SDK
cd C:\Symbian\PvZ-N95-Port-main\group
build_gcce.cmd
```

Логи на устройстве: `C:\Data\PvZ\`
- `boot.log` — загрузка
- `gs_log.txt` — GameSelector
- `gl_log.txt` — GL текстуры
- `gfx_log.txt` — Graphics
- `rmgr_log.txt` — ResourceManager
- `draw_progress.txt` — WidgetManager

## Upstream ссылки
- **PvZ-Portable**: https://github.com/wszqkzqk/PvZ-Portable
- **Порт**: https://github.com/hwindinkg/PvZ-N95-Port
- **Whisk3D (эталон EGL/Symbian)**: https://github.com/Dante-Leoncini/Whisk3D/tree/symbian
