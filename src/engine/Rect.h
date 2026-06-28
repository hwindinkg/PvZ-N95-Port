#ifndef RECT_H
#define RECT_H

#include "Common.h"
#include "Point.h"

namespace Sexy {

// ---------------------------------------------------------------------------
// TRect  -- templated axis-aligned rectangle
// ---------------------------------------------------------------------------
template<class _T> class TRect
{
public:
    _T mX;
    _T mY;
    _T mWidth;
    _T mHeight;

    TRect() : mX(0), mY(0), mWidth(0), mHeight(0) {}

    TRect(_T theX, _T theY, _T theWidth, _T theHeight)
        : mX(theX), mY(theY), mWidth(theWidth), mHeight(theHeight)
    {
    }

    bool Intersects(const TRect<_T>& theRect) const
    {
        return !((theRect.mX + theRect.mWidth  <= mX) ||
                 (theRect.mY + theRect.mHeight <= mY) ||
                 (theRect.mX >= mX + mWidth)  ||
                 (theRect.mY >= mY + mHeight));
    }

    TRect<_T> Intersection(const TRect<_T>& theRect) const
    {
        _T x1 = MAX(mX, theRect.mX);
        _T x2 = MIN(mX + mWidth,  theRect.mX + theRect.mWidth);
        _T y1 = MAX(mY, theRect.mY);
        _T y2 = MIN(mY + mHeight, theRect.mY + theRect.mHeight);
        if (x2 - x1 < 0 || y2 - y1 < 0)
            return TRect<_T>(0, 0, 0, 0);
        return TRect<_T>(x1, y1, x2 - x1, y2 - y1);
    }

    TRect<_T> Union(const TRect<_T>& theRect) const
    {
        _T x1 = MIN(mX, theRect.mX);
        _T x2 = MAX(mX + mWidth,  theRect.mX + theRect.mWidth);
        _T y1 = MIN(mY, theRect.mY);
        _T y2 = MAX(mY + mHeight, theRect.mY + theRect.mHeight);
        return TRect<_T>(x1, y1, x2 - x1, y2 - y1);
    }

    bool Contains(_T theX, _T theY) const
    {
        return (theX >= mX) && (theX < mX + mWidth) &&
               (theY >= mY) && (theY < mY + mHeight);
    }

    bool Contains(const TPoint<_T>& thePoint) const
    {
        return (thePoint.mX >= mX) && (thePoint.mX < mX + mWidth) &&
               (thePoint.mY >= mY) && (thePoint.mY < mY + mHeight);
    }

    void Offset(_T dx, _T dy)
    {
        mX += dx;
        mY += dy;
    }

    void Offset(const TPoint<_T>& thePoint)
    {
        mX += thePoint.mX;
        mY += thePoint.mY;
    }

    TRect Inflate(_T dx, _T dy)
    {
        mX -= dx;
        mWidth  += dx * 2;
        mY -= dy;
        mHeight += dy * 2;
        return *this;
    }

    bool operator==(const TRect<_T>& theRect) const
    {
        return (mX == theRect.mX) && (mY == theRect.mY) &&
               (mWidth == theRect.mWidth) && (mHeight == theRect.mHeight);
    }

    bool operator!=(const TRect<_T>& theRect) const
    {
        return !(*this == theRect);
    }

    // Convenience accessors
    _T GetWidth()  const { return mWidth; }
    _T GetHeight() const { return mHeight; }
    _T GetLeft()   const { return mX; }
    _T GetTop()    const { return mY; }
    _T GetRight()  const { return mX + mWidth; }
    _T GetBottom() const { return mY + mHeight; }
};

typedef TRect<int>   Rect;
typedef TRect<double> FRect;

} // namespace Sexy

#endif // RECT_H
