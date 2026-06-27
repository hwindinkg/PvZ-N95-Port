#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__
#include "../../engine/Widget.h"
#include "../../Resources.h"   // for IMAGE_TITLESCREEN / IMAGE_LOADBAR_* / IMAGE_PVZ_LOGO (all in namespace Sexy)

namespace Sexy { class LawnApp; }

namespace Sexy {

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
};

} // namespace Sexy

#endif
