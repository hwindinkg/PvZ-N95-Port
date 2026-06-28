#ifndef __CREDITSCREEN_H__
#define __CREDITSCREEN_H__
#include "../../engine/Widget.h"

namespace Sexy { class LawnApp; }

namespace Sexy {

class CreditScreen : public Widget
{
public:
    CreditScreen() : Widget() {}
    CreditScreen(LawnApp* theApp) : Widget(), mApp(theApp) {}
    virtual ~CreditScreen() {}
    virtual void Update() {}
    virtual void Draw(Graphics* g) { (void)g; }
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    LawnApp* mApp;
};

} // namespace Sexy

#endif
