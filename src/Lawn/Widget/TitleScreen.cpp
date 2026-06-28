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
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"
#include "../../engine/Color.h"
#include "../../engine/Rect.h"
#include "../../engine/SexyAppBase.h"
#include "../../engine/ResourceManager.h"   // for ResourceManager (LawnApp.h only forward-declares it)
#include "../../engine/SystemFont.h"        // for "Click to Start" text

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
    , mLogoPhase(0)
    , mLogoFrame(0)
    , mLogoAlpha(0)
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

    // [Session-9] PopCap logo intro phases.
    // Phase 0: fade in (30 frames, alpha 0->255)
    // Phase 1: hold (30 frames, alpha 255)
    // Phase 2: fade out (30 frames, alpha 255->0)
    // Phase 3: loading screen (progress bar)
    if (mLogoPhase < 3)
    {
        mLogoFrame++;
        if (mLogoPhase == 0)
        {
            mLogoAlpha = (mLogoFrame * 255) / 30;
            if (mLogoFrame >= 30) { mLogoPhase = 1; mLogoFrame = 0; }
        }
        else if (mLogoPhase == 1)
        {
            mLogoAlpha = 255;
            if (mLogoFrame >= 30) { mLogoPhase = 2; mLogoFrame = 0; }
        }
        else if (mLogoPhase == 2)
        {
            mLogoAlpha = 255 - (mLogoFrame * 255) / 30;
            if (mLogoFrame >= 30) { mLogoPhase = 3; mLogoFrame = 0; }
        }
        return; // don't advance loading bar during logo intro
    }

    // Phase 3: loading screen with progress bar.
    // Compute loading progress from LawnApp's counters.
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

    // Animate toward target (4px per Update tick, ~120px/s at 30fps).
    if (mCurBarWidth < target)
    {
        mCurBarWidth += 4.0f;
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

    // [Session-9] PopCap logo intro (phases 0-2).
    // Black background + PopCap logo centered, fading in/out.
    // [Session-10] DON'T call GetImage in the render loop — it triggers ICL
    // decode which hangs/crashes on the N95 (rmgr_log showed 2 incomplete
    // popcap_logo decodes, no "convert done"). Use the IMAGE_POPCAP_LOGO
    // global directly. If it's NULL (not loaded by LoadingThreadProc), skip
    // the logo intro entirely (jump to phase 3 = loading screen).
    if (mLogoPhase < 3)
    {
        // If PopCap logo isn't loaded, skip the intro immediately.
        if (!IMAGE_POPCAP_LOGO)
        {
            mLogoPhase = 3;
            mLogoFrame = 0;
        }
        else
        {
            // Black background
            g->SetColor(Color(0, 0, 0, 255));
            g->FillRect(0, 0, mWidth, mHeight);

            // PopCap logo centered (use global directly, no GetImage call)
            Image* popcap = IMAGE_POPCAP_LOGO;
            if (popcap && popcap->GetWidth() > 0)
            {
                int lw = popcap->GetWidth() / 3;
                int lh = popcap->GetHeight() / 3;
                int lx = (mWidth - lw) / 2;
                int ly = (mHeight - lh) / 2;
                MemoryImage* mem = static_cast<MemoryImage*>(popcap);
                g->DrawImage(mem, lx, ly, lw, lh);
            }
            return;
        }
    }

    // If we jumped here from the logo-skip above, fall through to loading screen.
    // Phase 3: loading screen (background + logo + progress bar + click text)
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

    if (dirtImg && grassImg && dirtImg->GetWidth() > 0 && grassImg->GetHeight() > 0)
    {
        // Real loading bar: dirt strip + grass overlay.
        MemoryImage* dirtMem = static_cast<MemoryImage*>(dirtImg);
        MemoryImage* grassMem = static_cast<MemoryImage*>(grassImg);

        // Scale dirt to bar size (full bar width).
        g->DrawImage(dirtMem, barX, barY, barW, barH);

        // Grass: [Session-12] Simple stretch approach. The grass image is
        // drawn scaled to (curW, barH). This stretches horizontally but is
        // the most reliable — glScissor doesn't work on MBX, and the overlay
        // approach looked like a filling bar. The stretch is visually close
        // to the original (the grass texture tiles horizontally anyway).
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

    // -- "Click to Start" text (when bar is full) -------------------------
    // Upstream shows a HyperlinkWidget "Click to Start" after the bar fills.
    // We use SystemFont (8x8 bitmap fallback) since PvZ font assets are not
    // in the PAK. Blink every ~1s to draw attention.
    if (mCurBarWidth >= mTotalBarWidth - 1.0f)
    {
        // Blink using mAppCounter (increments each Update).
        int blink = (mApp->mAppCounter / 15) % 2;  // ~0.5s on, 0.5s off at 30fps
        if (blink)
        {
            SystemFont* font = SystemFont::Get();
            if (font)
            {
                const char* msg = "Click to Start";
                int textW = font->StringWidth(msg);
                int textX = (mWidth - textW) / 2;
                int textY = barY + barH + 20;
                // Yellow text.
                g->SetFont(font);
                g->SetColor(Color(255, 255, 0, 255));
                g->DrawString(msg, textX, textY);
            }
        }
    }
}

} // namespace Sexy
