#include "WidgetContainer.h"
#include "Widget.h"
#include "WidgetManager.h"
#include "Graphics.h"
#include <e32debug.h>
#include <f32file.h>

namespace Sexy {

WidgetContainer::WidgetContainer()
    : mDirty(true)
    , mHasTransparencies(false)
    , mDoFinger(false)
    , mWidgetCount(0)
    , mMouseVisible(true)
    , mCursorX(0), mCursorY(0)
    , mDefaultCursor(true)
{
}

WidgetContainer::~WidgetContainer()
{
}

void WidgetContainer::AddWidget(Widget* theWidget)
{
    if (mWidgetCount >= MAX_WIDGETS)
        return;

    theWidget->mParent = (Widget*)this;
    theWidget->mWidgetManager = (WidgetManager*)this;
    theWidget->mDoFinger = mDoFinger;
    mWidgets[mWidgetCount] = theWidget;
    mWidgetCount++;
}

void WidgetContainer::RemoveWidget(Widget* theWidget)
{
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == theWidget)
        {
            theWidget->mParent = NULL;
            theWidget->mWidgetManager = NULL;
            for (int j = i; j < mWidgetCount - 1; j++)
                mWidgets[j] = mWidgets[j + 1];
            mWidgetCount--;
            break;
        }
    }
}

void WidgetContainer::DrawAll(Graphics* g)
{
    // Log first call with widget count
    static TInt logOnce = 1;
    if (logOnce) {
        logOnce = 0;
        RFs fs; RFile f;
        if (fs.Connect() == KErrNone) {
            fs.MkDirAll(_L("C:\\Data\\PvZ"));
            if (f.Replace(fs, _L("C:\\Data\\PvZ\\wgt_log.txt"), EFileWrite) == KErrNone) {
                TBuf8<64> msg;
                if (g && g->GetGLInterface())
                    msg.Copy(_L8("DrawAll: mGL set\n"));
                else
                    msg.Copy(_L8("DrawAll: mGL NULL\n"));
                f.Write(msg);
                msg.Num((TInt)mWidgetCount);
                msg.Insert(0, _L8("Widgets="));
                msg.Append('\n');
                f.Write(msg);
                f.Close();
            }
            fs.Close();
        }
    }
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        Widget* w = mWidgets[i];
        if (w->mVisible)
        {
            w->Draw(g);
        }
    }
    mDirty = false;
}

void WidgetContainer::UpdateAll()
{
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        mWidgets[i]->Update();
    }
}

void WidgetContainer::UpdateAllF(float f)
{
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        mWidgets[i]->UpdateF(f);
    }
}

void WidgetContainer::BringToFront(Widget* theWidget)
{
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == theWidget)
        {
            for (int j = i; j < mWidgetCount - 1; j++)
                mWidgets[j] = mWidgets[j + 1];
            mWidgets[mWidgetCount - 1] = theWidget;
            break;
        }
        else if (i == mWidgetCount - 1)
            return; // not found
    }
    MarkAllDirty();
}

void WidgetContainer::BringToBack(Widget* theWidget)
{
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == theWidget)
        {
            for (int j = i; j > 0; j--)
                mWidgets[j] = mWidgets[j - 1];
            mWidgets[0] = theWidget;
            break;
        }
    }
    MarkAllDirty();
}

void WidgetContainer::PutBehind(Widget* theWidget, Widget* theRef)
{
    if (theWidget == theRef)
        return;
    int wi = -1, ri = -1;
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == theWidget) wi = i;
        if (mWidgets[i] == theRef) ri = i;
    }
    if (wi < 0 || ri < 0 || wi <= ri)
        return;

    for (int j = wi; j > ri; j--)
        mWidgets[j] = mWidgets[j - 1];
    mWidgets[ri] = theWidget;
    MarkAllDirty();
}

void WidgetContainer::PutInfront(Widget* theWidget, Widget* theRef)
{
    if (theWidget == theRef)
        return;
    int wi = -1, ri = -1;
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == theWidget) wi = i;
        if (mWidgets[i] == theRef) ri = i;
    }
    if (wi < 0 || ri < 0 || wi >= ri)
        return;

    for (int j = wi; j < ri; j++)
        mWidgets[j] = mWidgets[j + 1];
    mWidgets[ri] = theWidget;
    MarkAllDirty();
}

void WidgetContainer::MarkAllDirty()
{
    mDirty = true;
    int i;
    for (i = 0; i < mWidgetCount; i++)
    {
        mWidgets[i]->MarkDirty();
    }
}

void WidgetContainer::DisableWidget(Widget* theWidget)
{
    theWidget->mDisabled = true;
}

void WidgetContainer::EnableWidget(Widget* theWidget)
{
    theWidget->mDisabled = false;
}

} // namespace Sexy
