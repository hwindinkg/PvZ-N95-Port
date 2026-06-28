#ifndef __STORESCREEN_H__
#define __STORESCREEN_H__
#include "../../engine/Widget.h"
#include <string>

namespace Sexy { class LawnApp; }

class StoreScreen : public Sexy::Widget
{
public:
    StoreScreen() : Sexy::Widget() {}
    StoreScreen(LawnApp* theApp) : Sexy::Widget(), mApp(theApp) {}
    virtual ~StoreScreen() {}
    virtual void Update() {}
    virtual void Draw(Sexy::Graphics* g) { (void)g; }
    static int GetItemCost(int item) { (void)item; return 0; }
    LawnApp* mApp;
};
#endif
