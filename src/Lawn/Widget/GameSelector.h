#ifndef __GAMESELECTOR_H__
#define __GAMESELECTOR_H__
#include "../../engine/Widget.h"

namespace Sexy { class LawnApp; }

namespace Sexy {

class GameSelector : public Widget
{
public:
    GameSelector() : Widget(), mApp(NULL) {}
    GameSelector(LawnApp* theApp) : Widget(), mApp(theApp) {}
    virtual ~GameSelector() {}
    virtual void Update() {}
    virtual void Draw(Graphics* g);
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    void SyncProfile(bool) {}
    LawnApp* mApp;
};

} // namespace Sexy

#endif
