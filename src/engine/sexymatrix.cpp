#include "SexyMatrix.h"
#include "Common.h"
#include <math.h>

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
// SexyMatrix3
///////////////////////////////////////////////////////////////////////////////
SexyMatrix3::SexyMatrix3()
{
    // Members are uninitialised by design (matches PopCap behaviour).
}

void SexyMatrix3::ZeroMatrix()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = 0;
}

void SexyMatrix3::LoadIdentity()
{
    m00 = 1; m01 = 0; m02 = 0;
    m10 = 0; m11 = 1; m12 = 0;
    m20 = 0; m21 = 0; m22 = 1;
}

SexyVector2 SexyMatrix3::operator*(const SexyVector2& theVec) const
{
    return SexyVector2(
        m00 * theVec.x + m01 * theVec.y + m02,
        m10 * theVec.x + m11 * theVec.y + m12);
}

SexyVector3 SexyMatrix3::operator*(const SexyVector3& theVec) const
{
    return SexyVector3(
        m00 * theVec.x + m01 * theVec.y + m02 * theVec.z,
        m10 * theVec.x + m11 * theVec.y + m12 * theVec.z,
        m20 * theVec.x + m21 * theVec.y + m22 * theVec.z);
}

SexyMatrix3 SexyMatrix3::operator*(const SexyMatrix3& theMat) const
{
    SexyMatrix3 aResult;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float sum = 0;
            for (int k = 0; k < 3; ++k)
                sum += m[i][k] * theMat.m[k][j];
            aResult.m[i][j] = sum;
        }
    }
    return aResult;
}

const SexyMatrix3& SexyMatrix3::operator*=(const SexyMatrix3& theMat)
{
    return operator=(operator*(theMat));
}

void SexyMatrix3::ToGLMatrix(float* out) const
{
    // Convert 3x3 affine (column-vector convention) to 4x4 column-major GL matrix.
    //
    //   [ m00  m01   0  m02 ]
    //   [ m10  m11   0  m12 ]
    //   [  0    0    1   0  ]
    //   [ m20  m21   0  m22 ]
    //
    // In column-major order (16 floats):
    out[0]  = m00;
    out[1]  = m10;
    out[2]  = 0.0f;
    out[3]  = m20;

    out[4]  = m01;
    out[5]  = m11;
    out[6]  = 0.0f;
    out[7]  = m21;

    out[8]  = 0.0f;
    out[9]  = 0.0f;
    out[10] = 1.0f;
    out[11] = 0.0f;

    out[12] = m02;
    out[13] = m12;
    out[14] = 0.0f;
    out[15] = m22;
}

///////////////////////////////////////////////////////////////////////////////
// SexyTransform2D
///////////////////////////////////////////////////////////////////////////////
SexyTransform2D::SexyTransform2D()
{
    LoadIdentity();
}

SexyTransform2D::SexyTransform2D(bool loadIdentity)
{
    if (loadIdentity)
        LoadIdentity();
}

SexyTransform2D::SexyTransform2D(const SexyMatrix3& theMatrix)
    : SexyMatrix3(theMatrix)
{
}

const SexyTransform2D& SexyTransform2D::operator=(const SexyMatrix3& theMat)
{
    SexyMatrix3::operator=(theMat);
    return *this;
}

void SexyTransform2D::Translate(float tx, float ty)
{
    SexyMatrix3 aMat;
    aMat.LoadIdentity();
    aMat.m02 = tx;
    aMat.m12 = ty;
    aMat.m22 = 1;
    *this = aMat * (*this);
}

void SexyTransform2D::RotateRad(float rot)
{
    SexyMatrix3 aMat;
    aMat.LoadIdentity();
    float sinRot = -sin(rot);
    float cosRot = cos(rot);
    aMat.m00 = cosRot;
    aMat.m01 = -sinRot;
    aMat.m10 = sinRot;
    aMat.m11 = cosRot;
    *this = aMat * (*this);
}

void SexyTransform2D::RotateDeg(float rot)
{
    RotateRad(3.14159265358979323846f * rot / 180.0f);
}

void SexyTransform2D::Scale(float sx, float sy)
{
    SexyMatrix3 aMat;
    aMat.LoadIdentity();
    aMat.m00 = sx;
    aMat.m11 = sy;
    *this = aMat * (*this);
}

///////////////////////////////////////////////////////////////////////////////
// Transform
///////////////////////////////////////////////////////////////////////////////
Transform::Transform()
    : mMatrix(false)
{
    Reset();
}

void Transform::Reset()
{
    mNeedCalcMatrix = true;
    mComplex = false;
    mTransX1 = mTransY1 = 0;
    mTransX2 = mTransY2 = 0;
    mScaleX = mScaleY = 1;
    mRot = 0;
    mHaveRot = false;
    mHaveScale = false;
}

void Transform::Translate(float tx, float ty)
{
    if (!mComplex)
    {
        mNeedCalcMatrix = true;
        if (mHaveRot || mHaveScale)
        {
            mTransX2 += tx;
            mTransY2 += ty;
        }
        else
        {
            mTransX1 += tx;
            mTransY1 += ty;
        }
    }
    else
    {
        mMatrix.Translate(tx, ty);
    }
}

void Transform::RotateRad(float rot)
{
    if (!mComplex)
    {
        if (mHaveScale)
        {
            MakeComplex();
            mMatrix.RotateRad(rot);
        }
        else
        {
            mNeedCalcMatrix = true;
            mHaveRot = true;
            mRot += rot;
        }
    }
    else
    {
        mMatrix.RotateRad(rot);
    }
}

void Transform::RotateDeg(float rot)
{
    RotateRad(3.14159265358979323846f * rot / 180.0f);
}

void Transform::Scale(float sx, float sy)
{
    if (!mComplex)
    {
        if (mHaveRot || mTransX1 != 0 || mTransY1 != 0 ||
            (sx < 0 && mScaleX * sx != -1) || sy < 0)
        {
            MakeComplex();
            mMatrix.Scale(sx, sy);
        }
        else
        {
            mNeedCalcMatrix = true;
            mHaveScale = true;
            mScaleX *= sx;
            mScaleY *= sy;
        }
    }
    else
    {
        mMatrix.Scale(sx, sy);
    }
}

void Transform::MakeComplex()
{
    if (!mComplex)
    {
        mComplex = true;
        CalcMatrix();
    }
}

void Transform::CalcMatrix() const
{
    if (mNeedCalcMatrix)
    {
        mNeedCalcMatrix = false;
        mMatrix.LoadIdentity();
        mMatrix.m02 = mTransX1;
        mMatrix.m12 = mTransY1;
        mMatrix.m22 = 1;

        if (mHaveScale)
        {
            mMatrix.m00 = mScaleX;
            mMatrix.m11 = mScaleY;
        }
        else if (mHaveRot)
        {
            mMatrix.RotateRad(mRot);
        }

        if (mTransX2 != 0 || mTransY2 != 0)
            mMatrix.Translate(mTransX2, mTransY2);
    }
}

const SexyTransform2D& Transform::GetMatrix() const
{
    CalcMatrix();
    return mMatrix;
}
