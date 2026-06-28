/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable to Symbian S60 3rd FP1 (GCCE, C++03).
 *
 * NOTE: All Zombie method implementations are now inline stubs in Zombie.h.
 * This file retains only the global/static data definitions.
 */

#include "Plant.h"
#include "Board.h"
#include "../ConstEnums.h"
#include "Zombie.h"
#include "Cutscene.h"
#include "GridItem.h"
#include "LawnMower.h"
#include "Challenge.h"
#include "Projectile.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "System/Music.h"
#include "Widget/AlmanacDialog.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/Attachment.h"
#include "LawnCommon.h"
#include "../Sexy.TodLib/TodParticle.h"
#include <limits.h>


ZombieDefinition gZombieDefs[NUM_ZOMBIE_TYPES] = {
    { ZOMBIE_NORMAL,            REANIM_ZOMBIE,              1,      1,      1,      4000,   "ZOMBIE" },
    { ZOMBIE_FLAG,              REANIM_ZOMBIE,              1,      1,      1,      0,      "FLAG_ZOMBIE" },
    { ZOMBIE_TRAFFIC_CONE,      REANIM_ZOMBIE,              2,      3,      1,      4000,   "CONEHEAD_ZOMBIE" },
    { ZOMBIE_POLEVAULTER,       REANIM_POLEVAULTER,         2,      6,      5,      2000,   "POLE_VAULTING_ZOMBIE" },
    { ZOMBIE_PAIL,              REANIM_ZOMBIE,              4,      8,      1,      3000,   "BUCKETHEAD_ZOMBIE" },
    { ZOMBIE_NEWSPAPER,         REANIM_ZOMBIE_NEWSPAPER,    2,      11,     1,      1000,   "NEWSPAPER_ZOMBIE" },
    { ZOMBIE_DOOR,              REANIM_ZOMBIE,              4,      13,     5,      3500,   "SCREEN_DOOR_ZOMBIE" },
    { ZOMBIE_FOOTBALL,          REANIM_ZOMBIE_FOOTBALL,     7,      16,     5,      2000,   "FOOTBALL_ZOMBIE" },
    { ZOMBIE_DANCER,            REANIM_DANCER,              5,      18,     5,      1000,   "DANCING_ZOMBIE" },
    { ZOMBIE_BACKUP_DANCER,     REANIM_BACKUP_DANCER,       1,      18,     1,      0,      "BACKUP_DANCER" },
    { ZOMBIE_DUCKY_TUBE,        REANIM_ZOMBIE,              1,      21,     5,      0,      "DUCKY_TUBE_ZOMBIE" },
    { ZOMBIE_SNORKEL,           REANIM_SNORKEL,             3,      23,     10,     2000,   "SNORKEL_ZOMBIE" },
    { ZOMBIE_ZAMBONI,           REANIM_ZOMBIE_ZAMBONI,      7,      26,     10,     2000,   "ZOMBONI" },
    { ZOMBIE_BOBSLED,           REANIM_BOBSLED,             3,      26,     10,     2000,   "ZOMBIE_BOBSLED_TEAM" },
    { ZOMBIE_DOLPHIN_RIDER,     REANIM_ZOMBIE_DOLPHINRIDER, 3,      28,     10,     1500,   "DOLPHIN_RIDER_ZOMBIE" },
    { ZOMBIE_JACK_IN_THE_BOX,   REANIM_JACKINTHEBOX,        3,      31,     10,     1000,   "JACK_IN_THE_BOX_ZOMBIE" },
    { ZOMBIE_BALLOON,           REANIM_BALLOON,             2,      33,     10,     2000,   "BALLOON_ZOMBIE" },
    { ZOMBIE_DIGGER,            REANIM_DIGGER,              4,      36,     10,     1000,   "DIGGER_ZOMBIE" },
    { ZOMBIE_POGO,              REANIM_POGO,                4,      38,     10,     1000,   "POGO_ZOMBIE" },
    { ZOMBIE_YETI,              REANIM_YETI,                4,      40,     1,      1,      "ZOMBIE_YETI" },
    { ZOMBIE_BUNGEE,            REANIM_BUNGEE,              3,      41,     10,     1000,   "BUNGEE_ZOMBIE" },
    { ZOMBIE_LADDER,            REANIM_LADDER,              4,      43,     10,     1000,   "LADDER_ZOMBIE" },
    { ZOMBIE_CATAPULT,          REANIM_CATAPULT,            5,      46,     10,     1500,   "CATAPULT_ZOMBIE" },
    { ZOMBIE_GARGANTUAR,        REANIM_GARGANTUAR,          10,     48,     15,     1500,   "GARGANTUAR" },
    { ZOMBIE_IMP,               REANIM_IMP,                 10,     48,     1,      0,      "IMP" },
    { ZOMBIE_BOSS,              REANIM_BOSS,                10,     50,     1,      0,      "BOSS" },
    { ZOMBIE_PEA_HEAD,          REANIM_ZOMBIE,              1,      99,     1,      4000,   "ZOMBIE" },
    { ZOMBIE_WALLNUT_HEAD,      REANIM_ZOMBIE,              4,      99,     1,      3000,   "ZOMBIE" },
    { ZOMBIE_JALAPENO_HEAD,     REANIM_ZOMBIE,              3,      99,     10,     1000,   "ZOMBIE" },
    { ZOMBIE_GATLING_HEAD,      REANIM_ZOMBIE,              3,      99,     10,     2000,   "ZOMBIE" },
    { ZOMBIE_SQUASH_HEAD,       REANIM_ZOMBIE,              3,      99,     10,     2000,   "ZOMBIE" },
    { ZOMBIE_TALLNUT_HEAD,      REANIM_ZOMBIE,              4,      99,     10,     2000,   "ZOMBIE" },
    { ZOMBIE_REDEYE_GARGANTUAR, REANIM_GARGANTUAR,          10,     48,     15,     6000,   "REDEYED_GARGANTUAR" },
};

static ZombieType gBossZombieList[] = {
    ZOMBIE_TRAFFIC_CONE,
    ZOMBIE_PAIL,
    ZOMBIE_FOOTBALL,
    ZOMBIE_POLEVAULTER,
    ZOMBIE_JACK_IN_THE_BOX,
    ZOMBIE_LADDER,
    ZOMBIE_ZAMBONI,
    ZOMBIE_CATAPULT,
    ZOMBIE_POGO,
    ZOMBIE_NEWSPAPER,
    ZOMBIE_DOOR,
    ZOMBIE_GARGANTUAR
};
