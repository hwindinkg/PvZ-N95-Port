#include "WidgetManager.h"
#include "Graphics.h"

namespace Sexy {

WidgetManager::WidgetManager()
    : mDefaultTab(NULL)
    , mFocusWidget(NULL)
    , mOverWidget(NULL)
    , mLastDownWidget(NULL)
    , mKeyDownTarget(NULL)
    , mMouseX(0), mMouseY(0)
    , mBaseModalWidget(NULL)
    , mPreModalCount(0)
{
}

WidgetManager::~WidgetManager()
{
}

void WidgetManager::FocusNextWidget(bool next)
{
    int i, j;
    for (i = 0; i < mWidgetCount; i++)
    {
        if (mWidgets[i] == mFocusWidget || !mFocusWidget)
        {
            for (j = 1; j < mWidgetCount; j++)
            {
                int idx = next ? (i + j) % mWidgetCount
                               : (i - j + mWidgetCount * j) % mWidgetCount;
                if (!mWidgets[idx]->mDisabled && mWidgets[idx]->mWantsFocus)
                {
                    SetFocus(mWidgets[idx]);
                    return;
                }
            }
            break;
        }
    }
}

void WidgetManager::SetFocus(Widget* theWidget)
{
    if (mFocusWidget == theWidget)
        return;

    if (mFocusWidget && mFocusWidget->mHasFocus)
    {
        mFocusWidget->mHasFocus = false;
        mFocusWidget->LostFocus();
    }

    mFocusWidget = theWidget;

    if (theWidget)
    {
        theWidget->mHasFocus = true;
        theWidget->GotFocus();
    }
}

void WidgetManager::DrawScreen(Graphics* g)
{
    DrawAll(g);
    mDirty = false;
}

void WidgetManager::UpdateFrame()
{
    UpdateAll();
}

void WidgetManager::UpdateFrameF(float f)
{
    UpdateAllF(f);
}

Widget* WidgetManager::FindWidget(int x, int y, bool /*checkTransparencies*/)
{
    int i;
    for (i = mWidgetCount - 1; i >= 0; i--)
    {
        Widget* w = mWidgets[i];
        if (w->mVisible && !w->mDisabled && w->Contains(x, y))
            return w;
    }
    return NULL;
}

void WidgetManager::RemapMouse(int& /*x*/, int& /*y*/)
{
    // No remapping by default (handled by platform later)
}

void WidgetManager::MouseDown(int x, int y, int btn)
{
    mMouseX = x;
    mMouseY = y;

    // Check modal widget limit
    if (mBaseModalWidget)
    {
        if (mBaseModalWidget->Contains(x, y))
        {
            mBaseModalWidget->MouseDown(x - mBaseModalWidget->mX,
                                         y - mBaseModalWidget->mY, btn);
        }
        return;
    }

    Widget* w = FindWidget(x, y);
    if (w)
    {
        mLastDownWidget = w;
        w->mIsDown = true;
        w->MouseDown(x - w->mX, y - w->mY, btn);
        SetFocus(w);
    }
}

void WidgetManager::MouseUp(int x, int y, int btn)
{
    mMouseX = x;
    mMouseY = y;

    if (mLastDownWidget)
    {
        mLastDownWidget->mIsDown = false;
        mLastDownWidget->MouseUp(x - mLastDownWidget->mX,
                                  y - mLastDownWidget->mY, btn);
        mLastDownWidget = NULL;
    }
}

void WidgetManager::MouseMove(int x, int y)
{
    mMouseX = x;
    mMouseY = y;

    Widget* w = FindWidget(x, y);

    if (w != mOverWidget)
    {
        if (mOverWidget)
            mOverWidget->MouseLeave();
        mOverWidget = w;
        if (w)
            w->MouseEnter();
    }

    if (w)
        w->MouseMove(x - w->mX, y - w->mY);
}

void WidgetManager::MouseDrag(int x, int y)
{
    mMouseX = x;
    mMouseY = y;

    if (mLastDownWidget)
    {
        mLastDownWidget->MouseDrag(x - mLastDownWidget->mX,
                                    y - mLastDownWidget->mY);
    }
}

void WidgetManager::MouseWheel(int delta)
{
    if (mOverWidget)
        mOverWidget->MouseWheel(delta);
}

bool WidgetManager::KeyDown(KeyCode key)
{
    if (mFocusWidget)
        mFocusWidget->KeyDown(key);
    return false;
}

bool WidgetManager::KeyUp(KeyCode key)
{
    if (mFocusWidget)
        return mFocusWidget->KeyUp(key);
    return false;
}

void WidgetManager::KeyChar(char c)
{
    if (mFocusWidget)
        mFocusWidget->KeyChar(c);
}

} // namespace Sexy
