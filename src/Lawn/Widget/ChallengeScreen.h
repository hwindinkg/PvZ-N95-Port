#ifndef _CHALLENGESCREEN_H_
#define _CHALLENGESCREEN_H_

#include "../../engine/Widget.h"

namespace Sexy {

class ChallengeScreen : public Widget
{
public:
    ChallengeScreen(LawnApp* theApp, ChallengePage thePage)
        : Widget(), mApp(theApp), mPage(thePage) {}

    virtual void Update() {}
    virtual void Draw(Graphics* g) { (void)g; }
    virtual void Resize(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; }

    LawnApp*      mApp;
    ChallengePage mPage;
};

} // namespace Sexy

#endif
