#ifndef __LAWNCOMMON_H__
#define __LAWNCOMMON_H__
//
// LawnCommon.h -- global helpers used throughout the Lawn module
// (Board/Plant/Zombie/Challenge/...) on Symbian. These are stub
// implementations to satisfy compilation; functional versions should
// replace them as resources and the reanimation system come online.
//
#include <e32def.h>
#include <e32math.h>
#include "../ConstEnums.h"

// ---------------------------------------------------------------------------
// Global RNG
// ---------------------------------------------------------------------------
inline unsigned int Rand()
{
    return static_cast<unsigned int>(Math::Random());
}

inline int Rand(int range)
{
    return (range > 0) ? (Math::Random() % range) : 0;
}

inline float RandRangeFloat(float theMin, float theMax)
{
    if (theMax <= theMin) return theMin;
    float r = static_cast<float>(Math::Random()) / static_cast<float>(KMaxTUint);
    return theMin + r * (theMax - theMin);
}

inline int RandRangeInt(int theMin, int theMax)
{
    if (theMax <= theMin) return theMin;
    return theMin + static_cast<int>(Math::Random()) % (theMax - theMin + 1);
}

// ---------------------------------------------------------------------------
// Reanimation / resource loading stubs
// ---------------------------------------------------------------------------
inline void ReanimatorEnsureDefinitionLoaded(int /*aReanimType*/, bool /*aDoLoad*/)
{
    // No-op on Symbian until the real reanimation system is wired up.
}

inline bool TodLoadResources(const char* /*aGroup*/)
{
    // Stub: always claim success.
    return true;
}

inline bool TodLoadResources(char* aGroup)
{
    return TodLoadResources(static_cast<const char*>(aGroup));
}

// ---------------------------------------------------------------------------
// TodStringFile helpers (localization) -- stubs
// ---------------------------------------------------------------------------
inline const char* TodStringTranslate(const char* theStr)
{
    return theStr;
}

// ---------------------------------------------------------------------------
// DrawSeedPacket -- forward declaration (defined in SeedPacket.cpp or equivalent)
// ---------------------------------------------------------------------------
void DrawSeedPacket(class Sexy::Graphics* g, float theX, float theY, int theSeedType, int theImitaterType, float theAlpha, int theGrayness, bool theIsSelected, bool theIsImitater);

// ---------------------------------------------------------------------------
// Misc helpers used by Board/Plant/Zombie
// ---------------------------------------------------------------------------
inline int FloatRoundToLong(float f)
{
    return static_cast<int>(f + (f >= 0 ? 0.5f : -0.5f));
}

#endif // __LAWNCOMMON_H__
