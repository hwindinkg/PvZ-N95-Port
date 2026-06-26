# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Port of **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) to Symbian OS S60 3rd Edition FP1 (Nokia N95), built with **GCCE + libgcc**.

> **READ THIS FIRST (handoff for the next session).** The build works, the
> engine boots, and **on-screen rendering is now correct from the first frame**
> (M1 + M2 done — real `Resources.cpp` compiled & linked, EGL/WSERV display bug
> fixed). The current task is **M3**: restore the real `GameSelector` menu
> content (Adventure/Survival) and verify fonts / strings / sounds load. Many
> `IMAGE_*` are still NULL pending real per-group resource loading.
>
> **⚠️ DO NOT RE-DEBUG THESE — already solved (see "S60 EGL/WSERV gotchas"):**
> - *"Green mesh / garbage until I open the Options menu"* → caused by
>   `Window().SetRequiredDisplayMode(EColor64K)`. **Removed. NEVER re-add it.**
> - *KERN-EXEC 3 on the first menu frame* → dangling `IMAGE_TITLESCREEN` after
>   `DeleteImage()`. Fixed by nulling the global.

## Current status (2026-06-26)

The crash chain has been fully peeled back. The app **boots, renders correct
colours from frame 1 (no menu kick needed), and runs a stable render loop**
(RF1..RF400+, `eglSwapBuffers` returns EGL_SUCCESS / 0x3000). Confirmed
on-device: purple background + white rectangle + RGB strip at the bottom.
Next: **M3** menu content + fonts/strings/sounds.

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
- **First-frame KERN-EXEC 3 from a stale GL texture-state cache.** Even after the
  NPOT fix the app still crashed on the first real draw (`gl_log` ends at
  `SetColor called`, purple `glClear` shows, then KERN-EXEC 3 — and crucially NO
  `GLI::CreateTexture` line, proving the crash is *before* any texture upload, in
  the first `FillRect`). Root cause: `GLInterface` constructor sets
  `mTextureEnabled = EFalse`, but `SetupGLState()` did `glEnable(GL_TEXTURE_2D)`.
  The first `FillRect` -> `SetTextureEnabled(EFalse)` early-returned on the stale
  cache, so `GL_TEXTURE_2D` stayed ON with NO texture bound. On PowerVR MBX,
  `glDrawArrays` then samples a null texture -> access violation. Fix
  (`src/engine/GLInterface.cpp`): `SetupGLState` now starts with texturing
  DISABLED (matching the cache) and the `GL_TEXTURE_COORD_ARRAY` client state
  disabled; `SetTextureEnabled` toggles `GL_TEXTURE_2D` AND the texcoord array in
  lockstep, so we never draw textured-without-a-texture and never leave the
  texcoord array live while texturing is off. *(Needs an on-device build to
  confirm the title screen renders.)*
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

**M2 DONE — real `Resources.cpp` is compiled & linked:**
- `group/PvZ_N95.mmp` **and** `build_sisx.cmd` now compile the real
  **`Resources.cpp`** (NOT `Resources_stub.cpp`). Both build scripts were
  updated — there are TWO of them, keep them in sync.
- `Resources.h` was **regenerated mechanically** from the actual definitions
  (774 globals: 585 `IMAGE_*`, 21 `FONT_*`, 168 `SOUND_*`; plus the
  `enum ResourceId` matched 1:1 to the `gResources[RESOURCE_ID_MAX]` array so
  `RESOURCE_ID_MAX` == array size). If you edit the resource list, regenerate the
  header the same way — do not hand-maintain it.
- See "GCCE / C++98 gotchas" and "Resources.cpp restoration scars" below for the
  type/namespace fixes that were needed.

**Still stubbed (this is M3):**
- `GameSelector` content is an empty stub (that's why you see only the test
  rectangle, not Adventure/Survival buttons).
- Many group loaders still need un-stubbing so `rmgr_log` walks ALL groups, not
  just LoaderBar; fonts/strings/sounds need real loading. ~51 game-referenced
  `IMAGE_/FONT_/SOUND_` symbols are defined but **NULL** until per-group loading
  runs (added only so the real `Resources.cpp` links).

## S60 EGL/WSERV gotchas (hard-won — read before touching rendering)

- **NEVER call `Window().SetRequiredDisplayMode(...)` for an EGL window.** Leave
  the window in the device's native mode. On the N95 the only EGL configs are
  **888 / 24-bit** — forcing the window to 16-bit (`EColor64K`) makes WSERV
  composite garbage until an external event triggers a full recomposite. The
  working re3-symbian reference does `CreateWindowL -> SetExtentToWholeScreen ->
  SetFocus -> ActivateL` and never sets the display mode. Match that.
- **`eglSwapBuffers` succeeding ≠ pixels on screen.** If swap returns EGL_TRUE /
  EGL_SUCCESS but the screen is wrong, suspect a **window/surface FORMAT
  mismatch** or a composite issue, NOT your GL code.
- **`EGL_BUFFER_SIZE` is a MINIMUM, not an exact request** — asking for 16 on the
  N95 returns a 32-bit (888 a8) config. Derive `wantR/G/B` from
  `Window().DisplayMode()` and don't assume RGB565 exists.
- Things that DON'T fix a "garbage until external event" composite bug (already
  tried, wasted time): `DrawDeferred()`, `Invalidate()+DrawDeferred()`,
  `SetOrdinalPosition(0)`, `SetBackgroundColor`. The real fix was the display
  mode (above).
- **Dangling globals after `DeleteImage`**: the engine's `DeleteImage(name)` frees
  the object but does NOT null the corresponding `IMAGE_*` global. Any `Draw()`
  guarded by `if (IMAGE_X)` will pass and deref freed memory -> KERN-EXEC 3.
  Null the global yourself after deleting.

## Stub-header & image-decode gotchas (M3, hard-won)

- **A stub in `Stubs.h` can SHADOW the real function and silently win.**
  `LawnApp.cpp` includes only `engine/Stubs.h`, NOT `Resources.h`. A global
  `inline bool ExtractResourcesByName(ResourceManager*, const char*) { return true; }`
  in Stubs.h therefore bound to `LawnApp::LoadGroup`'s call and loaded NOTHING
  (rmgr_log showed `StartLoadResources` per group but ZERO `LoadImageByResName`).
  The real `Sexy::ExtractResourcesByName` in Resources.cpp was never reached.
  *Fix: delete the stub, forward-declare the REAL one in `namespace Sexy` inside
  Stubs.h (commit 92fdc7d).* **When resources mysteriously don't load, grep
  `Stubs.h` for a same-named stub shadowing the real symbol.**
- **N95 ICL cannot Convert PNG into a 32-bit `EColor16MA` bitmap** — it returns
  `KErrNotSupported (-5)`. JPEG works (that's why titlescreen.jpg always
  decoded), PNG does not (106/106 failed). The PNG header DOES parse (dimensions
  log fine) — only the Convert step fails. *Fix: decode into a 24-bit
  `EColor16M` color bitmap + (if `TFrameInfo` reports transparency) a separate
  grayscale alpha mask (`EGray256`/`EGray2`) via the two-bitmap Convert overload,
  then recombine to ARGB; FORCE alpha=0xFF for opaque pixels, overlay mask alpha
  otherwise (commit ac38504).* Decode counters to watch in rmgr_log:
  `convert done err 0x0` = OK, `err -5` = the unsupported-target bug.
- **Counting decode outcomes**: split rmgr_log on `LoadImageByResName:` and tally
  `convert done err 0x0` (OK) vs `err -5` (codec target unsupported) vs
  `not found in PAK` (asset naming mismatch — a separate, still-open issue).

## Orientation / screen geometry (hard-won)

- **The game is LANDSCAPE 4:3; the N95 default is PORTRAIT.** GL ortho is 400x300
  (4:3). The N95 numeric-slide default window is 240x320 (3:4). If you render the
  4:3 canvas into a portrait window (or hardcode a portrait viewport) the image is
  squished vertically ("wallpaper shows vertically"). *Fix: lock the app to
  landscape with `CAknAppUiBase::SetOrientationL(EAppUiOrientationLandscape)`
  called right after `CreateWindowL()` and BEFORE `SetExtentToWholeScreen()`
  (same as the re3-symbian GTA3 reference). 320x240 is EXACTLY 4:3 -> full screen,
  no letterbox (commit f8a99f3).*
- **Never hardcode the viewport size.** `PvZGameView::InitGLES` used to call
  `SetViewport(0,0,240,320)`. Use the ACTUAL `Size()` and feed
  `GLInterface::UpdateViewport(w,h)` (it 4:3-letterboxes into the surface). Watch
  the `GL:viewport from Size %dx%d` log to confirm 320x240 on device.
- `SetOrientationL` needs `avkon.lib` + `<aknappui.h>` (already linked in both
  build scripts).
- The N95 multimedia slider exposes the gaming/media keys in landscape -- so a
  landscape lock is also the prerequisite for mapping those keys to game input
  (a later input task; the media keys likely need RWindowGroup::CaptureKey or the
  RemCon API to be received by the app).

## The plan (milestones)

**M1 — prove the pipeline end-to-end (ONE image). ✅ DONE.**
- Un-stub `TodLoadResources`/`TodLoadNextResource` for a single group.
- Make `IMAGE_TITLESCREEN` a real `Image*` loaded from `.pak` via the real `ResourceManager` -> GL texture.
- Draw it on the title screen. Success = real PvZ title art on the phone.

**M2 — restore the resource table. ✅ DONE.**
- Real `Resources.cpp` + mechanically-generated `Resources.h` now compile and
  link (instead of `Resources_stub.cpp`). `resources.xml` parsing path is wired.
- On-device rendering verified correct (purple bg + white rect + RGB strip).

**M3 — un-stub the loader groups & fonts/strings + real menu. ⬅ CURRENT.**
- Restore real `GameSelector` content (Adventure / Survival buttons).
- Real `TodStringListLoad`, `LoadProperties`, fonts. Verify `rmgr_log` walks ALL
  groups, not just LoaderBar; confirm `IMAGE_*` for the menu actually load.

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

## Diagnostic history — the KERN-EXEC 3 crash chain (newest first)

Each fix peeled back one layer. Kept here so the next session doesn't re-walk it.

**RENDER DISPLAY CHAIN (newest — these came AFTER the engine booted):**

A. **"Green mesh until I open the menu"** (`PvZGameView.cpp`) — the colours were
   correct in the EGL surface (`eglSwapBuffers` returned EGL_TRUE / EGL_SUCCESS
   = 0x3000 every frame) but the screen showed garbage until an external WSERV
   event (opening Options / a keypress) forced a full recomposite. Root cause:
   we called `Window().SetRequiredDisplayMode(EColor64K)` forcing the WINDOW to
   16-bit RGB565, while the N95's only EGL configs are **888 / 24-bit**
   (`GL:diag cfgN=4`, all "888 a8"). The window↔surface format mismatch made
   WSERV defer the conversion until a recomposite. *Fixed: removed
   `SetRequiredDisplayMode` entirely so the window keeps the device's native
   24-bit mode, matching the 888 surface (commit 8bbc6cb). `DrawDeferred()`,
   `SetOrdinalPosition(0)`, and an EGL_BUFFER_SIZE tweak were tried first and
   did NOT help — don't repeat them.* Confirmed fixed on-device.
B. **KERN-EXEC 3 on first menu frame** (`LawnApp.cpp` / `TitleScreen::Draw`) —
   `LoadingCompleted()` called `DeleteImage("IMAGE_TITLESCREEN")` which freed the
   image but left the global `IMAGE_TITLESCREEN` pointer dangling (non-NULL ->
   passed the `if(IMAGE_TITLESCREEN)` guard -> `DrawImage` deref'd freed memory).
   *Fixed: null the global right after `DeleteImage` (commit 5b17aca).*

1. **GL texture-state cache desync** (`GLInterface.cpp`) — `SetupGLState` enabled
   `GL_TEXTURE_2D` while the cache said disabled; first `FillRect` drew textured
   with no texture bound. *Fixed: start texturing off + lockstep texcoord array.*
2. **NPOT texture** (`Graphics.cpp`) — 800x600 title art uploaded to a non-POT
   texture; MBX is POT-only. *Fixed: pad to POT + scale texcoords by w/potW.*
3. **ICL decode deadlock** (`ResourceManager.cpp`) — `User::WaitForRequest` on the
   UI thread froze the whole phone. *Fixed: `CActive` + `CActiveSchedulerWait`.*
4. **Asset loader stubbed** — `LoadingThreadProc` never called; added synchronous
   load after GL init. (Bulk loader still stubbed — see M2.)
5. **C++ EH runtime missing under RVCT** — `User::Leave` (= `throw`) crashed with
   no unwinder. *Fixed: switched toolchain RVCT -> GCCE + `libgcc.lib` (matches
   the working Whisk3D reference).*

**Debugging method that worked:** on-device file logs with `RFile::Flush()` per
line, then read where each log stream stops. The combination of *which* log ends
and *which* expected line is absent (e.g. `gl_log` has `SetColor` but no
`CreateTexture`) localises the fault to a few lines without a debugger.

## Reference ports (clone & compare — this is how the bugs got found)
- **re3-symbian (GTA3)** — https://gitlab.com/shinovon/re3-symbian — full game, GCCE, RenderWare-on-EGL, `.pak`-style VFS, `CPeriodic` loop, `SetExtentToWholeScreen`. Best reference. Key file: `src/skel/symbian/symbian.cpp`.
- **Whisk3D** — minimal EGL/GL Symbian app; clean EGL config-from-DisplayMode example.
- **Upstream PvZ-Portable** — the desktop source this port is derived from. Compare file sizes to spot gutted files.

## Diagnostic tip for the next session
Before touching rendering, run: compare each `src/engine/*.cpp` size against the
upstream original; grep for `_stub`, `return true; //`, `inline .* {}`. A file that
is a fraction of the upstream size is gutted and is your real problem.