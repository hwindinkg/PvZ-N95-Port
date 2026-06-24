#ifndef __ENGINE_STUBS_H__
#define __ENGINE_STUBS_H__

#include <string.h>
#include "Common.h"
#include "Rect.h"
#include "ConstEnums.h"
#include "Widget.h"
#include <stdarg.h>
#include <e32std.h>

// ============================================================
// Forward declarations for types used in stub functions
// ============================================================
namespace Sexy { class LawnApp; }
class Reanimation;
class TodParticleSystem;

namespace Sexy { class Graphics; class Image; class Color; }
class _Font;

// ============================================================
// Stub constants
// ============================================================
const int IMG_DOWNSCALE = 1;
#define IMAGE_OPTIONS_MENUBACK       ((Sexy::Image*)0)
#define IMAGE_MONEYBAG               ((Sexy::Image*)0)
#define IMAGE_SHOVEL                ((Sexy::Image*)0)
#define IMAGE_REANIM_CRAZYDAVE_MOUTH1 ((Sexy::Image*)0)
#define IMAGE_REANIM_CRAZYDAVE_MOUTH4 ((Sexy::Image*)0)
#define IMAGE_REANIM_CRAZYDAVE_MOUTH5 ((Sexy::Image*)0)
#define IMAGE_REANIM_CRAZYDAVE_MOUTH6 ((Sexy::Image*)0)
#define IMAGE_STORE_SPEECHBUBBLE     ((Sexy::Image*)0)
#define IMAGE_STORE_SPEECHBUBBLE2    ((Sexy::Image*)0)
#define FONT_BRIANNETOD16            ((_Font*)0)
#define FONT_PICO129                 ((_Font*)0)

// These are #define macros (not real variables) to prevent duplicate definitions
// Real definitions are in Resources_stub.cpp
#define IMAGE_ZENSHOPBUTTON              ((Sexy::Image*)0)
#define IMAGE_ZENSHOPBUTTON_HIGHLIGHT    ((Sexy::Image*)0)
#define IMAGE_SHOVELBANK                 ((Sexy::Image*)0)

const int FOLEY_CRAZY_DAVE_SHORT = 100;
const int FOLEY_CRAZY_DAVE_LONG = 101;
const int FOLEY_CRAZY_DAVE_EXTRA_LONG = 102;
const int FOLEY_CRAZY_DAVE_CRAZY = 103;
const int FOLEY_CRAZY_DAVE_SCREAM = 104;
const int FOLEY_CRAZY_DAVE_SCREAM_2 = 105;

extern void* gLawnFoleyParamArray[1];
extern void* gLawnTrailArray[1];
extern void* gLawnParticleArray[1];
extern void* gLawnReanimationArray[1];

// Common math
template<typename T> inline T ClampInt(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// SafeDeleteWidget -- template to handle any Widget subclass pointer
template<typename T> inline void SafeDeleteWidget(T*& w) { delete w; w = NULL; }
template<typename T> inline void SafeDeleteWidgetT(T*& w) { delete w; w = NULL; }
inline void ProcessSafeDeleteList() {}
inline void FreeGlobalAllocators() {}

// File I/O (GetSavedGameName is in SaveGame.h)
inline void EraseFile(const std::string&) {}
inline bool FileExists(const std::string&) { return false; }
inline std::string GetLegacySavedGameName(int, int) { return std::string(); }

// Resource loading
inline void Tod_SWTri_AddAllDrawTriFuncs() {}
inline bool TodLoadResources(const char*) { return true; }
inline bool TodLoadResources(const char*, bool) { (void)0; return true; } // 2-arg overload
inline bool TodLoadNextResource() { return false; }
inline void TodStringListLoad(const char*) {}
inline void LoadProperties(const char*, bool = false) { (void)0; }
inline void LoadProperties(const char*, bool, bool) { (void)0; } // 3-arg overload
class ResourceManager;
inline bool ExtractResourcesByName(const char*, const char*) { return false; }
inline bool ExtractResourcesByName(class ResourceManager*, const char*) { return true; }
inline void TodFoleyInitialize(const int*, int, const int*, int, const int*, int) {}
inline void TodFoleyInitialize(void**, int) {} // overload for void* arrays
inline void TrailLoadDefinitions(const int*) {}
inline void TrailLoadDefinitions(void**, int) {} // overload for void* arrays
inline void TodParticleLoadDefinitions(const int*) {}
inline void TodParticleLoadDefinitions(void**, int) {} // overload for void* arrays
inline void ReanimatorLoadDefinitions(const int*) {}
inline void ReanimatorLoadDefinitions(void**, int) {} // 2-arg overload for gLawnReanimationArray
inline void ReanimatorEnsureDefinitionLoaded(int, int) {}
inline void FilterEffectDisposeForApp() {}
inline void TodParticleFreeDefinitions() {}
inline void ReanimatorFreeDefinitions() {}
inline void TrailFreeDefinitions() {}

// Rendering
inline void DrawDirtyStuff() {}
inline void CopyToClipboard(const std::string&) {}

// Achievements (stub)
#include "../Lawn/System/ReportAchievement.h"
inline void ReportAchievement(int) {}
inline void GiveAchievement(int) {}
inline void GiveAchievement(int, int, bool) { (void)0; } // 3-arg overload

// String translation
inline std::string TodStringTranslate(const std::string& s) { return s; }
inline bool TodStringListExists(const std::string&) { return false; }
inline std::string TodReplaceNumberString(const std::string& s, const char*, int) { return s; }

// Dialog management — overloaded for both patterns used in LawnApp.cpp
inline void AddDialog(Sexy::Widget* w) { (void)w; }
inline void AddDialog(int theId, Sexy::Widget* w) { (void)theId; (void)w; }

// Updates
inline void CheckForUpdates() {}

// Draw text helpers
inline void TodDrawStringWrapped(Sexy::Graphics*, const std::string&, const Sexy::Rect&, _Font*, const Sexy::Color&, int) {}

// Challenge definitions
struct ChallengeDefinition { int mType; const char* mChallengeName; int mPage; GameMode mChallengeMode; };
inline ChallengeDefinition& GetChallengeDefinition(int) { static ChallengeDefinition d; d.mType = 0; d.mChallengeName = ""; d.mPage = 0; d.mChallengeMode = GameMode(0); return d; }

const int NUM_CHALLENGE_MODES = 100;

// String operators for Symbian STL stubs
// (operator== is defined inside std::string stub; no need here)

// PerfTimer
class PerfTimer
{
public:
    void Start() {}
    int GetDuration() { return 0; }
};

// UI updates
inline void UpdateSeedBankMouseOver() {}

#endif // __ENGINE_STUBS_H__

// Missing IMAGE_* stubs for linker
namespace Sexy {
    Image* IMAGE_TREEOFWISDOM = NULL;
    Image* IMAGE_BACKGROUND1 = NULL;
    Image* IMAGE_BACKGROUND2 = NULL;
    Image* IMAGE_BACKGROUND3 = NULL;
    Image* IMAGE_BACKGROUND4 = NULL;
    Image* IMAGE_BACKGROUND5 = NULL;
    Image* IMAGE_BACKGROUND6 = NULL;
    Image* IMAGE_MUSHROOM_GARDEN = NULL;
    Image* IMAGE_GREENHOUSE = NULL;
    Image* IMAGE_ZOMBIQUARIUM = NULL;
    Image* IMAGE_FLAG = NULL;
    Image* IMAGE_ROOFDOOR_BOTTOM = NULL;
    Image* IMAGE_HOUSEDOOR_BOTTOM = NULL;
    Image* IMAGE_ROOFDOOR_TOP = NULL;
}

// Auto-generated IMAGE_* stubs
namespace Sexy {
    Image* IMAGE_ALMANAC = NULL;
    Image* IMAGE_AWARDPICKUPGLOW = NULL;
    Image* IMAGE_BLANK = NULL;
    Image* IMAGE_BRAIN = NULL;
    Image* IMAGE_BUGSPRAY = NULL;
    Image* IMAGE_CARKEYS = NULL;
    Image* IMAGE_CHOCOLATE = NULL;
    Image* IMAGE_CRATER = NULL;
    Image* IMAGE_CRATER_FADING = NULL;
    Image* IMAGE_CRATER_ROOF_CENTER = NULL;
    Image* IMAGE_CRATER_ROOF_LEFT = NULL;
    Image* IMAGE_CRATER_WATER_DAY = NULL;
    Image* IMAGE_CRATER_WATER_NIGHT = NULL;
    Image* IMAGE_FERTILIZER = NULL;
    Image* IMAGE_GLOVE = NULL;
    Image* IMAGE_MONEY_SIGN = NULL;
    Image* IMAGE_MONEYBAG_HI_RES = NULL;
    Image* IMAGE_PEA_SHADOWS = NULL;
    Image* IMAGE_PHONOGRAPH = NULL;
    Image* IMAGE_PLANTSHADOW = NULL;
    Image* IMAGE_PLANTSHADOW2 = NULL;
    Image* IMAGE_PLANTSPEECHBUBBLE = NULL;
    Image* IMAGE_PRESENT = NULL;
    Image* IMAGE_PRESENTOPEN = NULL;
    Image* IMAGE_PROJECTILE_STAR = NULL;
    Image* IMAGE_PROJECTILECACTUS = NULL;
    Image* IMAGE_PROJECTILEPEA = NULL;
    Image* IMAGE_PROJECTILESNOWPEA = NULL;
    Image* IMAGE_PUFFSHROOM_PUFF1 = NULL;
    Image* IMAGE_REANIM_CABBAGEPULT_CABBAGE = NULL;
    Image* IMAGE_REANIM_COBCANNON_COB = NULL;
    Image* IMAGE_REANIM_COIN_GOLD_DOLLAR = NULL;
    Image* IMAGE_REANIM_COIN_SILVER_DOLLAR = NULL;
    Image* IMAGE_REANIM_COINGLOW = NULL;
    Image* IMAGE_REANIM_CORNPULT_BUTTER = NULL;
    Image* IMAGE_REANIM_CORNPULT_KERNAL = NULL;
    Image* IMAGE_REANIM_DIAMOND = NULL;
    Image* IMAGE_REANIM_GARLIC_BODY3 = NULL;
    Image* IMAGE_REANIM_MELONPULT_MELON = NULL;
    Image* IMAGE_REANIM_TALLNUT_CRACKED1 = NULL;
    Image* IMAGE_REANIM_TALLNUT_CRACKED2 = NULL;
    Image* IMAGE_REANIM_WALLNUT_BODY = NULL;
    Image* IMAGE_REANIM_WALLNUT_CRACKED1 = NULL;
    Image* IMAGE_REANIM_WALLNUT_CRACKED2 = NULL;
    Image* IMAGE_REANIM_WINTERMELON_PROJECTILE = NULL;
    Image* IMAGE_REANIM_ZOMBIE_BUCKET1 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_BUCKET2 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_BUCKET3 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_CATAPULT_BASKETBALL = NULL;
    Image* IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE = NULL;
    Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET = NULL;
    Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_JACKBOX_BOX = NULL;
    Image* IMAGE_REANIM_ZOMBIE_LADDER_1 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_LADDER_5 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR1 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR2 = NULL;
    Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR3 = NULL;
    Image* IMAGE_SCARY_POT = NULL;
    Image* IMAGE_SEEDBANK = NULL;
    Image* IMAGE_SEEDBANK_BLANK = NULL;
    Image* IMAGE_SEEDBANK_CONVEYORBELT = NULL;
    Image* IMAGE_SEEDBANK_ICE = NULL;
    Image* IMAGE_SEEDBANK_ZOMBIE = NULL;
    Image* IMAGE_SEEDS = NULL;
    Image* IMAGE_SHOVEL_HI_RES = NULL;
    Image* IMAGE_SLOT_MACHINE_BACKGROUND = NULL;
    Image* IMAGE_SUN = NULL;
    Image* IMAGE_SUNFLOWER_TROPHY = NULL;
    Image* IMAGE_TACO = NULL;
    Image* IMAGE_TOMBSTONES = NULL;
    Image* IMAGE_TREE_FOOD = NULL;
    Image* IMAGE_TROPHY = NULL;
    Image* IMAGE_TROPHY_HI_RES = NULL;
    Image* IMAGE_WATERINGCAN = NULL;
    Image* IMAGE_WHEELBARROW = NULL;
    Image* IMAGE_ZOMBIE_NOTE_SMALL = NULL;
    Image* IMAGE_ZOMBIEPOGO = NULL;
}

namespace Sexy { class SexyAppBase; }
extern Sexy::SexyAppBase* gSexyAppBase;
