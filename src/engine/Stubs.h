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

namespace Sexy { class Graphics; class Image; class Color; class Font; }

// [M4 #4 fix] Unify _Font with Sexy::Font. Previously _Font was an opaque
// forward-declared class, and FONT_* globals were declared as `extern _Font*`
// in Resources.h while Graphics::SetFont expected `Sexy::Font*`. This made
// all FONT_* globals unusable (type mismatch). Now _Font IS Sexy::Font, so
// GetFontThrow can return a real Font* that Graphics can use.
// The #ifndef guard allows SystemFont.h to also define the typedef without
// conflict (whichever is included first wins).
#ifndef _FONT_TYPEDEF_DEFINED
#define _FONT_TYPEDEF_DEFINED
typedef Sexy::Font _Font;
#endif

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
// [M4 #4 fix] Removed FONT_BRIANNETOD16 / FONT_PICO129 #define macros.
// They were `((_Font*)0)` which conflicted with the `extern _Font*`
// declarations in Resources.h (the macro turned the extern into a syntax
// error). Now that _Font = Sexy::Font, Resources.h's extern declarations
// are valid and Resources.cpp assigns real Font* pointers to them.

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
// [M3] The REAL resource dispatcher lives in Resources.cpp as
//   Sexy::ExtractResourcesByName(::ResourceManager*, const char*)
// It drives on-demand image loading (GetImageThrow -> GetImage -> LoadImageByResName).
// Previously a global STUB here returned true and loaded NOTHING, so LawnApp::LoadGroup()
// -- which includes only Stubs.h, not Resources.h -- bound to the stub and populated ZERO
// IMAGE_* globals (rmgr_log showed StartLoadResources but no LoadImageByResName per group).
// Forward-declare the real one in namespace Sexy so LawnApp resolves & links against it.
namespace Sexy { bool ExtractResourcesByName(::ResourceManager*, const char*); }
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
    extern Image* IMAGE_TREEOFWISDOM;
    extern Image* IMAGE_BACKGROUND1;
    extern Image* IMAGE_BACKGROUND2;
    extern Image* IMAGE_BACKGROUND3;
    extern Image* IMAGE_BACKGROUND4;
    extern Image* IMAGE_BACKGROUND5;
    extern Image* IMAGE_BACKGROUND6;
    extern Image* IMAGE_MUSHROOM_GARDEN;
    extern Image* IMAGE_GREENHOUSE;
    extern Image* IMAGE_ZOMBIQUARIUM;
    extern Image* IMAGE_FLAG;
    extern Image* IMAGE_ROOFDOOR_BOTTOM;
    extern Image* IMAGE_HOUSEDOOR_BOTTOM;
    extern Image* IMAGE_ROOFDOOR_TOP;
}

// Auto-generated IMAGE_* stubs
namespace Sexy {
    extern Image* IMAGE_ALMANAC;
    extern Image* IMAGE_AWARDPICKUPGLOW;
    extern Image* IMAGE_BLANK;
    extern Image* IMAGE_BRAIN;
    extern Image* IMAGE_BUGSPRAY;
    extern Image* IMAGE_CARKEYS;
    extern Image* IMAGE_CHOCOLATE;
    extern Image* IMAGE_CRATER;
    extern Image* IMAGE_CRATER_FADING;
    extern Image* IMAGE_CRATER_ROOF_CENTER;
    extern Image* IMAGE_CRATER_ROOF_LEFT;
    extern Image* IMAGE_CRATER_WATER_DAY;
    extern Image* IMAGE_CRATER_WATER_NIGHT;
    extern Image* IMAGE_FERTILIZER;
    extern Image* IMAGE_GLOVE;
    extern Image* IMAGE_MONEY_SIGN;
    extern Image* IMAGE_MONEYBAG_HI_RES;
    extern Image* IMAGE_PEA_SHADOWS;
    extern Image* IMAGE_PHONOGRAPH;
    extern Image* IMAGE_PLANTSHADOW;
    extern Image* IMAGE_PLANTSHADOW2;
    extern Image* IMAGE_PLANTSPEECHBUBBLE;
    extern Image* IMAGE_PRESENT;
    extern Image* IMAGE_PRESENTOPEN;
    extern Image* IMAGE_PROJECTILE_STAR;
    extern Image* IMAGE_PROJECTILECACTUS;
    extern Image* IMAGE_PROJECTILEPEA;
    extern Image* IMAGE_PROJECTILESNOWPEA;
    extern Image* IMAGE_PUFFSHROOM_PUFF1;
    extern Image* IMAGE_REANIM_CABBAGEPULT_CABBAGE;
    extern Image* IMAGE_REANIM_COBCANNON_COB;
    extern Image* IMAGE_REANIM_COIN_GOLD_DOLLAR;
    extern Image* IMAGE_REANIM_COIN_SILVER_DOLLAR;
    extern Image* IMAGE_REANIM_COINGLOW;
    extern Image* IMAGE_REANIM_CORNPULT_BUTTER;
    extern Image* IMAGE_REANIM_CORNPULT_KERNAL;
    extern Image* IMAGE_REANIM_DIAMOND;
    extern Image* IMAGE_REANIM_GARLIC_BODY3;
    extern Image* IMAGE_REANIM_MELONPULT_MELON;
    extern Image* IMAGE_REANIM_TALLNUT_CRACKED1;
    extern Image* IMAGE_REANIM_TALLNUT_CRACKED2;
    extern Image* IMAGE_REANIM_WALLNUT_BODY;
    extern Image* IMAGE_REANIM_WALLNUT_CRACKED1;
    extern Image* IMAGE_REANIM_WALLNUT_CRACKED2;
    extern Image* IMAGE_REANIM_WINTERMELON_PROJECTILE;
    extern Image* IMAGE_REANIM_ZOMBIE_BUCKET1;
    extern Image* IMAGE_REANIM_ZOMBIE_BUCKET2;
    extern Image* IMAGE_REANIM_ZOMBIE_BUCKET3;
    extern Image* IMAGE_REANIM_ZOMBIE_CATAPULT_BASKETBALL;
    extern Image* IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE;
    extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET;
    extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2;
    extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3;
    extern Image* IMAGE_REANIM_ZOMBIE_JACKBOX_BOX;
    extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1;
    extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1;
    extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2;
    extern Image* IMAGE_REANIM_ZOMBIE_LADDER_5;
    extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR1;
    extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR2;
    extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR3;
    extern Image* IMAGE_SCARY_POT;
    extern Image* IMAGE_SEEDBANK;
    extern Image* IMAGE_SEEDBANK_BLANK;
    extern Image* IMAGE_SEEDBANK_CONVEYORBELT;
    extern Image* IMAGE_SEEDBANK_ICE;
    extern Image* IMAGE_SEEDBANK_ZOMBIE;
    extern Image* IMAGE_SEEDS;
    extern Image* IMAGE_SHOVEL_HI_RES;
    extern Image* IMAGE_SLOT_MACHINE_BACKGROUND;
    extern Image* IMAGE_SUN;
    extern Image* IMAGE_SUNFLOWER_TROPHY;
    extern Image* IMAGE_TACO;
    extern Image* IMAGE_TOMBSTONES;
    extern Image* IMAGE_TREE_FOOD;
    extern Image* IMAGE_TROPHY;
    extern Image* IMAGE_TROPHY_HI_RES;
    extern Image* IMAGE_WATERINGCAN;
    extern Image* IMAGE_WHEELBARROW;
    extern Image* IMAGE_ZOMBIE_NOTE_SMALL;
    extern Image* IMAGE_ZOMBIEPOGO;
}

namespace Sexy { class SexyAppBase; }
extern Sexy::SexyAppBase* gSexyAppBase;