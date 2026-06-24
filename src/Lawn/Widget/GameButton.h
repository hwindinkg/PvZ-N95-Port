#ifndef GAMEBUTTON_H
#define GAMEBUTTON_H

#include <e32base.h>
#include "../../engine/Widget.h"
#include "../../engine/ButtonListener.h"


class GameButton : public Sexy::Widget
{
public:
    GameButton() {}
    GameButton(int theId, Sexy::ButtonListener* theListener = NULL)
        : Sexy::Widget()
        , mId(theId)
        , mListener(theListener)
        , mBtnNoDraw(false)
        , mDrawStoneButton(false)
        , mButtonImage(NULL)
        , mOverImage(NULL)
        , mDownImage(NULL)
        , mParentWidget(NULL)
    {}

    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    void Resize(int x, int y, int w, int h) { Sexy::Widget::Resize(x, y, w, h); }
    void SetLabel(const char* label) { (void)label; }
    void SetVisible(bool visible) { Sexy::Widget::mVisible = visible; }
    bool MouseHitTest(int x, int y, int theClickCount) { (void)x; (void)y; (void)theClickCount; return false; }
    bool MouseHitTest(int& x, int& y) { (void)x; (void)y; return false; }

    int mId;
    Sexy::ButtonListener* mListener;
    bool mBtnNoDraw;
    bool mDrawStoneButton;
    class Sexy::Image* mButtonImage;
    class Sexy::Image* mOverImage;
    class Sexy::Image* mDownImage;
    Sexy::Widget* mParentWidget;
};

#endif // GAMEBUTTON_H
