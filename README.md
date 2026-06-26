# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Port of **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) to Symbian OS S60 3rd Edition FP1 (Nokia N95), built with **GCCE + libgcc**.

> **READ THIS FIRST (handoff for the next session).** Build compiles & links,
> the engine boots, EGL/WSERV display is correct, app is locked to **landscape**.
> M1 (single image end-to-end) and M2 (real `Resources.cpp`) are done. We are in
> **M3**: getting the FIRST real in-game frame on screen.
>
> **LATEST WORK (2026-06-26, commit `eba5a08`) — awaiting on-device test.**
> Found and fixed THREE compounding bugs that kept *every* real image off screen
> (we had only ever seen solid fills / test patterns, never a decoded asset):
> 1. **RTTI-dependent draw.** `Graphics::DrawImageF`/`DrawString` used
>    `dynamic_cast<MemoryImage*>(img)`. GCCE 3.4.3 Symbian builds typically have
>    **RTTI OFF**, so the cast returned NULL → every image fell through to a
>    WHITE placeholder `FillRect`. Switched both to `static_cast` (the
>    ResourceManager only ever creates `MemoryImage`-backed images).
> 2. **Oversized textures render invisibly.** `GLInterface::CreateTexture` made a
>    texture of the full POT size with no bound check. The lawn (1400x600) rounds
>    to a **2048x1024** POT, but **PowerVR MBX Lite (N95) has
>    `GL_MAX_TEXTURE_SIZE = 1024`** → `glTexImage2D` rejects it → draws nothing
>    (only the purple clear shows; that stale "2048x1024" in `gl_log` never hit
>    the screen). `CreateTexture` now queries the limit once (logs
>    `GLI::GL_MAX_TEXTURE_SIZE=N`) and REJECTS oversized requests (returns 0 →
>    visible white placeholder instead of silent nothing).
> 3. **Wrong first-frame asset.** `GameSelector` drew `IMAGE_BACKGROUND1`, which
>    on N95 (a) fails to decode (1400x600 → OOM; `gs_log` read `bg=NULL`) and
>    (b) needs a 2048 texture (see #2). Switched the first frame to
>    `IMAGE_TITLESCREEN` (800x600 JPEG, robust decode, POT **1024x1024** = exactly
>    at the MBX limit). It was `DeleteImage`'d after loading, so `GetImage`
>    re-decodes+caches on demand; we retry each frame until ready, never cache a
>    NULL, and draw at native (0,0) via `static_cast` + the direct
>    `DrawImage(MemoryImage*)` overload.
>
> **NEXT SESSION — verify first, then continue:** rebuild, run, read
> `C:\Data\PvZ\gs_log.txt` (expect `GS:Draw drawing IMAGE_TITLESCREEN 800x600`)
> and `gl_log` (expect a `GLI::GL_MAX_TEXTURE_SIZE=...` line — **this confirms the
> device's real texture limit; the whole #2 fix rests on it being ~1024**). If the
> top-left of the title art is visible → first real frame achieved 🎉.
>
> **⚠️ DO NOT RE-DEBUG THESE — already solved:**
> - *"Purple/pink screen, lawn never shows"* → NOT an empty-texture/bits problem.
>   It was the 2048-texture MBX limit (#2) + the 1400x600 decode OOM (#3). Do NOT
>   chase `GetBits()`/bulk-load-frees-bits theories again.
> - *"No image ever renders, only solid colours"* → the `dynamic_cast` RTTI bug
>   (#1). If images still don't draw, re-check that NO `dynamic_cast` crept back
>   into `Graphics.cpp`, and confirm RTTI status in the build.
> - *"Green mesh / garbage until I open the Options menu"* → caused by
>   `Window().SetRequiredDisplayMode(EColor64K)`. **Removed. NEVER re-add it.**
> - *KERN-EXEC 3 on the first menu frame* → dangling `IMAGE_TITLESCREEN` after
>   `DeleteImage()`. Fixed by nulling the global.
> - *White rectangle + RGB bars* → `TitleScreen::Draw`'s debug test pattern (now
>   removed; a lingering TitleScreen was painting it over the real frame).
> - *"Wallpaper renders vertically / squished"* → portrait window vs 4:3 canvas.
>   Fixed with `SetOrientationL(Landscape)` in `PvZAppUi` (NOT in the view).

## Current status (2026-06-26)

App **boots, renders correct colours, stable render loop** (RF1..RF4000+,
`eglSwapBuffers` = EGL_SUCCESS), locked to **landscape 320x240** (perfect 4:3).
The three-bug fix above (commit `eba5a08`) is the latest change and is **awaiting
on-device confirmation** — before it, the screen was flat purple/pink because no
real image could reach the framebuffer.

**Working:**
- GCCE build pipeline (`group/build_gcce.cmd` AND `build_sisx.cmd` — TWO scripts,
  keep them in sync) — compile, link, make SIS.
- C++ runtime (EH / `User::Leave` / `TRAP`) via `STATICLIBRARY libgcc.lib`.
- EGL/GLES 1.1 context + POT texture upload; landscape orientation.
- `.pak` VFS; ICL-based PNG **and** JPEG decode (115 images decode OK at boot).
- `WidgetManager` draws; heartbeat `CPeriodic` render loop stable.
- **Image draw path is now RTTI-independent and texture-size-safe** (this commit).

**TODO / still open (M3, priority order):**
1. **Confirm the first real frame** on device (read `gs_log.txt` + `gl_log`). Until
   a frame is verified, everything else is downstream.
2. **Scale the title to fill the screen.** Currently drawn at native size so only
   the top-left 400x300 of 800x600 shows. `DrawImage(MemoryImage*)` does NOT apply
   `mScaleX/mScaleY`; use `PushTransform` with a `SexyTransform2D().Scale(sx,sy)`
   (same mechanism as the working `DrawImageRotated`) — sx=400/800, sy=300/600.
3. **Restore the lawn (`IMAGE_BACKGROUND1`) properly.** Needs BOTH: (a) decode it
   without OOM, and (b) a texture ≤1024 — i.e. **texture TILING** (split into
   ≤1024 tiles, the way the original Sexy framework does) or a pre-downscale.
   The MBX 1024 limit is the hard constraint; do not try a single 2048 texture.
4. **Real `GameSelector` content** (Adventure / Survival buttons). NOTE: menu
   buttons are `REANIM_*` assets that are **NOT in this PAK** (`NOPAK` in
   rmgr_log) — can't be drawn from art until packed or the Reanimator is un-stubbed.
5. Fonts (`GetFontThrow` stubbed), strings, sounds not really loaded.
6. ~51 game-referenced `IMAGE_/FONT_/SOUND_` symbols are defined but **NULL**.
7. 149 `REANIM_*` images report `not in PAK` (asset naming / packing mismatch).

**Key code facts for next session:**
- `ResourceManager::GetImage(name)` looks up a name→`MemoryImage*` cache and
  **on a miss does an on-demand PAK decode** (it caches the result). It does NOT
  set the `IMAGE_*` globals. `LoadImageByResName` does NOT cache and re-decodes
  every call — **never call it per-frame** (that was an earlier bug).
- The `IMAGE_*` globals are only assigned by the `Extract*Resources` functions in
  `Resources.cpp`. `IMAGE_BACKGROUND1` is set in
  `ExtractDelayLoad_Background1Resources` — a **delay-load** group that is **not**
  loaded at boot, so that global stays NULL until something triggers the group.
- Logs on device are in `C:\Data\PvZ\` (RFile + Flush per line). `gs_log.txt` is
  REWRITTEN each run (reliable); `gl_log.txt` is APPEND (can show stale lines from
  previous runs — check texCount #).

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
- On-device rendering verified correct (correct colours from frame 1; the old
  debug test pattern has since been removed).

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