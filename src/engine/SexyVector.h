#ifndef __SEXYVECTOR_H__
#define __SEXYVECTOR_H__

#include <e32math.h>

inline float sqrtf_symbian(float x) { TReal r; Math::Sqrt(r, (TReal)x); return (float)r; }

#ifndef sqrtf
#define sqrtf(x) sqrtf_symbian((float)(x))
#endif

namespace Sexy
{

// SexyMatrix3 is declared in SexyMatrix.h; we only need pointer/reference
// here, so a forward declaration is enough.
class SexyMatrix3;

///////////////////////////////////////////////////////////////////////////////
// SexyVector2  -- 2D float vector
///////////////////////////////////////////////////////////////////////////////
class SexyVector2
{
public:
    float x;
    float y;

    SexyVector2() : x(0), y(0) {}
    SexyVector2(float theX, float theY) : x(theX), y(theY) {}

    float Dot(const SexyVector2& v) const { return x * v.x + y * v.y; }

    SexyVector2 operator+(const SexyVector2& v) const { return SexyVector2(x + v.x, y + v.y); }
    SexyVector2 operator-(const SexyVector2& v) const { return SexyVector2(x - v.x, y - v.y); }
    SexyVector2 operator-() const { return SexyVector2(-x, -y); }
    SexyVector2 operator*(float t) const { return SexyVector2(t * x, t * y); }
    SexyVector2 operator/(float t) const { return SexyVector2(x / t, y / t); }

    void operator+=(const SexyVector2& v) { x += v.x; y += v.y; }
    void operator-=(const SexyVector2& v) { x -= v.x; y -= v.y; }
    void operator*=(float t) { x *= t; y *= t; }
    void operator/=(float t) { x /= t; y /= t; }

    bool operator==(const SexyVector2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const SexyVector2& v) const { return x != v.x || y != v.y; }

    float Magnitude() const { return sqrtf(x * x + y * y); }
    float MagnitudeSquared() const { return x * x + y * y; }

    SexyVector2 Normalize() const
    {
        float aMag = Magnitude();
        return (aMag != 0) ? (*this) / aMag : *this;
    }

    SexyVector2 Perp() const
    {
        return SexyVector2(-y, x);
    }
};

///////////////////////////////////////////////////////////////////////////////
// SexyVector3  -- 3D float vector
///////////////////////////////////////////////////////////////////////////////
class SexyVector3
{
public:
    float x;
    float y;
    float z;

    SexyVector3() : x(0), y(0), z(0) {}
    SexyVector3(float theX, float theY, float theZ) : x(theX), y(theY), z(theZ) {}

    float Dot(const SexyVector3& v) const { return x * v.x + y * v.y + z * v.z; }

    SexyVector3 Cross(const SexyVector3& v) const
    {
        return SexyVector3(y * v.z - z * v.y,
                           z * v.x - x * v.z,
                           x * v.y - y * v.x);
    }

    SexyVector3 operator+(const SexyVector3& v) const { return SexyVector3(x + v.x, y + v.y, z + v.z); }
    SexyVector3 operator-(const SexyVector3& v) const { return SexyVector3(x - v.x, y - v.y, z - v.z); }
    SexyVector3 operator*(float t) const { return SexyVector3(t * x, t * y, t * z); }
    SexyVector3 operator/(float t) const { return SexyVector3(x / t, y / t, z / t); }

    float Magnitude() const { return sqrtf(x * x + y * y + z * z); }

    SexyVector3 Normalize() const
    {
        float aMag = Magnitude();
        return (aMag != 0) ? (*this) / aMag : *this;
    }
};

} // namespace Sexy

#endif // __SEXYVECTOR_H__
