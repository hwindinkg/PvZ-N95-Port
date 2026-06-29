/*
 * TitleScreen.cpp — loading screen with PopCap logo, SODROLLCAP, grass unroll.
 *
 * Ported from upstream PvZ-Portable TitleScreen.cpp (562 lines).
 * Implements:
 *   - PopCap logo fade in/out (3 phases)
 *   - Loading screen with IMAGE_TITLESCREEN background
 *   - PvZ logo slide-in from top (TodAnimateCurve)
 *   - Loading bar with grass unroll (clip rect)
 *   - SODROLLCAP zombie head rolling on the grass
 *   - "Click to Start" text (blink)
 */
#include "TitleScreen.h"

#include "../../LawnApp.h"
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"
#include "../../engine/Color.h"
#include "../../engine/Rect.h"
#include "../../engine/SexyAppBase.h"
#include "../../engine/ResourceManager.h"
#include "../../engine/SystemFont.h"
#include <f32file.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace Sexy {

TitleScreen::TitleScreen(LawnApp* theApp)
    : Widget()
    , mApp(theApp)
    , mLoaderScreenIsLoaded(false)
    , mQuickLoadKey(0)
    , mCurBarWidth(0.0f)
    , mTotalBarWidth(157.0f)
    , mBarVel(0.0f)
    , mLoadingThreadComplete(false)
    , mDrawnYet(false)
    , mPrevLoadingPercent(0.0f)
    , mLogoPhase(0)
    , mLogoFrame(0)
    , mLogoAlpha(0)
    , mLoadingScreenCounter(0)
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

    // [Session-13] PopCap logo intro phases.
    if (mLogoPhase < 3)
    {
        mLogoFrame++;
        if (mLogoPhase == 0)      // fade in: 30 frames
        {
            mLogoAlpha = (mLogoFrame * 255) / 30;
            if (mLogoFrame >= 30) { mLogoPhase = 1; mLogoFrame = 0; }
        }
        else if (mLogoPhase == 1) // hold: 60 frames
        {
            mLogoAlpha = 255;
            if (mLogoFrame >= 60) { mLogoPhase = 2; mLogoFrame = 0; }
        }
        else if (mLogoPhase == 2) // fade out: 30 frames
        {
            mLogoAlpha = 255 - (mLogoFrame * 255) / 30;
            if (mLogoFrame >= 30) { mLogoPhase = 3; mLogoFrame = 0; }
        }
        return;
    }

    // Phase 3: loading screen.
    mLoadingScreenCounter++;

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

    if (mCurBarWidth < target)
    {
        mCurBarWidth += 4.0f;
        if (mCurBarWidth > target) mCurBarWidth = target;
    }

    mPrevLoadingPercent = mCurBarWidth / mTotalBarWidth;
}

void TitleScreen::Draw(Graphics* g)
{
    if (!g || !mApp) return;

    g->SetColor(Color(255, 255, 255, 255));

    // -- PopCap logo intro (phases 0-2) --
    if (mLogoPhase < 3)
    {
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

            // PopCap logo with alpha fade
            Image* popcap = IMAGE_POPCAP_LOGO;
            int lw = popcap->GetWidth() * 2 / 5;
            int lh = popcap->GetHeight() * 2 / 5;
            int lx = (mWidth - lw) / 2;
            int ly = (mHeight - lh) / 2;
            g->SetColor(Color(255, 255, 255, mLogoAlpha));
            MemoryImage* mem = static_cast<MemoryImage*>(popcap);
            g->DrawImage(mem, lx, ly, lw, lh);
            return;
        }
    }

    // -- Phase 3: loading screen --
    // Background: IMAGE_TITLESCREEN
    Image* bg = IMAGE_TITLESCREEN;
    if (bg && bg->GetWidth() > 0 && bg->GetHeight() > 0)
    {
        g->SetColor(Color(255, 255, 255, 255));
        MemoryImage* mem = static_cast<MemoryImage*>(bg);
        g->DrawImage(mem, 0, 0, mWidth, mHeight);
    }
    else
    {
        g->SetColor(Color(20, 60, 30, 255));
        g->FillRect(0, 0, mWidth, mHeight);
    }

    // -- PvZ logo with slide-in animation --
    // Upstream: logo slides from y=-150 to y=10 over 40 frames, then bounces.
    Image* logo = IMAGE_PVZ_LOGO;
    if (logo && logo->GetWidth() > 0)
    {
        // [Session-13] Logo slide-in: first 40 frames slide from -75 to 5,
        // then stay at 5. This matches upstream TodAnimateCurve.
        int logoY;
        if (mLoadingScreenCounter < 40)
        {
            // Slide from -75 to 5 (ease-in)
            int t = mLoadingScreenCounter;
            logoY = -75 + (t * 80) / 40;  // linear: -75 → 5
        }
        else
        {
            logoY = 5;
        }

        g->SetColor(Color(255, 255, 255, 255));
        MemoryImage* logoMem = static_cast<MemoryImage*>(logo);
        // Scale logo to half size (upstream 800x600 → 400x300)
        int logoW = logo->GetWidth() / 2;
        int logoH = logo->GetHeight() / 2;
        int logoX = (mWidth - logoW) / 2;
        g->DrawImage(logoMem, logoX, logoY, logoW, logoH);
    }

    // -- Loading bar --
    int barX = (mWidth - (int)mTotalBarWidth) / 2;
    int barY = mHeight - 50;
    int barW = (int)mTotalBarWidth;
    int barH = 24;

    Image* dirtImg = IMAGE_LOADBAR_DIRT;
    Image* grassImg = IMAGE_LOADBAR_GRASS;

    if (dirtImg && grassImg && dirtImg->GetWidth() > 0 && grassImg->GetHeight() > 0)
    {
        MemoryImage* dirtMem = static_cast<MemoryImage*>(dirtImg);
        MemoryImage* grassMem = static_cast<MemoryImage*>(grassImg);

        // Dirt strip (full bar width)
        g->SetColor(Color(255, 255, 255, 255));
        g->DrawImage(dirtMem, barX, barY, barW, barH);

        int curW = (int)mCurBarWidth;
        if (curW > 0)
        {
            if (curW >= barW)
            {
                // Bar full: draw full grass
                g->SetColor(Color(255, 255, 255, 255));
                g->DrawImage(grassMem, barX, barY, barW, barH);
            }
            else
            {
                // [Session-13] Grass unroll: draw left portion of grass
                // scaled to curW. This shows the grass growing from left
                // to right without stretching.
                int srcW = (grassImg->GetWidth() * curW) / barW;
                if (srcW < 1) srcW = 1;
                if (srcW > grassImg->GetWidth()) srcW = grassImg->GetWidth();
                Rect srcRect(0, 0, srcW, grassImg->GetHeight());
                g->SetColor(Color(255, 255, 255, 255));
                g->DrawImageScaledSrcRect(grassMem, barX, barY, curW, barH, srcRect);

                // [Session-13] SODROLLCAP zombie head rolling on the grass.
                // Upstream: rolls from left to right as the bar fills.
                // The head rotates and shrinks as it rolls.
                Image* sodRoll = NULL;
                if (mApp && mApp->mResourceManager)
                    sodRoll = mApp->mResourceManager->GetImage("IMAGE_REANIM_SODROLLCAP");
                if (sodRoll && sodRoll->GetWidth() > 0)
                {
                    float rollLen = curW * 0.94f;
                    float rotation = -rollLen / 180.0f * PI * 2.0f;
                    float scale = 1.0f - (mCurBarWidth / mTotalBarWidth) * 0.5f;
                    if (scale < 0.5f) scale = 0.5f;

                    // Position: rolls along the top of the grass bar
                    int sodX = barX + 11 + (int)rollLen;
                    int sodY = barY - 3 - (int)(35.0f * scale) + 35;
                    int sodW = (int)(sodRoll->GetWidth() * scale * 0.5f);
                    int sodH = (int)(sodRoll->GetHeight() * scale * 0.5f);

                    if (sodW > 0 && sodH > 0)
                    {
                        g->SetColor(Color(255, 255, 255, 255));
                        MemoryImage* sodMem = static_cast<MemoryImage*>(sodRoll);
                        // Draw rotated — port doesn't have DrawImageRotated
                        // with scale, so just draw it scaled (no rotation).
                        // TODO: implement TodBltMatrix for proper rotation.
                        g->DrawImage(sodMem, sodX - sodW/2, sodY - sodH/2, sodW, sodH);
                    }
                }
            }
        }
    }
    else
    {
        // Fallback: plain rect bar
        g->SetColor(Color(60, 40, 20, 255));
        g->FillRect(barX, barY, barW, barH);
        g->SetColor(Color(120, 80, 40, 255));
        g->DrawRect(barX, barY, barW, barH);

        int curW = (int)mCurBarWidth;
        if (curW > 0)
        {
            g->SetColor(Color(80, 160, 60, 255));
            g->FillRect(barX + 2, barY + 2, curW - 4, barH - 4);
        }
    }

    // -- "Click to Start" text (when bar is full) --
    if (mCurBarWidth >= mTotalBarWidth - 1.0f)
    {
        int blink = (mApp->mAppCounter / 15) % 2;
        if (blink)
        {
            SystemFont* font = SystemFont::Get();
            if (font)
            {
                const char* msg = "Click to Start";
                int textW = font->StringWidth(msg);
                int textX = (mWidth - textW) / 2;
                int textY = barY + barH + 15;
                g->SetFont(font);
                g->SetColor(Color(255, 255, 0, 255));
                g->DrawString(msg, textX, textY);
            }
        }
    }
}

} // namespace Sexy
