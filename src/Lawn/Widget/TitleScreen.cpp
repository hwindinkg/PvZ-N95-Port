/*
 * TitleScreen.cpp -- M4 loading screen with animated progress bar.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/TitleScreen.cpp (562 lines),
 * heavily simplified. Upstream has:
 *   - TITLESTATE_WAITING_FOR_FIRST_DRAW / POPCAP_LOGO / PARTNER_LOGO / SCREEN
 *     state machine with fade animations
 *   - SODROLLCAP reanimation that rolls across the grass as it loads
 *   - Reanimation clouds/flowers decoration
 *   - HyperlinkWidget "Click to Start" button
 *
 * This port has NONE of those subsystems (Reanimation, TodParticle, HyperlinkWidget
 * are M5+). This implementation:
 *   - Draws IMAGE_TITLESCREEN as background (full-canvas scaled to 400x300)
 *   - Draws IMAGE_PVZ_LOGO if loaded (centered top)
 *   - Draws IMAGE_LOADBAR_DIRT (the dirt strip under the grass bar)
 *   - Draws IMAGE_LOADBAR_GRASS clipped to mCurBarWidth (the grass that grows)
 *   - Falls back to a plain green filled-rect bar if LOADBAR images are NULL
 *   - Animates mCurBarWidth based on LawnApp::mCompletedLoadingThreadTasks /
 *     mNumLoadingThreadTasks (sync loading in this port, so the bar fills
 *     during LoadingThreadProc which runs before the heartbeat timer starts)
 *
 * The bar position matches upstream: y = 650 in original 800x600 coords;
 * this port's canvas is 400x300, so y = 650 * 300/600 = 325 -> clamped to 270.
 * Bar width 314 upstream -> 157 at 400x300 scale.
 */
#include "TitleScreen.h"

#include "../../LawnApp.h"
// NOTE: do NOT #include ../../Resources.h here. LawnApp.h includes engine/Stubs.h
// which #defines IMAGE_MONEYBAG, IMAGE_REANIM_CRAZYDAVE_MOUTH1, etc. as macros.
// Those macros would corrupt the extern declarations in Resources.h (turning
// `extern Image* IMAGE_MONEYBAG;` into `extern Image* ((Sexy::Image*)0);` which
// is a syntax error). TitleScreen.h already forward-declares the 4 IMAGE_*
// globals we need (IMAGE_TITLESCREEN, IMAGE_LOADBAR_DIRT, IMAGE_LOADBAR_GRASS,
// IMAGE_PVZ_LOGO), so we don't need Resources.h.
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"
#include "../../engine/Color.h"
#include "../../engine/Rect.h"
#include "../../engine/SexyAppBase.h"

namespace Sexy {

TitleScreen::TitleScreen(LawnApp* theApp)
    : Widget()
    , mApp(theApp)
    , mLoaderScreenIsLoaded(false)
    , mQuickLoadKey(0)
    , mCurBarWidth(0.0f)
    , mTotalBarWidth(157.0f)   // 314 upstream / 2 (400x300 vs 800x600)
    , mBarVel(0.0f)
    , mLoadingThreadComplete(false)
    , mDrawnYet(false)
    , mPrevLoadingPercent(0.0f)
{
    mVisible = true;
    mMouseVisible = true;
    mHasTransparencies = true;
    mWantsFocus = true;
}

void TitleScreen::Update()
{
    Widget::Update();

    if (!mApp) return;

    // Compute loading progress from LawnApp's counters.
    // In this port LoadingThreadProc runs synchronously BEFORE the heartbeat
    // timer starts, so by the time Update() fires the loading is ALREADY DONE.
    // The bar will jump from 0 to full in one frame. To make it visible, we
    // animate mCurBarWidth toward the target at a fixed rate.
    float target = 0.0f;
    if (mApp->mNumLoadingThreadTasks > 0)
    {
        target = mTotalBarWidth * (float)mApp->mCompletedLoadingThreadTasks
                 / (float)mApp->mNumLoadingThreadTasks;
    }
    if (mApp->mLoadingThreadCompleted)
    {
        target = mTotalBarWidth;
        mLoadingThreadComplete = true;
    }

    // Animate toward target (ease-in: faster as more loads).
    if (mCurBarWidth < target)
    {
        mCurBarWidth += 4.0f;   // 4px per Update tick (~30fps -> ~120px/s)
        if (mCurBarWidth > target) mCurBarWidth = target;
    }

    mPrevLoadingPercent = mCurBarWidth / mTotalBarWidth;
}

void TitleScreen::Draw(Graphics* g)
{
    if (!g) return;
    if (!mApp) return;

    // White vertex colour so GL_MODULATE doesn't tint the texture (M3 fix #7).
    g->SetColor(Color(255, 255, 255, 255));

    // -- Background: IMAGE_TITLESCREEN scaled to full canvas ----------------
    Image* bg = IMAGE_TITLESCREEN;
    if (bg == NULL && mApp->mResourceManager)
        bg = mApp->mResourceManager->GetImage("IMAGE_TITLESCREEN");

    if (bg && bg->GetWidth() > 0 && bg->GetHeight() > 0)
    {
        MemoryImage* mem = static_cast<MemoryImage*>(bg);
        g->DrawImage(mem, 0, 0, mWidth, mHeight);
    }
    else
    {
        // Fallback: dark background.
        g->SetColor(Color(20, 60, 30, 255));
        g->FillRect(0, 0, mWidth, mHeight);
    }

    // -- PVZ logo (centered top) -------------------------------------------
    Image* logo = IMAGE_PVZ_LOGO;
    if (logo && logo->GetWidth() > 0)
    {
        MemoryImage* logoMem = static_cast<MemoryImage*>(logo);
        // Scale logo to fit (upstream 800x600 -> 400x300, logo ~440x170 -> 220x85)
        int logoW = logo->GetWidth() / 2;
        int logoH = logo->GetHeight() / 2;
        int logoX = (mWidth - logoW) / 2;
        int logoY = 20;
        g->DrawImage(logoMem, logoX, logoY, logoW, logoH);
    }

    // -- Loading bar -------------------------------------------------------
    // Upstream: bar at y=650 in 800x600 -> y=325 in 400x300, but our canvas
    // is only 300 tall. Clamp to y=270 (near bottom). Bar width 314 -> 157.
    int barX = (mWidth - (int)mTotalBarWidth) / 2;
    int barY = mHeight - 40;   // 270 in 300-tall canvas
    int barW = (int)mTotalBarWidth;
    int barH = 24;

    Image* dirtImg = IMAGE_LOADBAR_DIRT;
    Image* grassImg = IMAGE_LOADBAR_GRASS;

    if (dirtImg && grassImg && dirtImg->GetWidth() > 0 && grassImg->GetWidth() > 0)
    {
        // Real loading bar: dirt strip + clipped grass.
        MemoryImage* dirtMem = static_cast<MemoryImage*>(dirtImg);
        MemoryImage* grassMem = static_cast<MemoryImage*>(grassImg);

        // Scale dirt to bar size.
        g->DrawImage(dirtMem, barX, barY, barW, barH);

        // Grass: clip to mCurBarWidth. Port's Graphics::ClipRect exists but
        // DrawImage with srcRect is a stub. Workaround: draw grass scaled to
        // (mCurBarWidth, barH) -- visually approximates the clipping.
        int curW = (int)mCurBarWidth;
        if (curW > 0)
        {
            g->DrawImage(grassMem, barX, barY, curW, barH);
        }
    }
    else
    {
        // Fallback: plain rect bar.
        // Background (empty bar).
        g->SetColor(Color(60, 40, 20, 255));     // dark brown
        g->FillRect(barX, barY, barW, barH);
        g->SetColor(Color(120, 80, 40, 255));    // border
        g->DrawRect(barX, barY, barW, barH);

        // Filled portion (grass green).
        int curW = (int)mCurBarWidth;
        if (curW > 0)
        {
            g->SetColor(Color(80, 160, 60, 255));
            g->FillRect(barX + 2, barY + 2, curW - 4, barH - 4);
        }
    }

    // -- "Loading..." text (best-effort; FONT_DWARVEN may be NULL) ----------
    // (Fonts are stubbed in this port -- M4 #4. Text won't render until
    //  GetFontThrow is properly implemented.)
}

} // namespace Sexy
