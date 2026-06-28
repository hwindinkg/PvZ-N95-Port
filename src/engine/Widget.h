#ifndef WIDGET_H
#define WIDGET_H

#include "WidgetContainer.h"
#include "Insets.h"
#include "KeyCodes.h"
#include "Color.h"

namespace Sexy {

class WidgetManager;

class Widget : public WidgetContainer
{
public:
    Widget();
    virtual ~Widget();

    bool mVisible;
    bool mMouseVisible;
    bool mDisabled;
    bool mHasFocus;
    bool mIsDown;
    bool mIsOver;
    bool mHasTransparencies;
    bool mDoFinger;
    bool mWantsFocus;
    bool mClip;

    int mX, mY, mWidth, mHeight;
    int mPriority;

    Color mColors[8];
    Insets mMouseInsets;

    Widget* mParent;
    WidgetManager* mWidgetManager;
    Widget* mTabPrev;
    Widget* mTabNext;

    // Events
    virtual void Draw(Graphics* g);
    virtual void Update();
    virtual void UpdateF(float f);
    virtual void GotFocus();
    virtual void LostFocus();

    virtual void MouseDown(int x, int y, int btn);
    virtual void MouseUp(int x, int y, int btn);
    virtual void MouseMove(int x, int y);
    virtual void MouseDrag(int x, int y);
    virtual void MouseWheel(int delta);
    virtual void MouseEnter();
    virtual void MouseLeave();

    virtual void KeyDown(KeyCode key);
    virtual bool KeyUp(KeyCode key);
    virtual void KeyChar(char c);

    // Layout
    void Resize(int x, int y, int w, int h);
    void Move(int x, int y);
    bool Contains(int x, int y) const;
    void MarkDirty();

    int GetAbsX() const;
    int GetAbsY() const;

    // Resource unloading
    char* mLoadedResourceNames[64];
    int mLoadedResourceCount;

    void DirtyAll() { MarkDirty(); MarkAllDirty(); }
    void MarkDirtyFull() { MarkDirty(); MarkAllDirty(); }
};

} // namespace Sexy

#endif // WIDGET_H
