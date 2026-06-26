#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__
#include "../../engine/Widget.h"

namespace Sexy { class LawnApp; }
namespace Sexy { extern Image* IMAGE_TITLESCREEN; }

namespace Sexy {

class TitleScreen : public Widget
{
public:
    TitleScreen() : Widget() {}
    TitleScreen(LawnApp* theApp) : Widget(), mApp(theApp) {}
    virtual ~TitleScreen() {}
    virtual void Update() {}
    virtual void Draw(Graphics* g) {
        if (!g) return;
        // [M3 fix] This widget is the legitimate TITLE screen shown DURING loading,
        // while IMAGE_TITLESCREEN is valid. After LoadingCompleted() the image is
        // freed and nulled and the widget is supposed to be removed -- but if a
        // lingering TitleScreen is still in the WidgetManager it MUST NOT paint,
        // otherwise its full-screen purple fill + RGB test pattern covers the real
        // game frame (the lawn drawn by GameSelector behind it). So: only paint when
        // we actually have the title image; once it's gone, draw nothing (fully
        // transparent) and let whatever is behind us show through.
        if (!IMAGE_TITLESCREEN)
            return;

        // Fill background with dark purple, then the title art.
        g->SetColor(Color(38, 13, 51, 255));
        g->FillRect(0, 0, mWidth, mHeight);
        g->DrawImage(IMAGE_TITLESCREEN, 0, 0);
    }
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    LawnApp* mApp;
    bool mLoaderScreenIsLoaded;
    int mQuickLoadKey;
};

} // namespace Sexy

#endif