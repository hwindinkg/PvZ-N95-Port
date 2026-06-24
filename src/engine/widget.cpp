#include "Widget.h"
#include "Graphics.h"
#include "WidgetManager.h"

namespace Sexy {

Widget::Widget()
    : mVisible(true)
    , mMouseVisible(true)
    , mDisabled(false)
    , mHasFocus(false)
    , mIsDown(false)
    , mIsOver(false)
    , mHasTransparencies(false)
    , mDoFinger(false)
    , mWantsFocus(false)
    , mClip(false)
    , mX(0), mY(0), mWidth(0), mHeight(0)
    , mPriority(0)
    , mParent(NULL), mWidgetManager(NULL)
    , mTabPrev(NULL), mTabNext(NULL)
    , mLoadedResourceCount(0)
{
    int i;
    for (i = 0; i < 8; i++)
        mColors[i] = Color(255, 255, 255, 255);
    for (i = 0; i < 64; i++)
        mLoadedResourceNames[i] = NULL;
}

Widget::~Widget()
{
    int i;
    for (i = 0; i < mLoadedResourceCount; i++)
        delete[] mLoadedResourceNames[i];
}

void Widget::Draw(Graphics* /*g*/) {}
void Widget::Update() {}
void Widget::UpdateF(float /*f*/) {}
void Widget::GotFocus() {}
void Widget::LostFocus() {}

void Widget::MouseDown(int /*x*/, int /*y*/, int /*btn*/) {}
void Widget::MouseUp(int /*x*/, int /*y*/, int /*btn*/) {}
void Widget::MouseMove(int /*x*/, int /*y*/) {}
void Widget::MouseDrag(int /*x*/, int /*y*/) {}
void Widget::MouseWheel(int /*delta*/) {}
void Widget::MouseEnter() {}
void Widget::MouseLeave() {}

void Widget::KeyDown(KeyCode /*key*/) { }
bool Widget::KeyUp(KeyCode /*key*/) { return false; }
void Widget::KeyChar(char /*c*/) { }

void Widget::Resize(int x, int y, int w, int h)
{
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;
    MarkDirty();
}

void Widget::Move(int x, int y)
{
    mX = x;
    mY = y;
    MarkDirty();
}

bool Widget::Contains(int x, int y) const
{
    return x >= mX && x < mX + mWidth && y >= mY && y < mY + mHeight;
}

void Widget::MarkDirty()
{
    if (mWidgetManager)
        mWidgetManager->mDirty = true;
}

int Widget::GetAbsX() const
{
    if (mParent)
        return mParent->GetAbsX() + mX;
    return mX;
}

int Widget::GetAbsY() const
{
    if (mParent)
        return mParent->GetAbsY() + mY;
    return mY;
}

} // namespace Sexy
