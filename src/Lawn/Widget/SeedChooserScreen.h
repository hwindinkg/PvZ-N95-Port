#ifndef _SEEDCHOOSERSCREEN_H_
#define _SEEDCHOOSERSCREEN_H_

#include "../../engine/Widget.h"

namespace Sexy { class LawnApp; }

namespace Sexy {

class SeedChooserScreen : public Widget
{
public:
    SeedChooserScreen(LawnApp* theApp) : Widget(), mApp(theApp) {}
    SeedChooserScreen() : Widget(), mApp(NULL) {}

    virtual void Update() {}
    virtual void Draw(Graphics* g) { (void)g; }
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    void CancelLawnView() {}

    LawnApp* mApp;
    bool mMouseVisible;
};

} // namespace Sexy

#endif
