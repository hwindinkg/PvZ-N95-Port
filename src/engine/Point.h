#ifndef POINT_H
#define POINT_H

#include "Common.h"

namespace Sexy {

// ---------------------------------------------------------------------------
// TPoint  -- templated 2D coordinate
// ---------------------------------------------------------------------------
template<class _T> class TPoint
{
public:
    _T mX;
    _T mY;

    TPoint() : mX(0), mY(0) {}

    TPoint(_T theX, _T theY) : mX(theX), mY(theY) {}

    bool operator==(const TPoint<_T>& p) const
    {
        return (mX == p.mX) && (mY == p.mY);
    }

    bool operator!=(const TPoint<_T>& p) const
    {
        return !(*this == p);
    }

    TPoint operator+(const TPoint& p) const { return TPoint(mX + p.mX, mY + p.mY); }
    TPoint operator-(const TPoint& p) const { return TPoint(mX - p.mX, mY - p.mY); }
    TPoint operator*(const TPoint& p) const { return TPoint(mX * p.mX, mY * p.mY); }
    TPoint operator/(const TPoint& p) const { return TPoint(mX / p.mX, mY / p.mY); }

    TPoint& operator+=(const TPoint& p) { mX += p.mX; mY += p.mY; return *this; }
    TPoint& operator-=(const TPoint& p) { mX -= p.mX; mY -= p.mY; return *this; }
    TPoint& operator*=(const TPoint& p) { mX *= p.mX; mY *= p.mY; return *this; }
    TPoint& operator/=(const TPoint& p) { mX /= p.mX; mY /= p.mY; return *this; }

    TPoint operator*(_T s) const { return TPoint(mX * s, mY * s); }
    TPoint operator/(_T s) const { return TPoint(mX / s, mY / s); }
};

typedef TPoint<int> Point;
typedef TPoint<double> FPoint;

} // namespace Sexy

#endif // POINT_H
