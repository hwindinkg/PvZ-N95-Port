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
        // Fill background with dark purple
        g->SetColor(Color(38, 13, 51, 255));
        g->FillRect(0, 0, mWidth, mHeight);

        // Draw TitleScreen image if loaded
        if (IMAGE_TITLESCREEN)
        {
            g->DrawImage(IMAGE_TITLESCREEN, 0, 0);
        }
        else
        {
            // Draw a test pattern to verify rendering works
            // White rectangle in center
            g->SetColor(Color(255, 255, 255, 255));
            g->FillRect(50, 50, mWidth - 100, mHeight - 100);
            // Red/green/blue bars at bottom
            g->SetColor(Color(255, 0, 0, 255));
            g->FillRect(0, mHeight - 40, mWidth/3, 40);
            g->SetColor(Color(0, 255, 0, 255));
            g->FillRect(mWidth/3, mHeight - 40, mWidth/3, 40);
            g->SetColor(Color(0, 0, 255, 255));
            g->FillRect(2*mWidth/3, mHeight - 40, mWidth/3, 40);
        }
    }
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    LawnApp* mApp;
    bool mLoaderScreenIsLoaded;
    int mQuickLoadKey;
};

} // namespace Sexy

#endif
