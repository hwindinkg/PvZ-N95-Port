/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 *
 * NOTE: All Challenge method implementations are now inline stubs in Challenge.h.
 * This file retains only the global data definitions.
 */

#include "Coin.h"
#include "Board.h"
#include "Plant.h"
#include "Zombie.h"
#include "GridItem.h"
#include "Cutscene.h"
#include "ZenGarden.h"
#include "LawnMower.h"
#include "Challenge.h"
#include "SeedPacket.h"
#include "Projectile.h"
#include "../LawnApp.h"
#include "CursorObject.h"
#include "System/Music.h"
#include "ToolTipWidget.h"
#include "MessageWidget.h"
#include "../GameConstants.h"
#include "System/PlayerInfo.h"
#include "Widget/GameButton.h"
#include "Widget/StoreScreen.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "../engine/Font.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../engine/MTRand.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "../engine/WidgetManager.h"
#include "Widget/AchievementsScreen.h"
#include "../engine/SexyAppBase.h"
#include "LawnCommon.h"
#include "../Resources.h"
#include "System/ReportAchievement.h"
#include "SeedBank.h"

using namespace Sexy;

int gZombieWaves[NUM_LEVELS] = {
	4,  6,  8,  10, 8,  10, 20, 10, 20, 20,
	10, 20, 10, 20, 10, 10, 20, 10, 20, 20,
	10, 20, 20, 30, 20, 20, 30, 20, 30, 30,
	10, 20, 10, 20, 20, 10, 20, 10, 20, 20,
	10, 20, 20, 30, 20, 20, 30, 20, 30, 30,
};

ZombieAllowedLevels gZombieAllowedLevels[NUM_ZOMBIE_TYPES] = {
	{ ZOMBIE_NORMAL,
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_FLAG,
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_TRAFFIC_CONE,
		{
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_POLEVAULTER,
		{
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_PAIL,
		{
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 1, 0, 0, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 1, 0, 1, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 1, 1,
		}
	},
	{ ZOMBIE_NEWSPAPER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DOOR,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_FOOTBALL,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DANCER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_BACKUP_DANCER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	},
	{ ZOMBIE_DUCKY_TUBE, { 0 } },
	{ ZOMBIE_SNORKEL,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 0, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_ZAMBONI,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_BOBSLED,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DOLPHIN_RIDER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_JACK_IN_THE_BOX,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 0, 0, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
		}
	},
	{ ZOMBIE_BALLOON,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DIGGER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_POGO,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_YETI, {0} },
	{ ZOMBIE_BUNGEE,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_LADDER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 0, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_CATAPULT,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_GARGANTUAR,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
		}
	},
	{ ZOMBIE_IMP,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
		}
	},
	{ ZOMBIE_BOSS, {0} },
	{ ZOMBIE_PEA_HEAD, {0} },
	{ ZOMBIE_WALLNUT_HEAD, {0} },
	{ ZOMBIE_JALAPENO_HEAD, {0} },
	{ ZOMBIE_GATLING_HEAD, {0} },
	{ ZOMBIE_SQUASH_HEAD, {0} },
	{ ZOMBIE_TALLNUT_HEAD, {0} },
	{ ZOMBIE_REDEYE_GARGANTUAR, {0} },
};

SeedType gArtChallengeWallnut[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};

SeedType gArtChallengeSunFlower[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_STARFRUIT, SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_UMBRELLA,  SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_UMBRELLA,  SEED_UMBRELLA,  SEED_UMBRELLA,  SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};

SeedType gArtChallengeStarFruit[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};
