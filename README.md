# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Port of **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) to Symbian OS S60 3rd Edition FP1 (Nokia N95), built with **GCCE + libgcc**.

> **READ THIS FIRST (handoff for the next session).** The build works and the
> engine boots, but the game shows only a static frame because the **asset/
> resource pipeline is stubbed out**. This is the #1 thing to fix. Details below.

## Current status (2026-06)

**Working:**
- GCCE build pipeline (`group/build_gcce.cmd`) — compiles, links, makes SIS.
- C++ runtime: EH / `User::Leave` / `TRAP` work (via `STATICLIBRARY libgcc.lib`). No more KERN-EXEC 3.
- EGL/OpenGL ES context init, GL texture upload primitives (`glimage.cpp`, `GLInterface.cpp`).
- `.pak` virtual filesystem (`PvZVfs.cpp`, `pakinterface.cpp`), PNG decode (`stb_image`).
- `WidgetManager` draws (`DrawScreen -> DrawAll`).
- App boots: `LawnApp::Init` -> title screen created -> `InitGLES` -> heartbeat `CPeriodic` (EPriorityLow).
- Resource loading is now DRIVEN synchronously after GL is wired (commit 87157084): `LoadingThreadProc()` + `LoadingCompleted()` in `PvZAppUi::ConstructL`. This advanced the game from 1 widget to the GameSelector menu (`wgt_log: Widgets=2`).

**BROKEN / the real blocker — asset pipeline is stubbed:**
- `group/PvZ_N95.mmp` compiles **`Resources_stub.cpp`**, NOT `Resources.cpp`.
  => ALL `IMAGE_*` and `FONT_*` globals are **NULL**. Nothing has pixels to draw.
- `src/engine/Stubs.h` no-ops the loaders:
  - `TodLoadResources(...) { return true; }`  (loads nothing)
  - `TodLoadNextResource() { return false; }`
  - `TodStringListLoad`, `LoadProperties`, `ReanimatorLoadDefinitions`, `TodParticleLoadDefinitions` -> empty
  - `DrawDirtyStuff() {}` (SexyAppBase-level)
- `src/engine/SexyAppBase.cpp` is a **3.5 KB stub** (original is ~99 KB).
- `src/engine/misc/ResourceManager.cpp` ~10 KB (original ~30 KB) — trimmed.

Net effect: the engine runs game logic but every sprite is NULL, so the screen
is just the GL clear-colour (purple) plus one manually drawn rect (white box).
Chasing EGL/redraw/flush bugs was a dead end — **there is nothing to render until
real resources load.**

## The plan (milestones)

**M1 — prove the pipeline end-to-end (ONE image).**
- Un-stub `TodLoadResources`/`TodLoadNextResource` for a single group.
- Make `IMAGE_TITLESCREEN` a real `Image*` loaded from `.pak` via the real `ResourceManager` -> GL texture.
- Draw it on the title screen. Success = real PvZ title art on the phone.

**M2 — restore the resource table.**
- Bring back the real `Resources.cpp` + `Resources.h` (full `IMAGE_*`/`FONT_*` + `ExtractResourcesByName`) from upstream, adapt declarations, compile it INSTEAD of `Resources_stub.cpp`.
- Wire `resources.xml` parsing (`ResourceManager::ParseResourcesFile`) to populate the globals.

**M3 — un-stub the loader groups & fonts/strings.**
- Real `TodStringListLoad`, `LoadProperties`, fonts. Verify `rmgr_log` walks ALL groups, not just LoaderBar.

**M4 — main menu interactive**, then **M5 — gameplay board, reanimations, particles, sound**.

## How to build / run / debug
```
cd group
build_gcce.cmd            # outputs build\out\PvZ_N95.sis
# install the SIS on a cert-patched N95, launch
```
Logs are written to `C:\Data\PvZ\` on the device (RFile + Flush each line, no console):
- `boot.log` — step trace through startup
- `log.txt` — GL/render/frame trace (`GL:*`, `RF*`)
- `rmgr_log.txt` — ResourceManager loading (which groups loaded)
- `wgt_log.txt` — widget manager (`Widgets=N` count)
- `gl_log.txt`, `gfx_log.txt` — low-level GL/graphics

## Reference ports (clone & compare — this is how the bugs got found)
- **re3-symbian (GTA3)** — https://gitlab.com/shinovon/re3-symbian — full game, GCCE, RenderWare-on-EGL, `.pak`-style VFS, `CPeriodic` loop, `SetExtentToWholeScreen`. Best reference. Key file: `src/skel/symbian/symbian.cpp`.
- **Whisk3D** — minimal EGL/GL Symbian app; clean EGL config-from-DisplayMode example.
- **Upstream PvZ-Portable** — the desktop source this port is derived from. Compare file sizes to spot gutted files.

## Diagnostic tip for the next session
Before touching rendering, run: compare each `src/engine/*.cpp` size against the
upstream original; grep for `_stub`, `return true; //`, `inline .* {}`. A file that
is a fraction of the upstream size is gutted and is your real problem.