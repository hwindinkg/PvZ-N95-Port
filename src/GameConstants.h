/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __GAMECONSTANTS_H__
#define __GAMECONSTANTS_H__

#include "ConstEnums.h"

const double PI = 3.141592653589793;

// ============================================================
// Constants
// ============================================================
const int BOARD_WIDTH = 800;
const int BOARD_HEIGHT = 600;
const int WIDE_BOARD_WIDTH = 800;
const int BOARD_OFFSET = 220;
const int BOARD_EDGE = -100;
const int BOARD_IMAGE_WIDTH_OFFSET = 1180;
const int BOARD_ICE_START = 800;
const int LAWN_XMIN = 40;
const int LAWN_YMIN = 80;
const int HIGH_GROUND_HEIGHT = 30;

const int SEEDBANK_MAX = 10;
const int SEED_BANK_OFFSET_X = 0;
const int SEED_BANK_OFFSET_X_END = 10;
const int SEED_CHOOSER_OFFSET_Y = 516;
const int SEED_PACKET_WIDTH = 50;
const int SEED_PACKET_HEIGHT = 70;
const int IMITATER_DIALOG_WIDTH = 500;
const int IMITATER_DIALOG_HEIGHT = 600;

const int MAX_ACHIEVEMENTS = 50;
const int MAX_CHALLENGE_RECORDS = 200;

// ============================================================
// About levels
// ============================================================
const int ADVENTURE_AREAS = 5;
const int LEVELS_PER_AREA = 10;
const int NUM_LEVELS = ADVENTURE_AREAS * LEVELS_PER_AREA;
const int FINAL_LEVEL = NUM_LEVELS;
const int FLAG_RAISE_TIME = 100;
const int LAST_STAND_FLAGS = 5;
const int ZOMBIE_COUNTDOWN_FIRST_WAVE = 1800;
const int ZOMBIE_COUNTDOWN = 2500;
const int ZOMBIE_COUNTDOWN_RANGE = 600;
const int ZOMBIE_COUNTDOWN_BEFORE_FLAG = 4500;
const int ZOMBIE_COUNTDOWN_BEFORE_REPICK = 5499;
const int ZOMBIE_COUNTDOWN_MIN = 400;
const int FOG_BLOW_RETURN_TIME = 2000;
const int SUN_COUNTDOWN = 425;
const int SUN_COUNTDOWN_RANGE = 275;
const int SUN_COUNTDOWN_MAX = 950;
const int SURVIVAL_NORMAL_FLAGS = 5;
const int SURVIVAL_HARD_FLAGS = 10;

// ============================================================
// About the store screen layout
// ============================================================
const int STORESCREEN_ITEMOFFSET_1_X = 422;
const int STORESCREEN_ITEMOFFSET_1_Y = 206;
const int STORESCREEN_ITEMOFFSET_2_X = 372;
const int STORESCREEN_ITEMOFFSET_2_Y = 310;
const int STORESCREEN_ITEMSIZE = 74;
const int STORESCREEN_COINBANK_X = 650;
const int STORESCREEN_COINBANK_Y = 559;
const int STORESCREEN_PAGESTRING_X = 470;
const int STORESCREEN_PAGESTRING_Y = 500;

// ---------------------------------------------------------------------------
// Misc constants used by board.cpp
// ---------------------------------------------------------------------------
const int ZOMBIE_CUTSCENE = -2;
const int ZOMBIE_WAVE_DEBUG = -1;
const int RENDER_LAYER_BACKDROP = 200000;
const int HugeWaveCountDown = -1;
inline int MowingInvokedByMouseUp(int) { return -1; }
inline int RankRangeInt(int min = 0, int max = 0, int val = 0) { (void)min; (void)max; (void)val; return 0; }

// Undeclared identifiers from board.cpp
extern bool mChooseShapeSwapChallenged;
void UpdateSeedBankMouseOver();
const int DirtyAll = 0;

// SeedNotRecommend type and NOT_RECOMMENDED_* constants
const int NOT_RECOMMENDED_NONE = 0;
const int NOT_RECOMMENDED_WATER = 1;
const int NOT_RECOMMENDED_HIGHGROUND = 2;
const int NOT_RECOMMENDED_FOG = 3;
struct SeedNotRecommend { int mNotRecommend; SeedNotRecommend() : mNotRecommend(NOT_RECOMMENDED_NONE) {} SeedNotRecommend(int i) : mNotRecommend(i) {} };


// Font constants (stub)
const int FONT_DWARVEN = 1;
const int FONT_COUNTER = 2;

// Purchase offset constant
const int PURCHASE_COUNT_OFFSET = 1000;

// GetSeedDefinition stub
struct SeedDefinition { int mSunCost; };
inline SeedDefinition& GetSeedDefinition(int seedType) { (void)seedType; static SeedDefinition sd; sd.mSunCost = 50; return sd; }

// ---------------------------------------------------------------------------
// RENDER_GROUP_* constants for reanimation rendering
// ---------------------------------------------------------------------------
const int RENDER_GROUP_HIDDEN = -1;
const int RENDER_GROUP_NORMAL = 0;

// ---------------------------------------------------------------------------
// FilterEffect constants (use int to avoid conflict with FilterEffect.h struct)
// ---------------------------------------------------------------------------
#define FILTER_EFFECT_NONE           0
#define FILTER_EFFECT_WASHED_OUT     1
#define FILTER_EFFECT_LESS_WASHED_OUT 2

// ---------------------------------------------------------------------------
// FACING constants
// ---------------------------------------------------------------------------
#define FACING_LEFT  0
#define FACING_RIGHT 1

#endif // __GAMECONSTANTS_H__
