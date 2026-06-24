#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

#include "WidgetContainer.h"
#include "Widget.h"

namespace Sexy {

class Graphics;

class WidgetManager : public WidgetContainer
{
public:
    WidgetManager();
    virtual ~WidgetManager();

    Widget* mDefaultTab;
    Widget* mFocusWidget;
    Widget* mOverWidget;
    Widget* mLastDownWidget;
    Widget* mKeyDownTarget;

    int mMouseX, mMouseY;
    int mLastMouseX, mLastMouseY;
    bool mWidgetsEnabled;
    bool mKeyDown[256];

    void FocusNextWidget(bool next);
    void SetFocus(Widget* theWidget);

    void DrawScreen(Graphics* g);
    void UpdateFrame();
    void UpdateFrameF(float f);

    void MouseDown(int x, int y, int btn);
    void MouseUp(int x, int y, int btn);
    void MouseMove(int x, int y);
    void MouseDrag(int x, int y);
    void MouseWheel(int delta);
    bool KeyDown(KeyCode key);
    bool KeyUp(KeyCode key);
    void KeyChar(char c);

    void RemapMouse(int& x, int& y);

    Widget* FindWidget(int x, int y, bool checkTransparencies = false);

    struct PreModalInfo {
        Widget* mBaseModalWidget;
        Widget* mPrevBaseModalWidget;
        Widget* mPrevFocusWidget;
    };

    static const int MAX_MODAL = 16;
    Widget* mBaseModalWidget;
    PreModalInfo mPreModalInfoList[MAX_MODAL];
    int mPreModalCount;

    // Template RemoveWidget for any Widget subclass pointer (stub)
    template<typename T> void RemoveWidget(T*& w) { w = NULL; }
};

} // namespace Sexy

#endif // WIDGETMANAGER_H
