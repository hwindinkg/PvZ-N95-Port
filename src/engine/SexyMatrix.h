#ifndef __SEXYMATRIX_H__
#define __SEXYMATRIX_H__

#include "SexyVector.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
// SexyMatrix3  -- 3x3 affine transformation matrix
//
//  Layout:
//    m00  m01  m02     (scale/skew)  (translate x)
//    m10  m11  m12                   (translate y)
//    m20  m21  m22     (0,0,1 for affine transforms in this project)
///////////////////////////////////////////////////////////////////////////////
class SexyMatrix3
{
public:
    // RVCT/armcc does not enable anonymous structs by default. The
    // #pragma anon_unions directive turns on support for anonymous unions
    // AND anonymous structures so the named members (m00..m22) are visible.
    // Harmless on GCC/MSVC which already accept this layout.
#if defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma anon_unions
#endif
    union
    {
        float m[3][3];
        struct
        {
            float m00, m01, m02;
            float m10, m11, m12;
            float m20, m21, m22;
        };
    };

public:
    SexyMatrix3();
    void ZeroMatrix();
    void LoadIdentity();

    SexyVector2 operator*(const SexyVector2& theVec) const;
    SexyVector3 operator*(const SexyVector3& theVec) const;
    SexyMatrix3 operator*(const SexyMatrix3& theMat) const;
    const SexyMatrix3& operator*=(const SexyMatrix3& theMat);

    // Aliases for PopCap compatibility -----------------------------------
    void Identity() { LoadIdentity(); }
    void Multiply(const SexyMatrix3& a, const SexyMatrix3& b)
    {
        SexyMatrix3 r;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
                float sum = 0;
                for (int k = 0; k < 3; k++)
                    sum += a.m[i][k] * b.m[k][j];
                r.m[i][j] = sum;
            }
        *this = r;
    }

    /// Convert to a 4x4 column-major float array for glLoadMatrixf.
    void ToGLMatrix(float* out4x4) const;
};

///////////////////////////////////////////////////////////////////////////////
// SexyTransform2D  -- convenience wrapper around SexyMatrix3
///////////////////////////////////////////////////////////////////////////////
class SexyTransform2D : public SexyMatrix3
{
public:
    SexyTransform2D();
    explicit SexyTransform2D(bool loadIdentity);
    SexyTransform2D(const SexyMatrix3& theMatrix);

    const SexyTransform2D& operator=(const SexyMatrix3& theMat);

    void Translate(float tx, float ty);

    /// Positive rotation is counter-clockwise (screen coordinates).
    void RotateRad(float rot);
    void RotateDeg(float rot);
    void Scale(float sx, float sy);
};

///////////////////////////////////////////////////////////////////////////////
// Transform  -- decomposed transform (non-uniform, cached matrix)
///////////////////////////////////////////////////////////////////////////////
class Transform
{
protected:
    mutable SexyTransform2D mMatrix;
    mutable bool mNeedCalcMatrix;
    void MakeComplex();
    void CalcMatrix() const;

public:
    bool mComplex, mHaveRot, mHaveScale;
    float mTransX1, mTransY1, mTransX2, mTransY2;
    float mScaleX, mScaleY;
    float mRot;

public:
    Transform();

    void Reset();
    void Translate(float tx, float ty);

    /// Positive rotation is counter-clockwise.
    void RotateRad(float rot);
    void RotateDeg(float rot);
    void Scale(float sx, float sy);

    const SexyTransform2D& GetMatrix() const;
};

} // namespace Sexy

#endif // __SEXYMATRIX_H__
