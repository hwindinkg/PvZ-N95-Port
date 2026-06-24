#ifndef __AWARDSCREEN_H__
#define __AWARDSCREEN_H__
#include "../../engine/Widget.h"

namespace Sexy { class LawnApp; }

namespace Sexy {

class AwardScreen : public Widget
{
public:
    AwardScreen() : Widget() {}
    AwardScreen(LawnApp* theApp, int theAwardType, bool theShowAchievements) : Widget(), mApp(theApp) { (void)theAwardType; (void)theShowAchievements; }
    virtual ~AwardScreen() {}
    virtual void Update() {}
    virtual void Draw(Graphics* g) { (void)g; }
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    LawnApp* mApp;
};

} // namespace Sexy

#endif
