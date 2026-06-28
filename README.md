# Plants vs. Zombies — Nokia N95 / Symbian S60 3rd FP1 Port

Port of **PvZ-Portable** (https://github.com/wszqkzqk/PvZ-Portable) to Symbian OS S60 3rd Edition FP1 (Nokia N95), built with **GCCE + libgcc**.

> **READ THIS FIRST (handoff for the next session).** Build compiles & links,
> the engine boots, EGL/WSERV display is correct, app is locked to **landscape**.
> M1 (single image end-to-end) and M2 (real `Resources.cpp`) are done. We are in
> **M3**: getting the FIRST real in-game frame on screen.
>
> **LATEST WORK (2026-06-27, commit pending) — titlescreen fully visible.**
> The first real frame (`IMAGE_TITLESCREEN`) now renders **full-canvas with
> correct colours** — no purple tint, no 1/4 crop, no overlay covering it.
> Four bugs were found and fixed beyond commit `eba5a08`:
>
> **FIX 4 — `BringToBack` buried GameSelector under an overlay.**
> `ShowGameSelector()` called `mWidgetManager->BringToBack(mGameSelector)`, which
> moves the widget to **index 0** (drawn first, then covered by every subsequent
> widget). The loader/transition overlay widget was therefore drawn ON TOP,
> showing a solid-colour frame (the purple tint came from stale GL vertex colour
> lingering from that overlay's draw). *Fix: changed to `BringToFront` so
> GameSelector is drawn last / on top.*
>
> **FIX 5 — `CopyImageToTextureSub` + `glTexSubImage2DOES` on MBX produces
> garbage.** N95 PowerVR MBX driver has `glTexSubImage2DOES` available (function
> pointer non-null), but calling it with an NPOT sub-region results in garbage
> pixels. `CopyImageToTextureSub` now always returns `EFalse`, forcing
> `GetOrCreateTexture` to use the manual full-POT `glTexImage2D` fallback with
> zero-padding and row-by-row copy.
>
> **FIX 6 — Missing `ArgbToRgba` in POT fallback swaps R↔B.**
> MemoryImage stores pixels as `0xAARRGGBB` (little-endian word → memory bytes
> B,G,R,A). `GL_RGBA` + `GL_UNSIGNED_BYTE` expects memory bytes R,G,B,A. The
> fallback must call `GLInterface::ArgbToRgba()` to swap R and B; without it,
> blue channels become red and vice versa (sky turns yellow, red turns blue).
> *This was removed in a prior experiment (v6) and re-added once confirmed.*
>
> **FIX 7 — Purple tint = `GL_MODULATE` × stale vertex colour.**
> `GLInterface::Init` sets `GL_TEXTURE_ENV_MODE` to `GL_MODULATE`, which
> multiplies the texture by the current vertex colour. If `SetColor` was last
> called with magenta (255,0,255) by an earlier widget or a failure-path FillRect,
> the texture renders purple. *Fix: explicitly call `g->SetColor(255,255,255,255)`
> right before every `DrawImage` call in `GameSelector::Draw`.*
>
> **RESULT on device (all verified):** titlescreen at full canvas scale (400x300),
> correct colours (blue sky, green grass, red logo), no flicker, stable 30 FPS.
> This is the first time real game art has rendered correctly on the N95.
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

---

## Current status (2026-06-28, session 10) — upstream analysis + diagnostic logging

### User feedback (session 10)

1. PopCap logo not showing on empty screen
2. Loading bar grass still stretching (not "unrolling")
3. After click: purple screen, then menu BG + (wrongly) PvZ logo
4. Should have: tombstone with buttons, tree, vase-buttons, achievement pot
5. Quit click crashes

### Upstream loading flow analysis

The original PvZ (PvZ-Portable) loads resources like this:

```
LawnApp::StartLoadingThread()
  └─ LoadingThreadProc() [background thread]
       ├─ ExtractLoaderBarResources()  // loadbar dirt+grass, pvz logo
       ├─ TodStringListLoad("LawnStrings.txt")
       ├─ LoadProperties("default.xml")
       ├─ LoadGroup("LoadingImages", 9)      // ~50 images
       ├─ LoadGroup("LoadingFonts", 54)      // font .dat files
       ├─ ExtractResourcesByName("DelayLoad_Background1")  // lawn
       ├─ mMusic->MusicInit()
       ├─ TrailLoadDefinitions()
       ├─ TodParticleLoadDefinitions()
       └─ PreloadForUser()
     // thread completes → mLoadingThreadCompleted = true
     // TitleScreen shows progress bar based on mCompletedLoadingThreadTasks
     // user clicks → LoadingCompleted() → ShowGameSelector()

GameSelector::GameSelector()
  ├─ AddReanimation(REANIM_SELECTOR_SCREEN)  // creates Reanimation runtime
  ├─ PlayReanim("anim_open")                  // plays the open animation
  ├─ AssignRenderGroupToTrack("SelectorScreen_BG", 1)  // BG = group 1
  ├─ AddReanimation × 6 (clouds)
  ├─ AddReanimation × 3 (flowers)
  ├─ AddReanimation × 1 (leaves)
  └─ MakeNewButton × N (with sprite images)

GameSelector::Draw(g)
  ├─ aSelectorReanim->DrawRenderGroup(g, 1)     // draw BG group
  ├─ for each cloud: cloudReanim->Draw(g)
  ├─ aSelectorReanim->DrawRenderGroup(g, 0)     // draw rest
  └─ (buttons draw themselves via WidgetManager)
```

### Key insight: upstream uses Reanimation runtime, NOT static images

The upstream menu is ANIMATED:
- `anim_open` plays when the menu appears (tombstone rolls in)
- `anim_cloud1-7` play on loop (clouds drift across)
- `anim_flower1-3` play on loop (flowers sway)
- `anim_grass` plays on loop (leaves rustle)

The Reanimation runtime (`Reanimator.cpp`, 1501 lines) interpolates
transforms between keyframes and draws each track with a matrix transform
(`ReanimBltMatrix` — uses `TodTriangleGroup` for textured triangles).

### What the port has vs what it needs

| Component | Upstream | Port | Gap |
|-----------|----------|------|-----|
| ReanimLoader | Definition.cpp | ✅ Works (48 tracks) | — |
| Reanimation runtime | Reanimator.cpp (1501) | ❌ Stub (ReanimPlayer is partial) | Need full port |
| ReanimBltMatrix | Matrix transform draw | ❌ Not ported | Need TodTriangleGroup |
| AddReanimation | EffectSystem.cpp (541) | ❌ Not ported | Need EffectSystem |
| PlayReanim | Reanimation::PlayReanim | ❌ Not ported | Need layer/frame system |
| DrawRenderGroup | Reanimation::DrawRenderGroup | ❌ Not ported | Need render groups |
| GameSelector buttons | NewLawnButton + sprites | ❌ Not ported | Need DialogButton |

### Session 10 fixes

1. **Load PopCap logo + loadbar early** (in PvZAppUi::ConstructL, before
   LoadingThreadProc) so the intro + loading bar render on the first frame.
2. **Remove PvZ logo from menu** (user: "should not be there"). The menu
   should show the reanim BG (tombstone scene), not a logo overlay.
3. **Diagnostic logging in GetOrCreateTexture** — log first 10 texture
   creations to gl_log.txt. The old logging cutoff was 3, so we couldn't
   see if SelectorScreen_BG got a texture.
4. **Diagnostic logging in GameSelector::Draw** — log BG image dimensions
   + bits pointer to gs_log.txt.

### What we need to find out from the next on-device test

The `gl_log.txt` will now show `GOT: WxH bits=PTR pot=POTWxPOTH` for the
first 10 textures. This tells us:
- Is `GetOrCreateTexture` even called for SelectorScreen_BG?
- Are the image bits valid (non-NULL)?
- What POT size is being allocated?

The `gs_log.txt` will show `GS:Draw BG WxH bits=PTR` — confirming the BG
image pointer and dimensions.

### The real path forward: port Reanimator.cpp

The menu will NEVER look right without the full Reanimation runtime. The
upstream menu is an animated reanim, not a static image. The port needs:

1. **Reanimator.cpp** (1501 lines) — the runtime that plays animations
2. **EffectSystem.cpp** (541 lines) — AddReanimation/ReanimationGet
3. **TodTriangleGroup** — textured triangle rendering for matrix transforms
4. **ReanimationLawn.cpp** (438 lines) — PvZ-specific reanim setup

Without these, the menu is a static BG image at best.

### Build/test history (session 10)

| Commit | Description | Result |
|--------|-------------|--------|
| (this) | Early PopCap/loadbar load + remove logo from menu + diagnostics | Pending on-device build |

---

## Current status (2026-06-28, session 9) — direct image draw + PopCap logo + plan

### What was done this session

#### Fix 1: Bypass ReanimPlayer — direct image draw for menu background

**Problem:** ReanimPlayer::Draw was called (RP:track diagnostic fired), 34
images were preloaded, but `gl_log` showed ZERO new GL textures created.
DrawImage was never reaching GetOrCreateTexture. Root cause unclear
(GetCurrentTransform struct copy? mImage NULL despite preload?).

**Fix:** `GameSelector::Draw` now bypasses ReanimPlayer entirely. It directly
calls `ResourceManager::GetImage("IMAGE_REANIM_SELECTORSCREEN_BG")` and
`Graphics::DrawImage(mem, 0, 0, mWidth, mHeight)` to draw the menu background
(graveyard scene). This is the most reliable path — same as the TitleScreen
background draw that already works. Also draws IMAGE_PVZ_LOGO + "Click to
Begin" text on top.

#### Fix 2: Loading bar animation — revert to stretch (clip rect didn't work)

**Problem:** The SetClipRect approach (session 7) didn't work on the N95 MBX
driver — glScissor either wasn't supported or worked differently, so the grass
bar never appeared.

**Fix:** Reverted to the simple stretch approach: `DrawImage(grassMem, barX,
barY, curW, barH)` where curW grows from 0 to mTotalBarWidth. The grass is
horizontally stretched (slightly distorted) but the animation IS visible.

#### Fix 3: PopCap logo intro (like the original PvZ)

Added a 3-phase PopCap logo intro to TitleScreen:
- Phase 0: fade in (30 frames, ~1s)
- Phase 1: hold (30 frames, ~1s)
- Phase 2: fade out (30 frames, ~1s)
- Phase 3: loading screen (progress bar + Click to Start)

Uses `IMAGE_POPCAP_LOGO` on a black background, centered. Any key press
skips the intro (jumps to phase 3). The loading state machine frame count
was increased from 60 to 150 (90 for logo + 60 for bar).

Note: alpha fade isn't implemented (Graphics::SetColorizeImages is a stub),
so the logo appears/disappears instantly rather than fading. The fade will
work once per-draw alpha is added (Stage 4 ImageFont/Graphics port).

### Port vs upstream comparison — comprehensive status

#### Engine systems (what's ported vs stubbed)

| System | Upstream | Port | Status |
|--------|----------|------|--------|
| PAK VFS | main.pak reader | `PvZVfs.cpp` | ✅ Working (3198 files, XOR 0xF7) |
| Image decode | SDL_image | ICL (CImageDecoder) | ✅ Working (PNG + JPEG) |
| GL rendering | SDL GL | EGL/GLES 1.1 | ✅ Working (30 FPS, landscape) |
| Texture cache | — | `Graphics.cpp` | ✅ Working (POT, eviction deletes GL tex) |
| ReanimLoader | Definition.cpp (1445 lines) | `ReanimLoader.cpp` | ✅ XML parser works (48 tracks) |
| ReanimPlayer | Reanimator.cpp (1501 lines) | `ReanimLoader.cpp` | ⚠️ Partial (interp OK, Draw unreliable on device) |
| ResourceManager | resources.xml parser | `ResourceManager.cpp` | ✅ Working (on-demand GetImage) |
| SystemFont | ImageFont (1748 lines) | `SystemFont.cpp` | ⚠️ 8x8 bitmap fallback, not real PvZ fonts |
| WidgetManager | WidgetManager | `widgetmanager.cpp` | ✅ Working |
| Input (d-pad) | — | `PvZAppUi.cpp` | ✅ Working (cursor + click) |
| Sound/Music | — | stub | ❌ Not ported (Stage 4) |
| SaveGame | — | stub | ❌ Not ported (Stage 4) |

#### Game screens (what's playable vs stubbed)

| Screen | Upstream | Port | Status |
|--------|----------|------|--------|
| PopCap logo | TitleScreen.cpp state machine | `TitleScreen.cpp` | ✅ Phase 0-2 (no fade) |
| Loading screen | TitleScreen.cpp | `TitleScreen.cpp` | ✅ BG + bar + Click to Start |
| Main menu | GameSelector.cpp (1509 lines) | `GameSelector.cpp` | ⚠️ Direct BG draw, no button sprites |
| Seed chooser | SeedChooserScreen.cpp (1158) | — | ❌ Not ported |
| Gameplay (Board) | Board.cpp (6364) | `board.cpp` | ❌ Stub (no plants/zombies/sun) |
| Cutscenes | CutScene.cpp | — | ❌ Not ported |
| Store | StoreScreen.cpp (1187) | — | ❌ Not ported |
| Almanac | AlmanacDialog.cpp (718) | — | ❌ Not ported |
| Zen Garden | ZenGarden.cpp | — | ❌ Not ported |
| Options | NewOptionsDialog.cpp | — | ❌ Not ported |
| Award screen | AwardScreen.cpp | — | ❌ Not ported |
| Credits | CreditScreen.cpp | — | ❌ Not ported |

#### Game objects (what's implemented vs stubbed)

| Object | Upstream | Port | Status |
|--------|----------|------|--------|
| Plants | Plant.cpp (all plants) | `plant.cpp` | ❌ Stubs only |
| Zombies | Zombie.cpp (all types) | `Zombie.cpp` | ❌ Stubs only |
| Projectiles | Projectile.cpp | `Projectile.cpp` | ❌ Stubs only |
| Coins | Coin.cpp | `Coin.cpp` | ❌ Stubs only |
| Lawn mowers | LawnMower.cpp | `LawnMower.cpp` | ❌ Stubs only |
| Grid items | GridItem.cpp | `GridItem.cpp` | ❌ Stubs only |
| Reanimator runtime | Reanimator.cpp (1501) | `Reanimator.h` | ❌ Stubs (ReanimPlayer is separate) |
| Particle system | TodParticle.cpp (1290) | — | ❌ Not ported |
| Effect system | EffectSystem.cpp (541) | — | ❌ Not ported |
| Challenge (minigames) | Challenge.cpp | `Challenge.cpp` | ❌ Stubs only |

### Next steps (priority order)

1. **Verify direct BG draw works on device** — the SelectorScreen_BG image
   should now render (same path as TitleScreen BG which already works).
2. **Port Reanimator.cpp runtime** (1501 lines) — needed for animated plants/
   zombies/UI. ReanimPlayer is a lightweight subset; the full runtime has
   render groups, attachments, blend layers, skew matrices, TodTriangleGroup.
3. **Port Board.cpp** (6364 lines) — the gameplay field. Needs Reanimator +
   Plant + Zombie + Projectile + Coin + LawnMower + GridItem.
4. **Port ImageFont.cpp** (1748 lines) — real PvZ fonts (replaces SystemFont).
5. **Port remaining screens** (SeedChooser, Store, Almanac, etc.)

### Build/test history (session 9)

| Commit | Description | Result |
|--------|-------------|--------|
| (this) | Direct BG draw + loading bar stretch + PopCap logo intro | Pending on-device build |

---

## Current status (2026-06-28, session 6) — real reanim menu + bug fixes

### On-device diagnosis (from uploaded logs)

The session-5 build booted correctly (no crash, `LoadingCompleted returned
OK`), reanim loaded (`48 tracks, FPS=20`), all track names dumped to gs_log.
But the user saw **lawn background + 10 stub buttons** instead of the reanim
menu. Two root causes found:

#### Bug 1: `mImageName` not copied in `GetCurrentTransform` (CRITICAL)

`ReanimPlayer::GetCurrentTransform` copied `mImage`, `mFontName`, `mText`
from the active keyframe to the output transform, but **NOT `mImageName`**
(added in session 5). So `ReanimPlayer::Draw` always saw `mImageName == ""`
→ the lazy-load branch never fired → no images were ever resolved →
`ReanimPlayer::Draw` rendered nothing → the lawn background showed through
(because session-5 `GameSelector::Draw` draws lawn first as a base layer).

**Fix:** added `aOut.mImageName = a.mImageName;` in the interpolated branch
of `GetCurrentTransform`. (The single-transform and clamp branches use
`aOut = track.mTransforms[i]` which copies the whole struct, so they were
already correct.)

**Effect:** `Draw` now sees the image name, calls `GetImage`, resolves the
`Image*`, caches it in the keyframe, and renders it. The SelectorScreen_BG
track now actually draws the graveyard background, button sprites appear,
etc.

#### Bug 2: 10 stub GameButton widgets drawn on top of reanim

Even with images loading, the 10 stub `GameButton` widgets (created in the
old constructor, added to the `WidgetManager` as top-level widgets) were
drawn ON TOP of the reanim menu. They covered the reanim button sprites
with beige rectangles + text labels.

**Fix:** **rewrote `GameSelector` completely** — no more stub buttons:
- Constructor: just loads the reanim + binds `ReanimPlayer`. No
  `GameButton` widgets, no `ToolTipWidget`, no `LayoutButtons`.
- `Draw`: renders the reanim ONLY (SelectorScreen_BG covers the full
  canvas; no lawn fallback needed when reanim loads). Falls back to
  lawn+title only if reanim fails to load.
- `MouseDown`: hit-tests the click against each reanim button track's
  `transform[0]` position + image size (×0.5 for 800×600 → 400×300). Maps
  track names to button IDs:
  - `SelectorScreen_Adventure_button` → Adventure (100)
  - `SelectorScreen_Survival_button` → Survival (110) [locked]
  - `SelectorScreen_Challenges_button` → Minigames (101) [locked]
  - `SelectorScreen_ZenGarden_button` → Zen Garden (109) [locked]
  - `SelectorScreen_StartAdventure_button` → Adventure (100)
  - Options/Help/Quit use fixed canvas positions (bottom woodsign area)
- `mMouseVisible = true` so `WidgetManager::FindWidget` routes clicks to
  GameSelector (was `false` before, which is why clicks never reached it).
- `ButtonDepressed` / `KeyDown` routing unchanged (Options/Quit work;
  Adventure/Store/etc. log "not yet ported").

#### Bug 3: Loading bar rendered as a distorted strip

`TitleScreen::Draw` drew the grass bar by **stretching** it to
`(mCurBarWidth, barH)` — a horizontal squish that made it look like a
thin strip instead of a growing grass section.

**Fix:** use `Graphics::SetClipRect` + `ApplyClipRect` to clip the grass
to `(barX, barY, curW, barH)`, then draw it at full bar width. The clip
rect hides the right portion; the grass keeps its native aspect. (The
port's `ClipRect` is implemented in `Graphics.cpp`; `DrawImage` with
`srcRect` was a stub, which is why the old code stretched instead.)

### Reanim track inventory (from gs_log, 48 tracks)

| # | Track name | Purpose |
|---|-----------|---------|
| 0-7 | anim_open/sign/idle/grass/flower1-3/start | animation layers |
| 8-13 | anim_cloud1-7 | cloud animation layers |
| 14 | **SelectorScreen_BG** | full background |
| 15-20 | Cloud1-7 | cloud sprites |
| 21-23 | SelectorScreen_BG_Center/Left/Right | bg parts |
| 24 | almanac_key_shadow | almanac key shadow |
| 25-34 | SelectorScreen_*_button / _shadow | button sprites + shadows |
| 35-41 | leaf1-5, leaf22, leaf_SelectorScreen_Leaves | leaf decorations |
| 42-44 | flower1-3 | flower decorations |
| 45-47 | woodsign1-3 | wooden signs (Options/Help/Quit) |

### Expected on-device behaviour after session 6

1. Loading screen: grass bar grows with correct aspect (no strip)
2. Click-to-start → menu appears: **SelectorScreen_BG** (graveyard scene)
   fills the canvas, wooden signs + button sprites visible
3. No 10 stub buttons — the menu IS the reanim
4. D-pad cursor visible; centre key clicks reanim buttons (Adventure etc.)
5. Options/Help/Quit click zones work (bottom woodsign area)

### What is NOT done yet (carried forward)

- Adventure click → `PreNewGame` still logs "gameplay not yet ported"
  (Stage 2: Board/Plant/Zombie port).
- Store/Almanac/ZenGarden/Challenge click → "not yet ported" (Stage 3).
- The reanim is **static** (duration=0, all 706 transforms at frame 0) —
  clouds/flowers don't drift yet. The `anim_*` tracks are animation
  *layers*, not timeline keyframes; playing them needs the full
  Reanimator runtime (Stage 2). For now the menu is a correct static
  1:1 render.
- Sound/music still stubbed (Stage 4).

### Build/test history (session 6)

| Commit | Description | Result |
|--------|-------------|--------|
| (this) | Fix mImageName copy + rewrite GameSelector (no stub buttons) + clip loading bar | Pending on-device build |

---

## Current status (2026-06-28, session 5) — purple screen + crash fixed

### On-device symptoms (from uploaded logs)

After the session-4 fixes, the on-device behaviour was:
1. Loading bar still rendered as a strip (pre-existing visual issue)
2. After click-to-start: **purple screen** with only the Quit button visible
   (top-left corner)
3. D-pad cursor no longer appeared
4. Pressing d-pad buttons → game closed (crash)

### Root cause analysis (from gs_log, boot.log, rmgr_log, draw_progress)

**The `LoadingCompleted()` state machine had a re-entry bug that caused
35× GameSelector construction → cascading OOM → purple screen → crash.**

1. `HandleKeyEventL` set `iLoadingState = 3` **AFTER** calling
   `LoadingCompleted()`. If `LoadingCompleted()` Leaves (Symbian OOM
   exception) or crashes, the `iLoadingState = 3` line is never reached.
2. The Symbian framework catches the Leave and dispatches the next queued
   key event. `iLoadingState` is still 2, so the state-2 block fires again.
3. Each re-entry creates a new `GameSelector` (10 buttons + reanim load).
   The gs_log confirmed: **35 "GS:ctor buttons created" lines, 0
   "GS:reanim loaded" lines** — the constructor never finished.
4. The rmgr_log confirmed: `IMAGE_REANIM_SELECTORSCREEN_CLOUD1` decode
   started (`[dec] DataNewL` → `frame 220x206` → `EColor16M + alpha mask`)
   but **never completed** — repeated 9+ times. OOM during ICL decode.
5. 35 constructors × 328KB XML buffer + 14 image decodes each → heap
   exhaustion → KERN-EXEC 3.
6. The purple screen was the GL clear colour showing through because
   `ReanimPlayer::Draw` rendered nothing (images not yet loaded) and then
   `return`ed, skipping the lawn background fallback.
7. The draw_progress confirmed: 35 sets of 11 widgets (different pointers)
   = 35 GameSelectors, all buttons at (0,0) because `ShowGameSelector`'s
   `Resize` line was never reached (constructor Left first).

### Fixes (3 commits)

#### Fix 1: State machine — set state BEFORE the call + TRAP (`PvZAppUi.cpp`)

```cpp
// BEFORE (broken):
iLawnApp->LoadingCompleted();  // ← Leaves here on OOM
iLoadingState = 3;             // ← never reached

// AFTER (fixed):
iLoadingState = 3;             // ← one-shot transition, always runs
TRAPD(err, iLawnApp->LoadingCompleted());  // ← Leave caught
```

This makes the click-to-start → menu transition **one-shot**: regardless of
whether `LoadingCompleted` succeeds, Leaves, or crashes, `iLoadingState` is
already 3 so no queued key event can re-trigger it. The TRAP also ensures
`HandleKeyEventL` itself never Leaves (which would propagate to the
framework's `RunL` and could destabilise the app).

#### Fix 2: Defer reanim image loading to first Draw (`ReanimLoader.h/.cpp`)

**Root cause of the OOM:** `ReanimLoadCompiled` called
`gResourceManager->GetImage(imgName)` for **every** `<i>` tag during
parsing — 14+ ICL image decodes (each opening a `CImageDecoder` +
allocating `CFbsBitmap` for color + mask + ARGB output) all within the
`GameSelector` constructor. This is what caused the OOM → Leave →
re-entry cascade.

**Fix:** the parser now **stores the image name string only** (`mImageName`
field added to `ReanimTransform`) and sets `mImage = NULL`. The
`ReanimPlayer::Draw` method **lazily resolves** the name to an `Image*`
via `ResourceManager::GetImage` on first render. Benefits:
- The constructor does zero image decoding (just XML parsing + array alloc)
- Images load one-per-frame (spread across many frames, no OOM spike)
- The `ResourceManager` cache prevents re-decoding the same image
- If an image fails to decode, `GetImage` returns NULL and it's cached as
  NULL in the keyframe so we don't retry every frame
- The lawn background always shows as a base (no purple screen)

#### Fix 3: TRAP the reanim load + NULL checks (`GameSelector.cpp`,
`ReanimLoader.cpp`)

- `GameSelector` constructor wraps `ReanimLoadCompiled` in `TRAPD` — if it
  Leaves, the constructor completes with `mReanimLoaded = false` and falls
  back to the lawn+buttons menu (still functional, just not animated reanim).
- `ReanimLoadCompiled` checks `new` results for NULL (this port's `operator
  new` returns NULL on OOM, not throws/Leaves — see `newdel_compat.cpp`).
  Previously a NULL `new ReanimTrack[48]` would be `memset`'d → KERN-EXEC 3.

#### Fix 4: Always draw lawn background before reanim (`GameSelector.cpp`)

`GameSelector::Draw` now **always** draws `IMAGE_BACKGROUND1` (or
`IMAGE_TITLESCREEN` fallback, or dark-green fill last-resort) as the base
layer, then draws the reanim on top. Previously, if the reanim loaded but
its images hadn't been lazy-resolved yet (first few frames),
`ReanimPlayer::Draw` rendered nothing and the early `return` left the
screen showing the purple GL clear colour. Now the lawn always shows; the
`SelectorScreen_BG` track covers it once that image loads (a few frames
in).

### Expected on-device behaviour after these fixes

1. Click-to-start transitions to the menu **once** (no 35× re-entry)
2. The lawn background is always visible (no purple screen)
3. Reanim sprites appear progressively over the first ~14 frames as their
   images lazy-load (background → gravestone → signs → buttons → clouds)
4. The menu animates (clouds drift, flowers sway) via `ReanimPlayer`
5. The 10 stub buttons are positioned by `LayoutButtons` (now that the
   constructor completes and `ShowGameSelector`'s `Resize` runs)
6. No crash on d-pad input

### What is NOT fixed yet (carried forward)

- The loading bar still renders as a strip (pre-existing TitleScreen visual
  issue — the grass-bar clip/scale logic needs fixing, separate from this
  crash).
- The 10 stub buttons still draw on top of the reanim menu (need to be
  replaced with reanim sprite hit-zones — next step, now that the track
  names are correctly dumped to gs_log).
- D-pad cursor visibility (the cursor was disabled by the crash cascade;
  should work now that the state machine is fixed, but needs on-device
  verification).

### Build/test history (session 5)

| Commit | Description | Result |
|--------|-------------|--------|
| (this) | Fix state machine re-entry + lazy image loading + TRAP + NULL checks | Pending on-device build |

---

## Current status (2026-06-28, session 4) — ReanimLoader XML parser fixed

### What was done this session

The **#1 blocker — ReanimLoader parsed only 1 track instead of ~48** — is
fixed, along with its direct consequence **#2 — reanim images never loaded**.

#### Fix 1: ReanimLoader XML parser rewrite (`src/Sexy.TodLib/ReanimLoader.cpp`)

**Root cause (confirmed):** the old `FindTag(buf, len, "/track", ...)` call used
to locate each closing `</track>` built an `openTag = "</track>"` and a
`closeTag = "<//track>"` (because `FindTag` prepends `</` to the tag name to
make the close pattern). Since `<//track>` never appears in the document,
`FindTag` returned NULL, the counting loop hit `else break;`, and **only the
first track was ever counted**. The subsequent per-track parse loop had the
same defect and also broke after track 0.

**Fix:** replaced the entire `FindTag`/`ParseFloatTag`/`ParseStringTag` trio
with a single correct `FindElement()` that:

- explicitly **skips closing tags** (`</…`) and comments/declarations (`<!`, `<?`),
- matches the tag name **exactly** and then requires the next char to be `>`
  (open), `/>` (self-close), or whitespace (attributes) — so `<t>` no longer
  matches `<track>` or `<text>`,
- builds the real `</tag>` close pattern and scans for it from the content
  start,
- returns the content range **and** a pointer past the whole element so the
  caller iterates by advancing past each element (no off-by-one, no re-matching),
- handles `<tag/>`, `<tag></tag>`, and `<tag attr="x">…</tag>`.

`<fps>`, every `<track>`, every `<name>`, and every `<t>` transform (with
`<x>/<y>/<kx>/<ky>/<sx>/<sy>/<f>/<a>/<i>/<font>/<text>`) are now all parsed.
`<i>NULL</i>` and missing `<i>` both leave `mImage = NULL` (upstream convention).

**Verification before porting:** the exact `FindElement` algorithm was
reproduced in a host-side C++ harness (`g++`) against a 51-track / 55-transform
sample `.reanim` covering: single-transform tracks, multi-transform animated
tracks, empty `<t></t>` transforms, font/text tracks, and image tracks. All
checks passed (parsed track count == ground-truth `<track>` count, transform
count == ground-truth `<t>` count, first track name correct, multi-transform
track had 3 transforms, font/text fields correct, empty transform counted as 1).
The Symbian version uses the identical logic with `TBool`/`TInt`/`User::Alloc`.

The public API (`ReanimLoadCompiled`, `ReanimFindTrack`, `ReanimDefinition` /
`ReanimTrack` / `ReanimTransform` structs) is unchanged, so `GameSelector.cpp`
needs no changes — it already calls `ReanimLoadCompiled` and logs the track
count + first 5 names. On-device `gs_log.txt` should now show **~48 tracks**
instead of 1.

#### Fix 2: `IMAGE_REANIM_*` → `reanim/` asset mapping (`ResourceManager.cpp`)

**Root cause:** `LoadImageByResName` stripped only the `IMAGE_` prefix, so
`IMAGE_REANIM_SELECTORSCREEN_BG` became stem `reanim_selectorscreen_bg` and was
probed as `reanim/reanim_selectorscreen_bg.png` — which never matches the real
PAK entry `reanim/SelectorScreen_BG.jpg`. So even after the parser found all
tracks, the `<i>IMAGE_REANIM_…</i>` image refs resolved to NULL.

**Fix:** `LoadImageByResName` now strips the **full `IMAGE_REANIM_` prefix**
first (leaving `SELECTORSCREEN_BG` → lowercased `selectorscreen_bg`), which the
case-insensitive PAK matches against `reanim/SelectorScreen_BG.{png,jpg}`. The
existing `kPrefixes` table already tries `reanim/` and `kExts` already tries
`.png` then `.jpg`, so no other change is needed. Non-reanim `IMAGE_*` assets
keep stripping just `IMAGE_`.

#### Add: `ReanimPlayer` lightweight reanimation runtime (`ReanimLoader.h/.cpp`)

The port's `Reanimator.h` `Reanimation` class is fully stubbed (`Draw`/`Update`
are no-ops), and its `ReanimatorDefinition`/`ReanimatorTransform` structs are
incompatible with the real parsed data in `ReanimLoader.h`. Rather than attempt
a blind 1501-line port of upstream `Reanimator.cpp` (risky without on-device
compilation), a focused `ReanimPlayer` class was added that operates on the
now-correctly-parsed `ReanimDefinition`:

- `SetDefinition(ReanimDefinition*)` — bind + reset playback.
- `Update(float dtSeconds)` — advance `mAnimTime`, loop or clamp at end.
- `GetCurrentTransform(trackIndex, &out)` — linear interpolation between the
  two surrounding keyframes (frame = `mAnimTime * mFPS`); clamps at both ends;
  single-transform tracks return that transform; image/font/text come from the
  active ("from") keyframe (upstream behaviour — discrete swaps on keyframes).
- `Draw(Graphics*)` — renders every track's image at the interpolated
  trans/scale, mapped from reanim's 800×600 space to the 400×300 canvas via
  `mCoordScale` (default 0.5).
- `FindTrackIndex(name)` — case-insensitive, returns -1 if absent.

**Interpolation logic verified host-side** (`test_interp.cpp`): 2-transform
track lerps correctly at frame 0/2.5/5/10 and clamps before/after; single-
transform and empty tracks handled; `animTime * FPS` → frame conversion
confirmed. All checks passed before porting.

**Wired into `GameSelector`:** the constructor binds `mReanimPlayer` to
`&mReanimDef` with `LOOP_ON`; `Update()` advances it by `1/30 s` per frame;
`Draw()` calls `mReanimPlayer.Draw(g)` instead of the old static `transform[0]`
loop. So the menu now **animates** (clouds drift, flowers sway) rather than
showing a frozen frame-0 snapshot.

**Not yet implemented (deliberate scope):** render groups, attachments, blend
layers, color override, filter effects, skew matrices, TodTriangleGroup
textured-triangle rendering, and per-draw alpha modulation (the port's
`Graphics::SetColorizeImages` is a stub). These arrive with the full
`Reanimator.cpp` / `EffectSystem.cpp` Stage-2 ports.

#### Fix 3: SystemFont `)` glyph + button-label brackets

- **`)` glyph bug:** `kGlyphs8x8[')']` was a byte-for-byte copy of `(` with a
  comment claiming "mirror handled in draw" — but `DrawString` has no mirror
  logic, so `)` rendered as `(`. Fixed to the correct horizontal mirror
  (`{0x70,0xD8,0x18,0x18,0x18,0xD8,0x70,0x00}`). The other 95 glyphs are the
  standard public-domain 8×8 set and were already correct; the "garbled text"
  symptom was actually the bracket prefix (below), not a glyph bug.
- **Button-label brackets:** the 10 stub `GameButton`s were labelled
  `"[Adventure]"`, `"[Survival]"`, etc. The leading `[` (ASCII 91) rendered as
  a bracket shape that read as a "mirrored-L prefix" — this was misdiagnosed in
  the session-3 notes as a char-mapping bug. Removed the brackets (labels are
  now `"Adventure"`, `"Survival"`, …), which is also more 1:1: upstream uses
  empty labels with text baked into the sprite images.

Also: the GameSelector constructor now dumps **every** reanim track name +
transform count to `gs_log.txt` (was only the first 5), so the next session can
map reanim tracks to menu buttons for 1:1 hit-zone positioning.

### Effect on the menu (expected on-device)

With both fixes + ReanimPlayer, `GameSelector::Draw` now renders every track
that has an image at its interpolated position (background, gravestone, wooden
signs, button sprites, clouds, flowers, logo, etc.), scaled from the reanim's
800×600 space to the 400×300 canvas, and **animates** over time. The
lawn+10-rect-button stub is still the fallback if `mReanimLoaded` is false, but
should no longer trigger.

### What is NOT done yet (carried forward)

- The 10 stub `GameButton` widgets are still created and added to the
  `WidgetManager` — they draw on top of the reanim menu. The next step is to
  port upstream `GameSelector` 1:1: drop the rect buttons, use the reanim
  button sprites (with their polygon hit-shapes) for hit-testing, and wire
  `ButtonDepress` → `PreNewGame` etc. (needs the button track names, which
  `gs_log` will now dump correctly once the parser runs on-device).
- `TitleScreen` state machine (PopCap logo → partner logo → SODROLLCAP) is
  still the simplified loading screen, not the 1:1 upstream port.
- `SystemFont` 8×8 glyphs are still garbled (encoding/offset bug in
  `kGlyphs8x8`) — Stage 4 `ImageFont` port will replace this.
- Gameplay (Board / Plant / Zombie / full Reanimator runtime / particles) is
  Stage 2.

### Build/test history (session 4)

| Commit | Description | Result |
|--------|-------------|--------|
| `6a07bda` | Rewrite ReanimLoader `FindTag`→`FindElement`; fix `IMAGE_REANIM_` mapping | Parser verified host-side; pending on-device build |
| (this) | Add `ReanimPlayer` runtime (interpolation+Draw); wire into GameSelector | Interpolation verified host-side; menu now animates |

---

## Current status (2026-06-28, session 3)

### What works

- **Build**: GCCE 3.4.3 compiles & links, SIS produced. miniz (zlib) integrated.
- **Boot**: EGL/GLES 1.1 context, landscape 320x240, 30 FPS stable.
- **PAK VFS**: main.pak (45MB, 3198 files) loaded, XOR 0xF7 decrypted.
  Case-insensitive + path-separator-normalised file lookup.
- **Loading screen**: IMAGE_TITLESCREEN bg + IMAGE_PVZ_LOGO (transparent via
  colorkey) + IMAGE_LOADBAR_DIRT/GRASS progress bar + "Click to Start" text.
  State machine: 60-frame animation → wait for click → LoadingCompleted.
- **Main menu**: 10 GameButton widgets + tooltip. d-pad cursor (32px/press).
  All 10 buttons clickable (gs_log confirms all ButtonDepress IDs 100-114).
  Text renders via SystemFont (8x8 pixel-rect glyphs).
- **Fonts**: SystemFont (hardcoded 8x8 ASCII bitmap). GetFontThrow returns
  SystemFont::Get() for all FONT_* globals. Graphics::DrawString dispatches
  to mFont->DrawString. Text is visible but glyphs are wrong (encoding issue:
  labels show mirrored-L prefix + garbled chars — likely a char mapping bug
  in SystemFont or GameButton passing wrong string).
- **ReanimLoader**: XML parser for .reanim files. Successfully loads
  reanim/SelectorScreen.reanim (328KB XML). Parsed 1 track (anim_open, 706
  transforms) — BUT the XML parser has a bug: it counts <track> tags wrong
  (finds </track> as a separate track, so only gets 1 real track instead
  of ~48). Also no reanim images loaded (image name tags not found because
  parser only checks track[0]).
- **Colorkey**: MemoryImage::ApplyColorKey for JPEG logos (black→transparent).
- **IMAGE_BACKGROUND1**: loaded from images/background1.jpg (171KB in PAK).
- **_Font typedef**: `typedef Sexy::Font _Font` unified across all headers.

### Current blockers (blocking 1:1 menu)

1. **REANIM XML PARSER BUG** — only finds 1 track instead of ~48.
   The FindTag function matches `</track>` as a `<track>` tag because it
   doesn't exclude tags starting with `/`. Also, the parser's `<t>` counting
   is fragile — it needs to properly handle nested tags and self-closing
   tags. The `track[-2] == '/'` check is wrong (should check the open tag
   string itself, not the character before content).
   **Fix**: rewrite FindTag to properly distinguish `<track>` from `</track>`.
   Or use a different parsing approach (scan for `<track>` and `</track>`
   literally, not via generic FindTag).

2. **REANIM IMAGES NOT LOADED** — gl_log shows only 3 textures (titlescreen
   + 2 loadbar). No IMAGE_REANIM_SELECTORSCREEN_* loaded. This is because:
   (a) the parser only found 1 track (anim_open) which has no `<i>` image tags,
   (b) even for tracks with images, the `<i>` tag search may not work because
   FindTag doesn't handle the `i` tag correctly (matches `<i>` inside other
   tags like `<img>`).
   **Fix**: once the parser finds all 48 tracks, tracks like
   `SelectorScreen_BG` will have `<i>IMAGE_REANIM_SELECTORSCREEN_BG</i>`
   tags that load the background image.

3. **MENU STILL SHOWS LAWN+BUTTONS** — because reanim only parsed 1 track
   with no images, GameSelector::Draw reanim path draws nothing, falls through
   to IMAGE_BACKGROUND1 (lawn) fallback. The purple tint is from the GL clear
   color (0.15, 0.05, 0.20) showing through when no background image is drawn.

4. **TEXT GLYPHS WRONG** — labels show garbled characters. SystemFont uses
   hardcoded 8x8 bitmap glyphs, but the char mapping may be offset (the `L`
   prefix suggests the `[` bracket char (ASCII 91) is being rendered instead
   of the first letter, or the label string has a `[` prefix from
   GameButton::SetLabel which passes `"[Adventure]"` including brackets).

5. **UPSTREAM CODE NOT PORTED 1:1** — the current GameSelector/GameButton/
   TitleScreen are simplified approximations, NOT 1:1 ports from upstream
   PvZ-Portable. The upstream GameSelector uses Reanimation for background +
   button sprites, with track transforms for positioning. The port needs to
   either:
   (a) Port the full Reanimation engine (Reanimator.cpp 1501 lines +
       EffectSystem.cpp 541 lines + ReanimationLawn.cpp 438 lines), OR
   (b) Use ReanimLoader to parse track transforms and render static frames
       (current approach — simpler but not animated).

### 1:1 port roadmap (next session priorities)

1. **FIX REANIM XML PARSER** — fix FindTag to properly handle `</tag>` vs
   `<tag>`. Should find all 48 tracks in SelectorScreen.reanim. Verify with
   gs_log track name dump.

2. **FIX REANIM IMAGE LOADING** — once all tracks parsed, tracks with `<i>`
   tags will load images via ResourceManager::GetImage. Verify gl_log shows
   new textures (IMAGE_REANIM_SELECTORSCREEN_BG etc.).

3. **FIX GameSelector::Draw reanim rendering** — render all tracks with
   images at their transform[0] positions. Coordinates are in 800x600 space,
   scale to 400x300 (×0.5). Draw in track order (background first, then
   buttons, then clouds/flowers on top).

4. **REMOVE STUB MENU** — remove the 10 beige GameButton widgets and the
   IMAGE_BACKGROUND1 fallback. Replace with reanim-rendered menu where
   button sprites from the reanim are used for hit-testing.

5. **PORT UPSTREAM GameSelector.cpp 1:1** — port the constructor (creates
   NewLawnButton with IMAGE_REANIM_SELECTORSCREEN_* sprites), Draw (uses
   Reanimation for background + buttons), ButtonDepress (Adventure → zombie
   hand animation → PreNewGame).

6. **FIX TEXT ENCODING** — SystemFont glyphs are wrong. Either fix the
   char mapping in SystemFont (kGlyphs8x8 array may be offset), or remove
   the `[` brackets from button labels.

7. **PORT UPSTREAM TitleScreen.cpp 1:1** — state machine with PopCap logo
   fade, partner logo, SODROLLCAP animation on loading bar.

### Build/test history (session 3)

| Commit | Description | Result |
|--------|-------------|--------|
| `bed8c6e` | Colorkey for JPEG logos | Logo transparent ✓ |
| `6ca99f2` | SystemFont + _Font typedef + GetFontThrow | Build fixes (many) |
| `b31acc8` | SystemFont pixel-rect rendering | Text renders (wrong glyphs) |
| `153ca61` | Load IMAGE_BACKGROUND1 | Lawn background shown |
| `1451b15` | miniz + reanim/ prefix in LoadImageByResName | miniz builds |
| `4215df3` | ReanimLoader (binary compiled parser) | Binary format incompatible |
| `b0a367a` | GameSelector loads + renders reanim | Crash (OOM/binary parse) |
| `0f2adfa` | stdint.h shim for GCCE | Build fix |
| `7ce7505` | extern C miniz + intptr_t fix | Build fix |
| `ff46907` | Remove intptr_t from stdint.h | Build fix |
| `ec4a639` | miniz_tinfl.c + miniz_tdef.c + malloc stubs | Link fix |
| `8467849` | size_t → unsigned int in malloc stubs | Build fix |
| `3f44363` | Remove duplicate extern C block | Build fix |
| `734c19e` | TRAP + User::Alloc for OOM safety | Still crashes |
| `f91e52d` | DISABLE reanim (causes crash) | Menu shows lawn+buttons |
| `11acad7` | Graphics::DrawString dispatches to mFont | Text appears (wrong glyphs) |
| `588ae4c` | Include Font.h in Graphics.cpp | Build fix |
| `6e8104e` | Rewrite binary parser (struct layout) | Still fails (64-bit structs) |
| `4c35f0c` | Switch to XML parser | Build fix needed |
| `edaf1f4` | strncpy + atof stubs | Build OK |
| `4c35f0c+edaf1f4` | XML reanim parser | Loaded 1 track (bug in FindTag) |

---

## Previous status (2026-06-28, session 2)

### Build/test history (session 2)

| Commit | Description | Result |
|--------|-------------|--------|
| `bed8c6e` | Colorkey for JPEG logos | Logo transparency ✓ |
| `6ca99f2` | SystemFont + _Font typedef + GetFontThrow | Build failed: namespace |
| `a906dff` | Fix: Sexy::SystemFont::Get() | Build failed: typedef conflict |
| `dc6ba25` | Fix: _Font typedef guard | Build failed: _Font undeclared |
| `0bf8cf9` | Fix: ResourceManager.h includes Font.h + typedef | Build failed: TodCommon.h class _Font |
| `4cc7fc3` | Fix: replace _Font forward-decls in all headers | Build failed: class _Font in TodDrawString |
| `6536862` | Fix: remove 'class' prefix from _Font* | Build failed: GameButton SystemFont |
| `eca0f16` | Fix: Sexy::SystemFont in GameButton | Build failed: FONT_BRIANNETOD16 undeclared |
| `fa9e338` | Fix: include Resources.h in lawnapp.cpp | Build failed: Stubs.h macro conflict |
| `9720777` | Fix: remove ALL IMAGE_* #define macros | Build OK, fonts render as grey rects |

---

## Previous status (2026-06-28, session 1)

### Approach correction: 1:1 port, not approximation

Previous sessions created **simplified approximations** of GameSelector,
GameButton, TitleScreen, ToolTipWidget instead of porting upstream code 1:1.
This is the wrong approach. The goal is a **1:1 port** of the upstream
PvZ-Portable engine. Future work must port upstream files verbatim, adapting
only platform-specific calls (SDL→Symbian, filesystem, etc.).

### What works (on-device verified)

- **Build**: GCCE 3.4.3 compiles & links, SIS produced.
- **Boot**: EGL/GLES 1.1 context, landscape 320x240, 30 FPS stable.
- **PAK VFS**: main.pak loaded, 118 images decode OK (PNG+JPEG via ICL).
- **Loading screen**: IMAGE_TITLESCREEN background + IMAGE_PVZ_LOGO +
  IMAGE_LOADBAR_DIRT/GRASS animated progress bar (~2s visible).
  LoaderBar group loads via `ExtractResourcesByName("LoaderBar")`.
- **Main menu**: 10 buttons (Adventure/Survival/Minigame/Puzzle/Store/
  Almanac/ZenGarden/Options/Help/Quit) on titlescreen background.
- **Input**: d-pad arrows move virtual cursor (32px/press, visible yellow
  crosshair), centre key/Enter = click, Escape = quit. Touch via
  `HandlePointerEventL` (320x240→400x300 coord mapping).
- **Widget count**: 12 widgets in manager (1 GameSelector + 10 buttons +
  1 tooltip) — verified via wgt_log.txt.
- **Click routing**: `WidgetManager::FindWidget` skips `mMouseVisible=false`
  widgets (GameSelector), so clicks reach buttons. `GameButton::MouseUp` →
  `ButtonDepressed` → `GameSelector::ButtonDepress` dispatch.

### What does NOT work yet (blocking issues)

1. **FONTS (M4 #4) — CRITICAL BLOCKER**:
   `GetFontThrow` is a no-op stub returning NULL. All `FONT_*` globals
   are NULL. No text renders — button labels, "Click to Start", tooltip
   text, title text all invisible. The port has two incompatible font types:
   - `_Font*` (opaque, forward-declared in Stubs.h) — type of FONT_* globals
   - `Sexy::Font*` (real class in engine/Font.h with StringWidth/DrawString)
   To fix: port upstream `ImageFont.cpp` (1748 lines) + `FontData`/`DescParser`
   (parses `.dat` font description files from PAK for char maps + metrics).
   OR: typedef `_Font = Sexy::Font` and implement `GetFontThrow` to load
   font image + create simplified ImageFont with hardcoded ASCII char map.

2. **LOGO BLACK BACKGROUND**:
   `IMAGE_PVZ_LOGO` loaded as `images/pvz_logo.jpg` (JPEG, no alpha channel).
   The PAK may also have `pvz_logo.png` (with alpha). `LoadImageByResName`
   tries `.jpg` BEFORE `.png` — picks the JPEG. Fix: reorder kExts to try
   `.png` first.

3. **"CLICK TO CONTINUE" MISSING**:
   Loading screen auto-advances to menu after 60 frames. Original PvZ shows
   "Click to Start" hyperlink after bar fills; user must click to advance.
   Need to port upstream `TitleScreen` state machine (TITLESTATE_SCREEN with
   HyperlinkWidget).

4. **MENU NOT 1:1**:
   Current menu is 10 beige rect buttons on titlescreen background. Original
   PvZ menu has: lawn background (IMAGE_BACKGROUND1), tombstone/wooden sign
   with buttons rendered ON it, Reanimation clouds/flowers, particle effects.
   Need: port upstream `GameSelector.cpp` (1509 lines) verbatim + port
   `IMAGE_BACKGROUND1` tiling (M4 #3, 1400x600 → ≤1024 tiles).

5. **BUTTON CLICKS SILENT ON DISABLED BUTTONS**:
   Survival/Minigame/Puzzle/Store/Almanac/ZenGarden buttons are `mDisabled=true`
   because `HasFinishedAdventure()` returns false (default PlayerInfo).
   `GameButton::MouseUp` skips `ButtonDepressed` when disabled. This is
   correct behaviour (matches upstream) but confusing without labels.

6. **51 NULL IMAGE_/FONT_/SOUND_ symbols (M4 #5)**:
   150 of 268 resources not found in PAK. `IMAGE_BUTTON_LEFT/MIDDLE/RIGHT`
   (9-slice button texture) are NULL → buttons fall back to beige rects.
   `IMAGE_OPTIONS_MENUBACK` is NULL → DoNewOptions crashes on NULL->mWidth
   (guarded now but dialog won't render).

7. **149 REANIM_* not in PAK (M4 #6)**:
   Asset naming/packing mismatch. Reanimation images not found.

### Build/test history (this session)

| Commit | Description | Result |
|--------|-------------|--------|
| `5e01aeb` | M4 #1: GameSelector + GameButton + ToolTipWidget + d-pad input | Build failed: `MouseHitTest` missing |
| `d276e59` | Fix: restore `MouseHitTest` compat stubs | Build OK, but stuck on loading screen |
| `5092643` | Fix: use `Sexy::FONT_DWARVEN` instead of `_Font*` globals | Build OK, stale widget covering buttons |
| `a467804` | Fix: `BringToBack` instead of `BringToFront` | Buttons visible but purple tint + 1/4 crop |
| `461ddff` | Fix: nuke stale widgets (while-loop) | CRASHED: write to freed memory |
| `7f0d9b1` | Fix: revert while-loop, add diagnostic dump | App runs, stale widget still present |
| `5e0be58` | Fix: hard-reset `mWidgetCount=0` + visible cursor | Stale widget gone, but clicks crash |
| `83126fc` | Fix: PreNewGame NULL guard + hard-reset | Background correct, clicks crash deep |
| `de0759b` | Fix: disable crashy button handlers + default PlayerInfo | All buttons safe, no crashes |
| `b96108b` | Feat: loading screen with progress bar | Build failed: Image not declared |
| `26e265e` | Fix: include Resources.h for IMAGE_* | Build failed: Stubs.h macro conflict |
| `fa1d616` | Fix: avoid Stubs.h macro conflict | Build failed: ResourceManager undefined |
| `e36871e` | Fix: include ResourceManager.h | Build OK, loading screen visible |
| `7480c93` | Fix: `mMouseVisible=false` for GameSelector | Clicks still not reaching buttons |
| `670ff94` | Feat: visible loading + cursor on Adventure + 32px d-pad | Loading visible, clicks work on Adventure |

### Next priorities (1:1 port roadmap)

1. **Fix .png before .jpg** in `LoadImageByResName` — fixes logo transparency
2. **Port fonts** — typedef `_Font = Sexy::Font`, implement `GetFontThrow` with
   ImageFont (load font image from PAK, hardcoded ASCII char map)
3. **Port upstream TitleScreen.cpp** — state machine with "Click to Start"
4. **Port upstream GameSelector.cpp 1:1** — tombstone background, proper layout
5. **Port IMAGE_BACKGROUND1 tiling** (M4 #3) — lawn background for menu
6. **Port upstream GameButton.cpp 1:1** — with NewLawnButton, DialogButton
7. **Port StoreScreen/AlmanacDialog/ZenGarden** — sub-screens for button clicks

---

## Previous status (2026-06-27)

App **boots, renders real game art with correct colours**, stable render loop
(RF1..RF6000+, `eglSwapBuffers` = EGL_SUCCESS), locked to **landscape 320x240**
(perfect 4:3). `IMAGE_TITLESCREEN` renders **full-canvas scaled to 400x300**
with correct colours (no purple tint, no R/B swap). The four fixes above
(commits pending) are **confirmed on-device**.

**LATEST M4 #1 WORK (2026-06-27, code-review only — not yet built on device).**
GameSelector + GameButton + ToolTipWidget ported; d-pad→mouse synthesis +
`HandlePointerEventL` added to `PvZAppUi`. The menu now has 10 clickable
buttons (Adventure/Survival/Minigames/Puzzles/Store/Almanac/ZenGarden/Options/
Help/Quit) laid out on a 4-row grid over the title background. Click routing
goes through `ButtonListener::ButtonDepressed` → `LawnApp` flow methods
(`PreNewGame` / `ShowChallengeScreen` / `ShowStoreScreen` / `DoNewOptions` /
`ConfirmQuit` / `DoAlmanacDialog`). Patches in
`/home/z/my-project/download/m4-gameselector-patches/`.

**What's new in M4 #1 (this session):**
- `WidgetManager` gained `mDownButtons` bitmask (matches upstream) so
  `GameButton::Update` can poll `mDownButtons & 5` for left|middle mouse.
- `WidgetManager::MouseDown/MouseUp/MouseMove` now assign `mLastMouseX/Y`.
- `GameButton` rewritten from 50-line stub to full `Widget` subclass (~165
  lines .h, ~415 lines .cpp) with image+label, hover/down states, click
  routing via `ButtonDepressed`. Stone-button 9-slice path with beige-rect
  fallback when `IMAGE_BUTTON_*` globals are NULL.
- `GameSelector` rewritten from 112-line M3 stub to ~410-line implementation
  with 10 `GameButton` children, layout grid, click dispatch, tooltip, ESC
  → `ConfirmQuit`, ENTER → `PreNewGame(GAMEMODE_ADVENTURE)`.
- `ToolTipWidget` rewritten from 13-line stub to ~45-line .h + ~60-line .cpp.
  Compat API (`SetWarningText/SetLabel/SetPosition/mCenter`) preserved so
  `board.cpp` keeps compiling. NEW: `SetText` + real `Draw` (semi-transparent
  black bg, thin border, text via `FONT_PICO129` if loaded).
- `PvZAppUi`: `HandlePointerEventL` added (maps 320x240 window → 400x300
  logical, forwards to `WidgetManager::MouseDown/Up/Move/Drag`). `HandleKeyEventL`
  extended: d-pad arrows move a virtual cursor 16px/press + synthesize
  `MouseMove`; centre key (EStdKeyDevice3) / Enter synthesizes
  `MouseDown+MouseUp` (a click). Escape still routes to `KeyDown`.
- `PvZ_N95.mmp`: `ToolTipWidget.cpp` + `Widget\GameButton.cpp` added to build.
- BUILD MARKER bumped to `v13-m4-gameselector`.

**What's NOT done (M5+ TODOs — see "M4 still open" below):**
- Fonts (`GetFontThrow` stubbed) — button labels/title won't render until wired.
- 51 NULL `IMAGE_/FONT_/SOUND_` symbols — `IMAGE_BUTTON_*` may still be NULL,
  buttons fall back to beige filled rects.
- `IMAGE_BACKGROUND1` tiling (M4 #3) — GameSelector still uses `IMAGE_TITLESCREEN`.
- 149 `REANIM_*` not in PAK (M4 #6).
- `StoreScreen.cpp` / `AlmanacDialog.cpp` / `ZenGarden.cpp` full impls — clicking
  those buttons calls the LawnApp flow methods, which currently no-op.
- Reanimation cloud/flower/leaf/hand decoration, ParticleSystem trophy sparkle.

**Working:**
- GCCE build pipeline (`group/build_gcce.cmd`) — compile, link, make SIS.
- C++ runtime (EH / `User::Leave` / `TRAP`) via `STATICLIBRARY libgcc.lib`.
- EGL/GLES 1.1 context + POT texture upload (via manual full-POT fallback,
  skipping malfunctioning `glTexSubImage2DOES` on MBX); landscape orientation.
- `.pak` VFS; ICL-based PNG **and** JPEG decode (115 images decode OK at boot).
- `WidgetManager` draws; heartbeat `CPeriodic` render loop stable (30 FPS).
- **Image draw path: RTTI-independent, texture-size-safe, R/B-correct, no tint.**
- **`BringToFront`** ensures the target widget is drawn on top (the previous
  `BringToBack` buried it under an overlay).
- **`ArgbToRgba`** in POT fallback fixes red/blue channel ordering.
- **[M4 #1]** 10-button main menu + d-pad/touch input routing (not yet
  device-verified — code review only).

**TODO / still open (M4, priority order):**
1. **Full `GameSelector` implementation** — Adventure/Survival/minigames/options
   menu. Requires porting `GameButton.cpp`, `StoreScreen.cpp`, `AlmanacDialog.cpp`,
   `ToolTipWidget.cpp`, `Music.cpp`, `ProfileMgr.cpp`, `ZenGarden.cpp`,
   `SaveGame.cpp` (only `.h` stubs exist currently). Upstream reference:
   `src/Lawn/Widget/GameSelector.cpp` from `wszqkzqk/PvZ-Portable`.
   **[MVP DONE 2026-06-27]** `GameButton.cpp` + `GameSelector.cpp` + `ToolTipWidget.cpp`
   ported (patches in `/home/z/my-project/download/m4-gameselector-patches/`).
   10 clickable buttons, d-pad/touch input, click routing to LawnApp flow
   methods. `Music.cpp`/`ProfileMgr.cpp`/`SaveGame.cpp` are already inline
   no-op stubs in their .h files (no .cpp needed). **Still TODO**:
   `StoreScreen.cpp`/`AlmanacDialog.cpp`/`ZenGarden.cpp` full impls (clicking
   those buttons currently no-ops); Reanimation decoration; ProfileMgr real
   persistence; SaveGame real serialisation.
2. **Key/touch input** — N95 needs d-pad + centre-key mapped to
   `WidgetManager::MouseDown/MouseUp` or `KeyDown`. Currently `PvZAppUi::HandleKeyEventL`
   only forwards four arrow keys + Enter + Escape via `WidgetManager::KeyDown`;
   the WidgetManager's `MouseDown/MouseUp/hit-test path is unreachable because
   `PvZGameView` doesn't call it. Add a `HandlePointerEventL` or extend the
   key handler to synthesise Mouse events.
   **[DONE 2026-06-27]** `HandlePointerEventL` added to `PvZAppUi` (forwards
   touch events with 320x240→400x300 coord mapping). `HandleKeyEventL`
   extended: d-pad arrows move a virtual cursor 16px/press and synthesise
   `MouseMove`; centre key (EStdKeyDevice3) / Enter synthesises
   `MouseDown+MouseUp` (a click). Escape still routes to `KeyDown`. See
   patches `08_pvzappui-h.patch` + `09_pvzappui-cpp.patch`.
3. **Restore the lawn (`IMAGE_BACKGROUND1`) properly.** Needs BOTH: (a) decode it
   without OOM, and (b) a texture ≤1024 — i.e. **texture TILING** (split into
   ≤1024 tiles, the way the original Sexy framework does) or a pre-downscale.
   The MBX 1024 limit is the hard constraint; do not try a single 2048 texture.
4. Fonts (`GetFontThrow` stubbed), strings, sounds not really loaded.
5. ~51 game-referenced `IMAGE_/FONT_/SOUND_` symbols are defined but **NULL**.
6. 149 `REANIM_*` images report `not in PAK` (asset naming / packing mismatch).

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