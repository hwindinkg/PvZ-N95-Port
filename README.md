# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Port of **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) to Symbian OS S60 3rd Edition FP1 (Nokia N95), built with **GCCE + libgcc**.

> **READ THIS FIRST (handoff for the next session).** The build works and the
> engine boots, but the game shows only a static frame because the **asset/
> resource pipeline is stubbed out**. This is the #1 thing to fix. Details below.

## Current status (2026-06-25)

The crash chain has been peeled back one layer at a time. We are now well past
the engine-boot stage and into **real asset rendering** — the screen no longer
freezes the whole phone.

**Working:**
- GCCE build pipeline (`group/build_gcce.cmd`) — compiles, links, makes SIS.
- C++ runtime: EH / `User::Leave` / `TRAP` work (via `STATICLIBRARY libgcc.lib`). No more startup KERN-EXEC 3.
- EGL/OpenGL ES context init; GL texture upload primitives.
- `.pak` virtual filesystem; ICL-based PNG **and JPEG** decode.
- `WidgetManager` draws; `ConstructL` completes fully (heartbeat `CPeriodic` running, `Widgets=2` GameSelector menu).
- **M1 in progress — single-image asset path proven up to decode.** `PvZAppUi::ConstructL` explicitly loads `IMAGE_TITLESCREEN` to exercise the path GetImage -> resources lookup -> PAK -> ICL decode -> MemoryImage -> lazy GL texture.

**Recently fixed (this session):**
- **ICL decode deadlock (whole-phone freeze).** `DecodeToBitmapL` waited on the
  async `CImageDecoder::Convert` with `User::WaitForRequest` on the UI thread —
  the codec's own active objects could never run, hanging the UI thread *and* the
  window server (battery-pull required). Replaced with a proper `CActive` +
  nested `CActiveSchedulerWait` driver, plus a `KErrUnderflow` (progressive
  JPEG) guard and per-step `[dec]` logging. Result: `M1: TITLESCREEN OK 800x600`,
  `decoded OK`.
- **NPOT-texture KERN-EXEC 3 on the first textured draw.** After a clean decode
  the app crashed on the very first frame (`gl_log` ends at `SetColor called`,
  then KERN-EXEC 3). Root cause: the N95 GPU is a **PowerVR MBX (GL ES 1.1) that
  supports power-of-two textures only**. The title art is 800x600 (NPOT); uploading
  it via `glTexImage2D` and sampling 0..1 read out of bounds -> access violation.
  Fix (`src/engine/Graphics.cpp`): `GetOrCreateTexture` now rounds W/H up with
  `NextPow2`, allocates an **empty POT texture**, sub-uploads the image top-left
  via `CopyImageToTextureSub`, and caches the padding ratio (uMax = w/potW,
  vMax = h/potH). Both `DrawImage` overloads (full-image and src-rect) now scale
  their texcoords by the cached uMax/vMax instead of assuming 0..1. *(Needs an
  on-device build to confirm the title art renders.)*

**Still the bigger blocker — bulk asset pipeline is stubbed:**
- `group/PvZ_N95.mmp` compiles **`Resources_stub.cpp`**, NOT `Resources.cpp`.
  => ALL `IMAGE_*` and `FONT_*` globals are **NULL** except the one M1 image.
- `src/engine/Stubs.h` no-ops the loaders (`TodLoadResources` returns true and
  loads nothing, `TodLoadNextResource` returns false, string/property/reanimator/
  particle loaders empty).
- `src/engine/SexyAppBase.cpp` is a **3.5 KB stub** (original ~99 KB);
  `ResourceManager.cpp` is trimmed vs upstream.

Net: the engine runs game logic, the single M1 image proves the decode+GL path,
but the bulk `IMAGE_*/FONT_*` table is still NULL — M2 (restore `Resources.cpp`)
is the next big step.

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