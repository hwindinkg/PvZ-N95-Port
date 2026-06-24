# PvZ-N95-Port

A native **Symbian S60 3rd Edition (FP1)** port of
[PvZ-Portable](https://github.com/wszqkzqk/PvZ-Portable) — *Plants vs. Zombies*
running on the **Nokia N95** (Symbian OS 9.2, ARMv5, GCCE, OpenGL ES 1.1).

> ⚠️ Work in progress. This repo contains the C++ source, the build files
> (`.mmp`/`bld.inf`) and the resource scripts. Game assets are **not** shipped
> here — you provide `main.pak` on the device (see *Assets* below).

---

## Status

The game boots, initialises an EGL / OpenGL ES 1.1 context and runs the engine
heartbeat. Recent work fixed the early-boot crashes that produced the
"green grid / flashing purple screen that could not be closed".

### Fixed in this revision
| Symptom | Root cause | Fix |
|---------|-----------|-----|
| App could not be closed; **purple screen flashing** every frame; system dialogs overdrawn; app relaunched itself (`E32Main` twice in `boot.log`) | The whole game loop (`while(...) { RenderFrame(); User::After(); }`) ran **inside `CPvZAppUi::ConstructL()`**, starving the window server so it could never deliver key/exit events. The dark-purple `glClearColor` was swapped to screen every frame. | Game loop moved to a **`CPeriodic` heartbeat timer** (active object). `ConstructL` now returns normally and the active scheduler runs the UI. See `src/platform/symbian/PvZAppUi.cpp`. |
| Commands / exit not handled | `CAknAppUi::BaseConstructL()` was never called | `BaseConstructL(EAknEnableSkin)` added at the top of `ConstructL`. |
| `wgt_log.txt`: **`DrawAll: mGL NULL`** — widgets drew nothing | The `Sexy::Graphics` object keeps its **own** GL pointer; only `LawnApp::mGL` was set, never the Graphics object's. | After `InitGLES()` we call `mGraphics->SetGLInterface(GetGL())`. |
| `rmgr_log.txt`: **green/purple checkerboard** instead of art; "image not found" | PNG decoding was **not implemented** — `ResourceManager::LoadImageFromPak()` threw the PNG bytes away and built a magenta placeholder. | Implemented a real decoder using the **Symbian Image Conversion Library (ICL)**: `CImageDecoder` → `CFbsBitmap (EColor16MA)` → ARGB (`0xAARRGGBB`) buffer handed to `MemoryImage::SetBits`. Driven synchronously with a nested `CActiveSchedulerWait`. Added `imageconversion.lib`. |
| Case-insensitive PAK lookup broken | Typo `c += ('a' - 'a')` (`+= 0`, a no-op) | Corrected to `c += ('a' - 'A')`. |

---

## Building

Requires the **S60 3rd Edition FP1 SDK (Symbian OS 9.2)** and the **GCCE** ARM
toolchain (plus the Open C/C++ plug-in for the libc functions used by the engine).

```bat
cd group
bldmake bldfiles
abld build gcce urel
:: package
makesis ..\sis\PvZ_N95.pkg
signsis PvZ_N95.sis PvZ_N95.sisx <your-cert>.cer <your-key>.key
```

> The `.mmp` currently has a couple of absolute `SYSTEMINCLUDE` paths
> (`stl_stubs`). Adjust them to your checkout location if you do not build from
> `\Symbian\N95PVZ`.

> **Signing keys are intentionally not in this repo.** Generate your own
> self-signed cert (`makekeys`) — never commit `.key`/`.cer`.

---

## Assets

The game reads its packed assets from:

```
C:\Data\PvZ\main.pak
```

Place a converted PvZ-Portable `main.pak` there before launching. Boot/render
logs are written next to it in `C:\Data\PvZ\`.

---

## Project layout

```
group/        bld.inf, PvZ_N95.mmp, ABLD.BAT          (build configuration)
data/         *.rss                                   (Avkon resources + registration)
sis/          PvZ_N95.pkg                             (installer descriptor)
src/
  main_symbian.cpp                                    (E32Main entry point)
  platform/symbian/  Application, Document, AppUi, GameView (EGL/GLES, heartbeat)
  engine/            GLInterface, Graphics, ResourceManager (ICL), PAK/VFS, MemoryImage
  Lawn/ Sexy.TodLib/ ...                              (ported game/engine code)
```

---

## Credits & License

- Original game framework & assets: PopCap *Plants vs. Zombies*.
- PvZ-Portable by **Zhou Qiankang (wszqkzqk)** — https://github.com/wszqkzqk/PvZ-Portable
  (LGPL-3.0-or-later AND LicenseRef-PopCap).
- Symbian/N95 port: this repository.

Engine/port source is **LGPL-3.0-or-later**; PopCap-derived content remains
under its original license. See source headers for per-file SPDX identifiers.
