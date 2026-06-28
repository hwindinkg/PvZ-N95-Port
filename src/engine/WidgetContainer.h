#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include "Common.h"

namespace Sexy {

class Widget;
class Graphics;

class WidgetContainer
{
public:
    WidgetContainer();
    virtual ~WidgetContainer();

    virtual void AddWidget(Widget* theWidget);
    virtual void RemoveWidget(Widget* theWidget);
    virtual void DrawAll(Graphics* g);
    virtual void UpdateAll();
    virtual void UpdateAllF(float f);

    void BringToFront(Widget* theWidget);
    void BringToBack(Widget* theWidget);
    void PutBehind(Widget* theWidget, Widget* theRef);
    void PutInfront(Widget* theWidget, Widget* theRef);

    int GetWidgetCount() const { return mWidgetCount; }
    Widget* GetWidgetAt(int index) const
    {
        if (index >= 0 && index < mWidgetCount)
            return mWidgets[index];
        return NULL;
    }

    void MarkAllDirty();
    void DisableWidget(Widget* theWidget);
    void EnableWidget(Widget* theWidget);

    bool mDirty;
    bool mHasTransparencies;
    bool mDoFinger;

    static const int MAX_WIDGETS = 512;
    Widget* mWidgets[MAX_WIDGETS];
    int mWidgetCount;

    bool mMouseVisible;
    int mCursorX, mCursorY;
    bool mDefaultCursor;
};

} // namespace Sexy

#endif // WIDGETCONTAINER_H
