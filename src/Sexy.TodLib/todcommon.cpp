/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * Ported from PvZ-Portable Sexy.TodLib/TodCommon.cpp for Symbian S60 3rd FP1.
 * Animation curves, weighted arrays, matrix math, string utilities, and
 * drawing stubs.
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// GCCE math compatibility
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../engine/Common.h"
#include "../engine/Color.h"
#include "../engine/SexyMatrix.h"
#include "../engine/Rect.h"

// Bring Sexy types into scope for stub function definitions below
using namespace Sexy;
#include "../engine/SexyVector.h"
#include "../engine/Rect.h"
#include "TodCommon.h"
#include "TodDebug.h"

// ===========================================================================
// Weighted random selection
// ===========================================================================

intptr_t TodPickFromWeightedArray(const TodWeightedArray* theArray, int theCount)
{
	return TodPickArrayItemFromWeightedArray(theArray, theCount)->mItem;
}

TodWeightedArray* TodPickArrayItemFromWeightedArray(const TodWeightedArray* theArray, int theCount)
{
	if (theCount <= 0)
		return NULL;

	int aTotalWeight = 0;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0);

	aTotalWeight = Sexy_Rand(aTotalWeight);

	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight -= theArray[i].mWeight;
		if (aTotalWeight < 0)
		{
			return const_cast<TodWeightedArray*>(&theArray[i]);
		}
	}

	TOD_ASSERT(false);
	return NULL;
}

TodWeightedGridArray* TodPickFromWeightedGridArray(const TodWeightedGridArray* theArray, int theCount)
{
	if (theCount <= 0)
		return NULL;

	int aTotalWeight = 0;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0);

	aTotalWeight = Sexy_Rand(aTotalWeight);

	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight -= theArray[i].mWeight;
		if (aTotalWeight < 0)
		{
			return const_cast<TodWeightedGridArray*>(&theArray[i]);
		}
	}

	TOD_ASSERT(false);
	return NULL;
}

// ===========================================================================
// Smooth array functions
// ===========================================================================

float TodCalcSmoothWeight(float aWeight, float aLastPicked, float aSecondLastPicked)
{
	if (aWeight < 1E-6f)
	{
		return 0.0f;
	}

	float aExpectedLength1 = 1.0f / aWeight;
	float aExpectedLength2 = aExpectedLength1 * 2.0f;
	float aAdvancedLength1 = aLastPicked + 1.0f - aExpectedLength1;
	float aAdvancedLength2 = aSecondLastPicked + 1.0f - aExpectedLength2;
	float aFactor1 = 1.0f + aAdvancedLength1 / aExpectedLength1 * 2.0f;
	float aFactor2 = 1.0f + aAdvancedLength2 / aExpectedLength2 * 2.0f;
	float aFactorFinal = ClampFloat(aFactor1 * 0.75f + aFactor2 * 0.25f, 0.01f, 100.0f);
	return aWeight * aFactorFinal;
}

int TodPickFromSmoothArray(TodSmoothArray* theArray, int theCount)
{
	float aTotalWeight = 0.0f;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0.0f);

	float aNormalizeFactor = 1.0f / aTotalWeight;
	float aTotalAdjustedWeight = 0.0f;
	for (int j = 0; j < theCount; j++)
	{
		aTotalAdjustedWeight += TodCalcSmoothWeight(theArray[j].mWeight * aNormalizeFactor, theArray[j].mLastPicked, theArray[j].mSecondLastPicked);
	}
	TOD_ASSERT(aTotalAdjustedWeight > 0.0f);

	float aRandWeight = static_cast<float>(Sexy_Rand(static_cast<int>(aTotalAdjustedWeight + 1.0f)));
	float aAccumulatedWeight = 0.0f;
	int k;
	for (k = 0; k < theCount - 1; k++)
	{
		aAccumulatedWeight += TodCalcSmoothWeight(theArray[k].mWeight * aNormalizeFactor, theArray[k].mLastPicked, theArray[k].mSecondLastPicked);
		if (aRandWeight <= aAccumulatedWeight)
		{
			break;
		}
	}

	TodUpdateSmoothArrayPick(theArray, theCount, k);
	return theArray[k].mItem;
}

void TodUpdateSmoothArrayPick(TodSmoothArray* theArray, int theCount, int thePickIndex)
{
	for (int i = 0; i < theCount; i++)
	{
		if (theArray[i].mWeight > 0.0f)
		{
			theArray[i].mLastPicked += 1.0f;
			theArray[i].mSecondLastPicked += 1.0f;
		}
	}

	theArray[thePickIndex].mSecondLastPicked = theArray[thePickIndex].mLastPicked;
	theArray[thePickIndex].mLastPicked = 0.0f;
}

// ===========================================================================
// Animation curve primitives
// ===========================================================================

float TodCurveQuad(float theTime)
{
	return theTime * theTime;
}

float TodCurveInvQuad(float theTime)
{
	return 2.0f * theTime - theTime * theTime;
}

float TodCurveS(float theTime)
{
	return 3.0f * theTime * theTime - 2.0f * theTime * theTime * theTime;
}

float TodCurveInvQuadS(float theTime)
{
	if (theTime <= 0.5f)
	{
		return TodCurveInvQuad(theTime * 2.0f) * 0.5f;
	}
	return TodCurveQuad((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurveBounce(float theTime)
{
	return 1.0f - fabs(2.0f * theTime - 1.0f);
}

float TodCurveQuadS(float theTime)
{
	if (theTime <= 0.5f)
	{
		return TodCurveQuad(theTime * 2.0f) * 0.5f;
	}
	return TodCurveInvQuad((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurveCubic(float theTime)
{
	return theTime * theTime * theTime;
}

float TodCurveInvCubic(float theTime)
{
	float t = theTime - 1.0f;
	return t * t * t + 1.0f;
}

float TodCurveCubicS(float theTime)
{
	if (theTime <= 0.5f)
	{
		return TodCurveCubic(theTime * 2.0f) * 0.5f;
	}
	return TodCurveInvCubic((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurvePoly(float theTime, float thePoly)
{
	return static_cast<float>(pow(theTime, thePoly));
}

float TodCurveInvPoly(float theTime, float thePoly)
{
	return static_cast<float>(pow(theTime - 1.0f, thePoly)) + 1.0f;
}

float TodCurvePolyS(float theTime, float thePoly)
{
	if (theTime <= 0.5f)
	{
		return TodCurvePoly(theTime * 2.0f, thePoly) * 0.5f;
	}
	return TodCurveInvPoly((theTime - 0.5f) * 2.0f, thePoly) * 0.5f + 0.5f;
}

float TodCurveCircle(float theTime)
{
	if (theTime > 1.0f - 1.0e-6f)
	{
		return 1.0f;
	}
	return 1.0f - static_cast<float>(sqrt(1.0f - theTime * theTime));
}

float TodCurveInvCircle(float theTime)
{
	if (theTime < 1.0e-6f)
	{
		return 0.0f;
	}
	return static_cast<float>(sqrt(1.0f - (theTime - 1.0f) * (theTime - 1.0f)));
}

// ===========================================================================
// Curve evaluation
// ===========================================================================

float TodCurveEvaluate(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	float aWarpedTime = 0;
	switch (theCurve)
	{
	case CURVE_CONSTANT:            aWarpedTime = 0;                                                    break;
	case CURVE_LINEAR:              aWarpedTime = theTime;                                              break;
	case CURVE_EASE_IN:             aWarpedTime = TodCurveQuad(theTime);                                break;
	case CURVE_EASE_OUT:            aWarpedTime = TodCurveInvQuad(theTime);                             break;
	case CURVE_EASE_IN_OUT:         aWarpedTime = TodCurveS(TodCurveS(theTime));                        break;
	case CURVE_EASE_IN_OUT_WEAK:    aWarpedTime = TodCurveS(theTime);                                  break;
	case CURVE_FAST_IN_OUT:         aWarpedTime = TodCurveInvQuadS(TodCurveInvQuadS(theTime));          break;
	case CURVE_FAST_IN_OUT_WEAK:    aWarpedTime = TodCurveInvQuadS(theTime);                            break;
	case CURVE_BOUNCE:              aWarpedTime = TodCurveBounce(theTime);                              break;
	case CURVE_BOUNCE_FAST_MIDDLE:  aWarpedTime = TodCurveQuad(TodCurveBounce(theTime));                break;
	case CURVE_BOUNCE_SLOW_MIDDLE:  aWarpedTime = TodCurveInvQuad(TodCurveBounce(theTime));             break;
	case CURVE_SIN_WAVE:            aWarpedTime = sin(2.0f * static_cast<float>(M_PI) * theTime);      break;
	case CURVE_EASE_SIN_WAVE:       aWarpedTime = sin(2.0f * static_cast<float>(M_PI) * TodCurveS(theTime)); break;
	default:                        TOD_ASSERT(false);                                                  break;
	}
	return (thePositionEnd - thePositionStart) * aWarpedTime + thePositionStart;
}

float TodCurveEvaluateClamped(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	if (theTime <= 0.0f)
	{
		return thePositionStart;
	}

	if (theTime >= 1.0f)
	{
		if (theCurve == CURVE_BOUNCE ||
			theCurve == CURVE_BOUNCE_FAST_MIDDLE ||
			theCurve == CURVE_BOUNCE_SLOW_MIDDLE ||
			theCurve == CURVE_SIN_WAVE ||
			theCurve == CURVE_EASE_SIN_WAVE)
		{
			return thePositionStart;
		}
		else
		{
			return thePositionEnd;
		}
	}

	return TodCurveEvaluate(theTime, thePositionStart, thePositionEnd, theCurve);
}

float TodAnimateCurveFloatTime(float theTimeStart, float theTimeEnd, float theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	float aWarpedAge = (theTimeAge - theTimeStart) / (theTimeEnd - theTimeStart);
	return TodCurveEvaluateClamped(aWarpedAge, thePositionStart, thePositionEnd, theCurve);
}

float TodAnimateCurveFloat(int theTimeStart, int theTimeEnd, int theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	float aWarpedAge = (theTimeAge - theTimeStart) / static_cast<float>(theTimeEnd - theTimeStart);
	return TodCurveEvaluateClamped(aWarpedAge, thePositionStart, thePositionEnd, theCurve);
}

int TodAnimateCurve(int theTimeStart, int theTimeEnd, int theTimeAge, int thePositionStart, int thePositionEnd, TodCurves theCurve)
{
	return FloatRoundToInt(TodAnimateCurveFloat(theTimeStart, theTimeEnd, theTimeAge, static_cast<float>(thePositionStart), static_cast<float>(thePositionEnd), theCurve));
}

// ===========================================================================
// RandRange helpers
// ===========================================================================

int RandRangeInt(int theMin, int theMax)
{
	TOD_ASSERT(theMin <= theMax);
	return Sexy_Rand(theMax - theMin + 1) + theMin;
}

float RandRangeFloat(float theMin, float theMax)
{
	TOD_ASSERT(theMin <= theMax);
	return static_cast<float>(Sexy_Rand(static_cast<int>(theMax - theMin))) + theMin;
}

// ===========================================================================
// Matrix operations
// ===========================================================================

void SexyMatrix3Translation(SexyMatrix3& m, float x, float y)
{
	m.m02 += x;
	m.m12 += y;
}

void TodScaleTransformMatrix(SexyMatrix3& m, float x, float y, float theScaleX, float theScaleY)
{
	m.m00 = theScaleX;
	m.m10 = 0.0f;
	m.m20 = 0.0f;
	m.m01 = 0.0f;
	m.m11 = theScaleY;
	m.m21 = 0.0f;
	m.m02 = x;
	m.m12 = y;
	m.m22 = 1.0f;
}

void TodScaleRotateTransformMatrix(SexyMatrix3& m, float x, float y, float rad, float theScaleX, float theScaleY)
{
	m.m00 = cos(rad) * theScaleX;
	m.m10 = -sin(rad) * theScaleX;
	m.m20 = 0.0f;
	m.m01 = sin(rad) * theScaleY;
	m.m11 = cos(rad) * theScaleY;
	m.m21 = 0.0f;
	m.m02 = x;
	m.m12 = y;
	m.m22 = 1.0f;
}

void SexyMatrix3ExtractScale(const SexyMatrix3& m, float& theScaleX, float& theScaleY)
{
	float kx = atan2(m.m00, m.m10);
	if (fabs(kx) < static_cast<float>(M_PI) / 4.0f || fabs(kx) > 4.0f * static_cast<float>(M_PI) / 3.0f)
	{
		theScaleX = m.m10 / cos(kx);
	}
	else
	{
		theScaleX = m.m00 / sin(kx);
	}

	float ky = atan2(m.m11, m.m01);
	if (fabs(ky) < static_cast<float>(M_PI) / 4.0f || fabs(ky) > 4.0f * static_cast<float>(M_PI) / 3.0f)
	{
		theScaleY = m.m01 / cos(ky);
	}
	else
	{
		theScaleY = m.m11 / sin(ky);
	}
}

void SexyMatrix3Transpose(const SexyMatrix3& m, SexyMatrix3& r)
{
	SexyMatrix3 temp;
	temp.m00 = m.m00;
	temp.m01 = m.m10;
	temp.m02 = m.m20;
	temp.m10 = m.m01;
	temp.m11 = m.m11;
	temp.m12 = m.m21;
	temp.m20 = m.m02;
	temp.m21 = m.m12;
	temp.m22 = m.m22;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r.m[i][j] = temp.m[i][j];
		}
	}
}

void SexyMatrix3Inverse(const SexyMatrix3& m, SexyMatrix3& r)
{
	float aDet = (m.m22 * m.m11 - m.m21 * m.m12) * m.m00
	           - (m.m22 * m.m10 - m.m20 * m.m12) * m.m01
	           + (m.m21 * m.m10 - m.m20 * m.m11) * m.m02;
	float aInvDet = 1.0f / aDet;

	SexyMatrix3 temp;
	temp.m00 = (m.m22 * m.m11 - m.m21 * m.m12) * aInvDet;
	temp.m01 = (m.m02 * m.m21 - m.m22 * m.m01) * aInvDet;
	temp.m02 = (m.m12 * m.m01 - m.m02 * m.m11) * aInvDet;
	temp.m10 = (m.m20 * m.m12 - m.m22 * m.m10) * aInvDet;
	temp.m11 = (m.m00 * m.m22 - m.m02 * m.m20) * aInvDet;
	temp.m12 = (m.m02 * m.m10 - m.m12 * m.m00) * aInvDet;
	temp.m20 = (m.m21 * m.m10 - m.m20 * m.m11) * aInvDet;
	temp.m21 = (m.m20 * m.m01 - m.m21 * m.m00) * aInvDet;
	temp.m22 = (m.m00 * m.m11 - m.m10 * m.m01) * aInvDet;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r.m[i][j] = temp.m[i][j];
		}
	}
}

void SexyMatrix3Multiply(SexyMatrix3& m, const SexyMatrix3& l, const SexyMatrix3& r)
{
	SexyMatrix3 temp;
	temp.m00 = l.m00 * r.m00 + l.m01 * r.m10 + l.m02 * r.m20;
	temp.m01 = l.m00 * r.m01 + l.m01 * r.m11 + l.m02 * r.m21;
	temp.m02 = l.m00 * r.m02 + l.m01 * r.m12 + l.m02 * r.m22;
	temp.m10 = l.m10 * r.m00 + l.m11 * r.m10 + l.m12 * r.m20;
	temp.m11 = l.m10 * r.m01 + l.m11 * r.m11 + l.m12 * r.m21;
	temp.m12 = l.m10 * r.m02 + l.m11 * r.m12 + l.m12 * r.m22;
	temp.m20 = l.m20 * r.m00 + l.m21 * r.m10 + l.m22 * r.m20;
	temp.m21 = l.m20 * r.m01 + l.m21 * r.m11 + l.m22 * r.m21;
	temp.m22 = l.m20 * r.m02 + l.m21 * r.m12 + l.m22 * r.m22;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m.m[i][j] = temp.m[i][j];
		}
	}
}

// ===========================================================================
// Point-in-polygon test
// ===========================================================================

bool TodIsPointInPolygon(const SexyVector2* thePolygonPoint, int theNumberPolygonPoints, const SexyVector2& theCheckPoint)
{
	TOD_ASSERT(theNumberPolygonPoints >= 3);

	for (int i = 0; i < theNumberPolygonPoints; i++)
	{
		const SexyVector2& cur = thePolygonPoint[i];
		const SexyVector2& nex = thePolygonPoint[i == theNumberPolygonPoints - 1 ? 0 : i + 1];

		SexyVector2 u = (nex - cur).Perp();
		SexyVector2 v = theCheckPoint - cur;
		if (u.Dot(v) < 0)
			return false;
	}
	return true;
}

// ===========================================================================
// Application query stubs
// ===========================================================================

std::string TodGetCurrentLevelName()
{
	return "Unknown level";
}

bool TodHasUsedCheatKeys()
{
	return false;
}

bool TodAppCloseRequest()
{
	return false;
}

// ===========================================================================
// Color utilities
// ===========================================================================

Color GetFlashingColor(uint32_t theCounter, int theFlashTime)
{
	int aTimeAge = static_cast<int>(theCounter % static_cast<uint32_t>(theFlashTime));
	int aTimeInf = theFlashTime / 2;
	int aGrayness = ClampInt(200 * abs(aTimeInf - aTimeAge) / aTimeInf + 55, 0, 255);
	return Color(aGrayness, aGrayness, aGrayness, 255);
}

int ColorComponentMultiply(int theColor1, int theColor2)
{
	return ClampInt(theColor1 * theColor2 / 255, 0, 255);
}

Color ColorsMultiply(const Color& theColor1, const Color& theColor2)
{
	return Color(
		ColorComponentMultiply(theColor1.mRed, theColor2.mRed),
		ColorComponentMultiply(theColor1.mGreen, theColor2.mGreen),
		ColorComponentMultiply(theColor1.mBlue, theColor2.mBlue),
		ColorComponentMultiply(theColor1.mAlpha, theColor2.mAlpha)
	);
}

Color ColorAdd(const Color& theColor1, const Color& theColor2)
{
	int r = theColor1.mRed + theColor2.mRed;
	int g = theColor1.mGreen + theColor2.mGreen;
	int b = theColor1.mBlue + theColor2.mBlue;
	int a = theColor1.mAlpha + theColor2.mAlpha;

	return Color(ClampInt(r, 0, 255), ClampInt(g, 0, 255), ClampInt(b, 0, 255), ClampInt(a, 0, 255));
}

// ===========================================================================
// String utilities
// ===========================================================================

std::string TodReplaceString(const std::string& theText, const char* theStringToFind, const std::string& theStringToSubstitute)
{
	// C++03 / GCCE std::string stub: no find()/replace(), use C-style
	const char* p = strstr(theText.c_str(), theStringToFind);
	if (!p)
		return theText;

	int prefixLen = (int)(p - theText.c_str());
	int findLen = (int)strlen(theStringToFind);
	int remainLen = (int)strlen(p + findLen);

	char* buf = new char[prefixLen + (int)strlen(theStringToSubstitute.c_str()) + remainLen + 1];
	if (prefixLen > 0) memcpy(buf, theText.c_str(), prefixLen);
	memcpy(buf + prefixLen, theStringToSubstitute.c_str(), strlen(theStringToSubstitute.c_str()));
	if (remainLen > 0) memcpy(buf + prefixLen + strlen(theStringToSubstitute.c_str()), p + findLen, remainLen + 1);
	else buf[prefixLen + strlen(theStringToSubstitute.c_str())] = '\0';

	std::string result(buf);
	delete[] buf;
	return result;
}

std::string TodReplaceNumberString(const std::string& theText, const char* theStringToFind, int theNumber)
{
	// C++03 / GCCE std::string stub: no find()/replace(), use C-style
	const char* p = strstr(theText.c_str(), theStringToFind);
	if (!p)
		return theText;

	char aNumberStr[32];
	TodSnprintf(aNumberStr, sizeof(aNumberStr), "%d", theNumber);

	int prefixLen = (int)(p - theText.c_str());
	int findLen = (int)strlen(theStringToFind);
	int remainLen = (int)strlen(p + findLen);
	int numLen = (int)strlen(aNumberStr);

	char* buf = new char[prefixLen + numLen + remainLen + 1];
	if (prefixLen > 0) memcpy(buf, theText.c_str(), prefixLen);
	memcpy(buf + prefixLen, aNumberStr, numLen);
	if (remainLen > 0) memcpy(buf + prefixLen + numLen, p + findLen, remainLen + 1);
	else buf[prefixLen + numLen] = '\0';

	std::string result(buf);
	delete[] buf;
	return result;
}

// ===========================================================================
// Safe snprintf wrappers
// ===========================================================================

int TodVsnprintf(char* theBuffer, int theSize, const char* theFormat, va_list theArgList)
{
	if (!theBuffer || theSize <= 0)
		return 0;

	// vsnprintf not available on Symbian; use vsprintf + manual truncation
	int aCount = vsprintf(theBuffer, theFormat, theArgList);
	if (aCount > theSize)
	{
		theBuffer[theSize - 1] = '\0';
		aCount = theSize - 1;
	}
	if (aCount < 0 || aCount >= theSize)
	{
		theBuffer[theSize - 1] = '\0';
		aCount = theSize - 1;
	}
	return aCount;
}

int TodSnprintf(char* theBuffer, int theSize, const char* theFormat, ...)
{
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(theBuffer, theSize, theFormat, argList);
	va_end(argList);
	return aCount;
}

// ===========================================================================
// Image sanding stubs  (implement when GLImage/MemoryImage are available)
// ===========================================================================

void TodMarkImageForSanding(Image* /*theImage*/)
{
}

void TodSandImageIfNeeded(Image* /*theImage*/)
{
}

void FixPixelsOnAlphaEdgeForBlending(Image* /*theImage*/)
{
}

uint32_t AverageNearByPixels(MemoryImage* /*theImage*/, uint32_t* /*thePixel*/, int /*x*/, int /*y*/)
{
	return 0;
}

// ===========================================================================
// Drawing stubs  (implement when Graphics/Font are fully available)
// ===========================================================================

void TodDrawString(Graphics* /*g*/, const std::string& /*theText*/, int /*thePosX*/, int /*thePosY*/,
				   _Font* /*theFont*/, const Color& /*theColor*/, int /*theJustification*/)
{
}

void TodDrawImageScaledF(Graphics* /*g*/, Image* /*theImage*/, float /*thePosX*/, float /*thePosY*/,
						 float /*theScaleX*/, float /*theScaleY*/)
{
}

void TodDrawImageCenterScaledF(Graphics* /*g*/, Image* /*theImage*/, float /*thePosX*/, float /*thePosY*/,
							   float /*theScaleX*/, float /*theScaleY*/)
{
}

void TodDrawImageCelF(Graphics* /*g*/, Image* /*theImageStrip*/, float /*thePosX*/, float /*thePosY*/,
					  int /*theCelCol*/, int /*theCelRow*/)
{
}

void TodDrawImageCelScaled(Graphics* /*g*/, Image* /*theImageStrip*/, int /*thePosX*/, int /*thePosY*/,
						   int /*theCelCol*/, int /*theCelRow*/, float /*theScaleX*/, float /*theScaleY*/)
{
}

void TodDrawImageCelScaledF(Graphics* /*g*/, Image* /*theImageStrip*/, float /*thePosX*/, float /*thePosY*/,
							int /*theCelCol*/, int /*theCelRow*/, float /*theScaleX*/, float /*theScaleY*/)
{
}

void TodDrawImageCelCenterScaledF(Graphics* /*g*/, Image* /*theImageStrip*/, float /*thePosX*/, float /*thePosY*/,
								  int /*theCelCol*/, float /*theScaleX*/, float /*theScaleY*/)
{
}

void TodBltMatrix(Graphics* /*g*/, Image* /*theImage*/, const SexyMatrix3& /*theTransform*/,
				  const Rect& /*theClipRect*/, const Color& /*theColor*/, int /*theDrawMode*/,
				  const Rect& /*theSrcRect*/)
{
}
