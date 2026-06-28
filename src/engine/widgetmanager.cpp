#include "WidgetManager.h"
#include "Graphics.h"

namespace Sexy {

// [M4 #2] Virtual d-pad cursor globals (declared in WidgetManager.h).
// Set by CPvZAppUi::HandleKeyEventL, read by WidgetManager::DrawScreen.
int  g_sCursorX = 200;
int  g_sCursorY = 150;
bool g_sCursorVisible = false;

WidgetManager::WidgetManager()
    : mDefaultTab(NULL)
    , mFocusWidget(NULL)
    , mOverWidget(NULL)
    , mLastDownWidget(NULL)
    , mKeyDownTarget(NULL)
    , mMouseX(0), mMouseY(0)
    , mLastMouseX(0), mLastMouseY(0)
    , mWidgetsEnabled(true)
    , mDownButtons(0)
    , mBaseModalWidget(NULL)
    , mPreModalCount(0)
{
    for (int i = 0; i < 256; i++) mKeyDown[i] = false;
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

    // [M4 #2] Draw the virtual d-pad cursor overlay on top of all widgets.
    // The cursor position is set by CPvZAppUi::HandleKeyEventL (d-pad arrows
    // move it 16px/press). Without a visible cursor the user can't tell where
    // their click will land. Drawn as a yellow crosshair + small filled square
    // so it's visible over any background.
    // Cursor position is stored in static globals set by the platform layer
    // (CPvZAppUi) -- declared in WidgetManager.h.
    if (g_sCursorVisible && g_sCursorX >= 0 && g_sCursorY >= 0)
    {
        int cx = g_sCursorX;
        int cy = g_sCursorY;
        // Yellow crosshair.
        g->SetColor(Sexy::Color(255, 255, 0, 255));
        g->DrawRect(cx - 8, cy - 1, 17, 3);   // horizontal bar
        g->DrawRect(cx - 1, cy - 8, 3, 17);   // vertical bar
        // Center filled square (the "click point").
        g->SetColor(Sexy::Color(255, 0, 0, 255));
        g->FillRect(cx - 2, cy - 2, 5, 5);
    }

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
        // [M4 #1 fix] Skip widgets with mMouseVisible == false. GameSelector
        // sets this to false so it doesn't intercept clicks meant for its
        // child buttons (which are top-level widgets in the manager, drawn
        // on top of GameSelector's full-screen background). Without this
        // check, FindWidget returns GameSelector for any point inside its
        // 400x300 rect, blocking all button clicks.
        if (w->mVisible && !w->mDisabled && w->mMouseVisible && w->Contains(x, y))
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
    mLastMouseX = x;
    mLastMouseY = y;
    if (btn >= 0 && btn < 8)
        mDownButtons |= (1 << btn);

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
    mLastMouseX = x;
    mLastMouseY = y;
    if (btn >= 0 && btn < 8)
        mDownButtons &= ~(1 << btn);

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
    mLastMouseX = x;
    mLastMouseY = y;

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
