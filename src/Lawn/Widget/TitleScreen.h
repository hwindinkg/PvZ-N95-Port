#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__
#include "../../engine/Widget.h"
#include "../../engine/Image.h"   // for Sexy::Image (defined class)

namespace Sexy { class LawnApp; }

namespace Sexy {

// Forward-declare the IMAGE_* globals we use. They are defined in Resources.cpp
// and declared in Resources.h, but we can't #include Resources.h here because
// Stubs.h (included before us in some .cpp files) #defines IMAGE_MONEYBAG,
// IMAGE_REANIM_CRAZYDAVE_MOUTH1, etc. as macros, which would corrupt the
// extern declarations in Resources.h. Declaring just the ones we need avoids
// that conflict.
extern Image* IMAGE_TITLESCREEN;
extern Image* IMAGE_LOADBAR_DIRT;
extern Image* IMAGE_LOADBAR_GRASS;
extern Image* IMAGE_PVZ_LOGO;
extern Image* IMAGE_POPCAP_LOGO;

class TitleScreen : public Widget
{
public:
    TitleScreen() : Widget() {}
    TitleScreen(LawnApp* theApp);
    virtual ~TitleScreen() {}
    virtual void Update();
    virtual void Draw(Graphics* g);
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    LawnApp* mApp;
    bool mLoaderScreenIsLoaded;
    int mQuickLoadKey;

    // Loading bar state (ported from upstream TitleScreen.h).
    float mCurBarWidth;       // current grass width in px (0..mTotalBarWidth)
    float mTotalBarWidth;     // full bar width = 314 in upstream
    float mBarVel;            // px per Update tick
    bool mLoadingThreadComplete;
    bool mDrawnYet;
    float mPrevLoadingPercent;

    // [Session-9] PopCap logo intro state (like the original PvZ).
    // Phase 0: PopCap logo fading in (0-30 frames)
    // Phase 1: PopCap logo held (30-60 frames)
    // Phase 2: PopCap logo fading out (60-90 frames)
    // Phase 3: Loading screen (progress bar + Click to Start)
    int mLogoPhase;
    int mLogoFrame;     // frame counter within current phase
    int mLogoAlpha;     // 0-255, computed from phase
};

} // namespace Sexy

#endif
