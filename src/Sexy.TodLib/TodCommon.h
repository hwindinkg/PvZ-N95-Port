/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * Ported from PvZ-Portable Sexy.TodLib/TodCommon.h for Symbian S60 3rd FP1.
 * Math utilities, animation curves, weighted random selection, drawing helpers.
 */

#ifndef TODCOMMON_H
#define TODCOMMON_H

#include <stdlib.h>
#include <cmath>
#include <cfloat>
#include <string>

#include "../engine/Common.h"
#include "../engine/Rect.h"
#include <e32math.h>
#include "TodDebug.h"

// Forward declarations for types used in function signatures below.
// GCCE 3.4.3 treats unknown elaborated type specifiers as 'int', so we must
// pre-declare them.
namespace Sexy {
class Graphics;
class Image;
class MemoryImage;
class SexyMatrix3;
class SexyVector2;
class Color;
}
// [M4 #4 fix] _Font is typedef'd to Sexy::Font in ResourceManager.h / Stubs.h /
// SystemFont.h. Forward-declaring `class _Font` here would CONFLICT with that
// typedef. Include Font.h + use the guarded typedef instead.
#include "../engine/Font.h"
#ifndef _FONT_TYPEDEF_DEFINED
#define _FONT_TYPEDEF_DEFINED
typedef Sexy::Font _Font;
#endif

// ---------------------------------------------------------------------------
// Sexy_Rand  --  replacement for Sexy::Rand() using Symbian Math::Random()
// ---------------------------------------------------------------------------
inline int Sexy_Rand(int range)
{
        return (range > 0) ? (Math::Random() % range) : 0;
}

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define RENDERIMAGEFLAG_SANDING 0x1000
#define DEG_TO_RAD(deg) ((deg) * 0.017453292f)
#define RAD_TO_DEG(rad) ((rad) * 57.29578f)

// ---------------------------------------------------------------------------
// TodCurves  --  animation curve types
// ---------------------------------------------------------------------------
enum TodCurves
{
        CURVE_CONSTANT,
        CURVE_LINEAR,
        CURVE_EASE_IN,
        CURVE_EASE_OUT,
        CURVE_EASE_IN_OUT,
        CURVE_EASE_IN_OUT_WEAK,
        CURVE_FAST_IN_OUT,
        CURVE_FAST_IN_OUT_WEAK,
        CURVE_BOUNCE,
        CURVE_BOUNCE_FAST_MIDDLE,
        CURVE_BOUNCE_SLOW_MIDDLE,
        CURVE_SIN_WAVE,
        CURVE_EASE_SIN_WAVE,
        CURVE_NUM
};


// ---------------------------------------------------------------------------
// Structs for weighted random selection
// ---------------------------------------------------------------------------
struct TodWeightedArray
{
        intptr_t   mItem;
        int32_t    mWeight;
};

struct TodWeightedGridArray
{
        int32_t    mX;
        int32_t    mY;
        int32_t    mWeight;
};

class TodSmoothArray
{
public:
        int32_t    mItem;
        float      mWeight;
        float      mLastPicked;
        float      mSecondLastPicked;
};

// ---------------------------------------------------------------------------
// Template: pick random element from array
// ---------------------------------------------------------------------------
template <typename T>
inline T TodPickFromArray(const T* theArray, int theCount)
{
        TOD_ASSERT(theCount > 0);
        return theCount > 0 ? theArray[Sexy_Rand(theCount)] : T();
}

// ---------------------------------------------------------------------------
// Weighted / smooth random selection
// ---------------------------------------------------------------------------
intptr_t             TodPickFromWeightedArray(const TodWeightedArray* theArray, int theCount);
TodWeightedArray*    TodPickArrayItemFromWeightedArray(const TodWeightedArray* theArray, int theCount);
TodWeightedGridArray* TodPickFromWeightedGridArray(const TodWeightedGridArray* theArray, int theCount);
float                TodCalcSmoothWeight(float aWeight, float aLastPicked, float aSecondLastPicked);
void                 TodUpdateSmoothArrayPick(TodSmoothArray* theArray, int theCount, int thePickIndex);
int                  TodPickFromSmoothArray(TodSmoothArray* theArray, int theCount);

// ---------------------------------------------------------------------------
// Animation curve primitives
// ---------------------------------------------------------------------------
float TodCurveQuad(float theTime);
float TodCurveInvQuad(float theTime);
float TodCurveS(float theTime);
float TodCurveInvQuadS(float theTime);
float TodCurveBounce(float theTime);
float TodCurveQuadS(float theTime);
float TodCurveCubic(float theTime);
float TodCurveInvCubic(float theTime);
float TodCurveCubicS(float theTime);
float TodCurvePoly(float theTime, float thePoly);
float TodCurveInvPoly(float theTime, float thePoly);
float TodCurvePolyS(float theTime, float thePoly);
float TodCurveCircle(float theTime);
float TodCurveInvCircle(float theTime);
float TodCurveEvaluate(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve);
float TodCurveEvaluateClamped(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve);
float TodAnimateCurveFloatTime(float theTimeStart, float theTimeEnd, float theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve);
float TodAnimateCurveFloat(int theTimeStart, int theTimeEnd, int theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve);
int   TodAnimateCurve(int theTimeStart, int theTimeEnd, int theTimeAge, int thePositionStart, int thePositionEnd, TodCurves theCurve);

// ---------------------------------------------------------------------------
// Matrix operations
// ---------------------------------------------------------------------------
/*inline*/ void  TodScaleTransformMatrix(class Sexy::SexyMatrix3& m, float x, float y, float theScaleX, float theScaleY);
void             TodScaleRotateTransformMatrix(class Sexy::SexyMatrix3& m, float x, float y, float rad, float theScaleX, float theScaleY);
void             SexyMatrix3ExtractScale(const class Sexy::SexyMatrix3& m, float& theScaleX, float& theScaleY);
/*inline*/ void  SexyMatrix3Translation(class Sexy::SexyMatrix3& m, float x, float y);
void             SexyMatrix3Transpose(const class Sexy::SexyMatrix3& m, class Sexy::SexyMatrix3& r);
void             SexyMatrix3Inverse(const class Sexy::SexyMatrix3& m, class Sexy::SexyMatrix3& r);
void             SexyMatrix3Multiply(class Sexy::SexyMatrix3& m, const class Sexy::SexyMatrix3& l, const class Sexy::SexyMatrix3& r);
bool             TodIsPointInPolygon(const class Sexy::SexyVector2* thePolygonPoint, int theNumberPolygonPoints, const class Sexy::SexyVector2& theCheckPoint);

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void             TodDrawString(class Sexy::Graphics* g, const std::string& theText, int thePosX, int thePosY, _Font* theFont, const class Sexy::Color& theColor, int theJustification);
void             TodDrawImageScaledF(class Sexy::Graphics* g, class Sexy::Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY);
void             TodDrawImageCenterScaledF(class Sexy::Graphics* g, class Sexy::Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY);
void             TodDrawImageCelF(class Sexy::Graphics* g, class Sexy::Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow);
void             TodDrawImageCelScaled(class Sexy::Graphics* g, class Sexy::Image* theImageStrip, int thePosX, int thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY);
void             TodDrawImageCelScaledF(class Sexy::Graphics* g, class Sexy::Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY);
void             TodDrawImageCelCenterScaledF(class Sexy::Graphics* g, class Sexy::Image* theImageStrip, float thePosX, float thePosY, int theCelCol, float theScaleX, float theScaleY);
void             TodBltMatrix(class Sexy::Graphics* g, class Sexy::Image* theImage, const class Sexy::SexyMatrix3& theTransform, const Sexy::Rect& theClipRect, const class Sexy::Color& theColor, int theDrawMode, const Sexy::Rect& theSrcRect);
void             TodMarkImageForSanding(class Sexy::Image* theImage);
void             TodSandImageIfNeeded(class Sexy::Image* theImage);
void             FixPixelsOnAlphaEdgeForBlending(class Sexy::Image* theImage);
uint32_t         AverageNearByPixels(class Sexy::MemoryImage* theImage, uint32_t* thePixel, int x, int y);

// ---------------------------------------------------------------------------
// String utilities
// ---------------------------------------------------------------------------
std::string      TodReplaceString(const std::string& theText, const char* theStringToFind, const std::string& theStringToSubstitute);
std::string      TodReplaceNumberString(const std::string& theText, const char* theStringToFind, int theNumber);
int              TodSnprintf(char* theBuffer, int theSize, const char* theFormat, ...);
int              TodVsnprintf(char* theBuffer, int theSize, const char* theFormat, va_list theArgList);

// ---------------------------------------------------------------------------
// Application query stubs
// ---------------------------------------------------------------------------
std::string      TodGetCurrentLevelName();
bool             TodHasUsedCheatKeys();
bool             TodAppCloseRequest();

// ---------------------------------------------------------------------------
// Inline math helpers
// ---------------------------------------------------------------------------
/*inline*/ int        RandRangeInt(int theMin, int theMax);
/*inline*/ float      RandRangeFloat(float theMin, float theMax);

inline char           ClampByte(char theNum, char theMin, char theMax)           { return theNum <= theMin ? theMin : theNum >= theMax ? theMax : theNum; }
inline int            ClampInt(int theNum, int theMin, int theMax)               { return theNum <= theMin ? theMin : theNum >= theMax ? theMax : theNum; }
inline float          ClampFloat(float theNum, float theMin, float theMax)       { return theNum <= theMin ? theMin : theNum >= theMax ? theMax : theNum; }
inline float          Distance2D(float x1, float y1, float x2, float y2)        { return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); }
inline float          FloatLerp(float theStart, float theEnd, float theFactor)   { return theStart + theFactor * (theEnd - theStart); }
inline int            FloatRoundToInt(float theFloatValue)                       { return theFloatValue > 0 ? static_cast<int>(theFloatValue + 0.5f) : static_cast<int>(theFloatValue - 0.5f); }
inline bool           FloatApproxEqual(float theFloatVal1, float theFloatVal2)   { return fabs(theFloatVal1 - theFloatVal2) < FLT_EPSILON; }

// ---------------------------------------------------------------------------
// Color utilities
// ---------------------------------------------------------------------------
class Sexy::Color     GetFlashingColor(uint32_t theCounter, int theFlashTime);
/*inline*/ int        ColorComponentMultiply(int theColor1, int theColor2);
class Sexy::Color     ColorsMultiply(const class Sexy::Color& theColor1, const class Sexy::Color& theColor2);
class Sexy::Color     ColorAdd(const class Sexy::Color& theColor1, const class Sexy::Color& theColor2);

inline void           SetBit(unsigned int& theNum, int theIdx, bool theValue = true) { if (theValue) theNum |= 1 << theIdx; else theNum &= ~(1 << theIdx); }
inline bool           TestBit(unsigned int theNum, int theIdx)                       { return (theNum & (1 << theIdx)) != 0; }

#endif
