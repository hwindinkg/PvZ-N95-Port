/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#include "Coin.h"
#include "Plant.h"
#include "Board.h"
#include "Zombie.h"
#include "Cutscene.h"
#include "GridItem.h"
#include "ZenGarden.h"
#include "Challenge.h"
#include "Projectile.h"
#include "SeedPacket.h"
#include "LawnApp.h"
#include "CursorObject.h"
#include "Resources.h"
#include "GameConstants.h"
#include "LawnCommon.h"
#include "System/PlayerInfo.h"
#include "System/ReportAchievement.h"
#include "System/ReanimationLawn.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Attachment.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/EffectSystem.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "Widget/AchievementsScreen.h"

// Achievement IDs used by plant.cpp
#ifndef RollSomeHeads
#define RollSomeHeads 0
#endif
#ifndef Explodonator
#define Explodonator 1
#endif
#ifndef Spudow
#define Spudow 2
#endif

// std::max replacement for Symbian (no <algorithm>)
namespace std {
    template<typename T> inline const T& max(const T& a, const T& b) { return (a > b) ? a : b; }
    template<typename T> inline const T& min(const T& a, const T& b) { return (a < b) ? a : b; }
}

extern LawnApp* gLawnApp;

PlantDefinition gPlantDefs[NUM_SEED_TYPES] = {
    { SEED_PEASHOOTER,        NULL, REANIM_PEASHOOTER,     0,  100,    750,    SUBCLASS_SHOOTER,   150,    "PEASHOOTER" },
    { SEED_SUNFLOWER,         NULL, REANIM_SUNFLOWER,      1,  50,     750,    SUBCLASS_NORMAL,    2500,   "SUNFLOWER" },
    { SEED_CHERRYBOMB,        NULL, REANIM_CHERRYBOMB,     3,  150,    5000,   SUBCLASS_NORMAL,    0,      "CHERRY_BOMB" },
    { SEED_WALLNUT,           NULL, REANIM_WALLNUT,        2,  50,     3000,   SUBCLASS_NORMAL,    0,      "WALL_NUT" },
    { SEED_POTATOE_MINE,      NULL, REANIM_NONE,           37, 25,     3000,   SUBCLASS_NORMAL,    0,      "POTATO_MINE" },
    { SEED_SNOW_PEA,          NULL, REANIM_NONE,           4,  175,    750,    SUBCLASS_SHOOTER,   150,    "SNOW_PEA" },
    { SEED_CHOMPER,           NULL, REANIM_CHOMPER,        31, 150,    750,    SUBCLASS_NORMAL,    0,      "CHOMPER" },
    { SEED_REPEATER,          NULL, REANIM_NONE,           5,  200,    750,    SUBCLASS_SHOOTER,   150,    "REPEATER" },
    { SEED_GARLIC,            NULL, REANIM_NONE,           8,  50,     750,    SUBCLASS_NORMAL,    0,      "GARLIC" },
    { SEED_MARIGOLD,          NULL, REANIM_MARIGOLD,       24, 50,     3000,   SUBCLASS_NORMAL,    2500,   "MARIGOLD" },
    { SEED_SUNSHROOM,         NULL, REANIM_NONE,           7,  25,     750,    SUBCLASS_NORMAL,    2500,   "SUN_SHROOM" },
    { SEED_FUMESHROOM,        NULL, REANIM_NONE,           9,  75,     750,    SUBCLASS_SHOOTER,   150,    "FUME_SHROOM" },
    { SEED_GRAVE_BUSTER,      NULL, REANIM_NONE,           40, 75,     750,    SUBCLASS_NORMAL,    0,      "GRAVE_BUSTER" },
    { SEED_HYPNOSHROOM,       NULL, REANIM_NONE,           10, 75,     3000,   SUBCLASS_NORMAL,    0,      "HYPNO_SHROOM" },
    { SEED_SCAREDY_SHROOM,    NULL, REANIM_NONE,           33, 25,     750,    SUBCLASS_SHOOTER,   150,    "SCAREDY_SHROOM" },
    { SEED_ICE_SHROOM,        NULL, REANIM_NONE,           36, 75,     5000,   SUBCLASS_NORMAL,    0,      "ICE_SHROOM" },
    { SEED_DOOM_SHROOM,       NULL, REANIM_NONE,           20, 125,    5000,   SUBCLASS_NORMAL,    0,      "DOOM_SHROOM" },
    { SEED_LILY_PAD,          NULL, REANIM_NONE,           19, 25,     750,    SUBCLASS_NORMAL,    0,      "LILY_PAD" },
    { SEED_SQUASH,            NULL, REANIM_NONE,           21, 50,     3000,   SUBCLASS_NORMAL,    0,      "SQUASH" },
    { SEED_THREEPEATER,       NULL, REANIM_NONE,           12, 325,    750,    SUBCLASS_SHOOTER,   150,    "THREEPEATER" },
    { SEED_TANGLE_KELP,       NULL, REANIM_NONE,           17, 25,     3000,   SUBCLASS_NORMAL,    0,      "TANGLE_KELP" },
    { SEED_STARFRUIT,         NULL, REANIM_NONE,           30, 125,    750,    SUBCLASS_SHOOTER,   150,    "STARFRUIT" },
    { SEED_PUMPKIN,           NULL, REANIM_NONE,           25, 125,    3000,   SUBCLASS_NORMAL,    0,      "PUMPKIN" },
    { SEED_TORCHWOOD,         NULL, REANIM_NONE,           29, 175,    750,    SUBCLASS_NORMAL,    0,      "TORCHWOOD" },
    { SEED_CACTUS,            NULL, REANIM_NONE,           15, 125,    750,    SUBCLASS_SHOOTER,   150,    "CACTUS" },
    { SEED_BLOVER,            NULL, REANIM_NONE,           18, 100,    750,    SUBCLASS_NORMAL,    0,      "BLOVER" },
    { SEED_SPLIT_PEA,         NULL, REANIM_NONE,           32, 125,    750,    SUBCLASS_SHOOTER,   150,    "SPLIT_PEA" },
    { SEED_CABBAGE_PULT,      NULL, REANIM_NONE,           13, 100,    750,    SUBCLASS_SHOOTER,   300,    "CABBAGE_PULT" },
    { SEED_KERNEL_PULT,       NULL, REANIM_NONE,           13, 100,    750,    SUBCLASS_SHOOTER,   300,    "KERNEL_PULT" },
    { SEED_MELON_PULT,        NULL, REANIM_NONE,           14, 300,    750,    SUBCLASS_SHOOTER,   300,    "MELON_PULT" },
    { SEED_COB_CANNON,        NULL, REANIM_NONE,           16, 500,    5000,   SUBCLASS_NORMAL,    600,    "COB_CANNON" },
    { SEED_GARLIC2,           NULL, REANIM_NONE,           8,  50,     750,    SUBCLASS_NORMAL,    0,      "GARLIC2" },
    { SEED_SPIKEWEED,         NULL, REANIM_NONE,           22, 100,    750,    SUBCLASS_NORMAL,    0,      "SPIKEWEED" },
    { SEED_SPIKEROCK,         NULL, REANIM_NONE,           27, 125,    5000,   SUBCLASS_NORMAL,    0,      "SPIKEROCK" },
    { SEED_UPGRADE_REPEATER,  NULL, REANIM_NONE,           5,  250,    5000,   SUBCLASS_SHOOTER,   150,    "GATLING_PEA" },
    { SEED_IMITATER,          NULL, REANIM_NONE,           33, 0,      750,    SUBCLASS_NORMAL,    0,      "IMITATER" },
    { SEED_EXPLODE_O_NUT,     NULL, REANIM_WALLNUT,        2,  0,      3000,   SUBCLASS_NORMAL,    0,      "EXPLODE_O_NUT" },
    { SEED_GIANT_WALLNUT,     NULL, REANIM_WALLNUT,        2,  0,      3000,   SUBCLASS_NORMAL,    0,      "GIANT_WALLNUT" },
    { SEED_SPROUT,            NULL, REANIM_NONE,           33, 0,      3000,   SUBCLASS_NORMAL,    0,      "SPROUT" },
    { SEED_LEFTPEATER,        NULL, REANIM_NONE,           5,  200,    750,    SUBCLASS_SHOOTER,   150,    "REPEATER" }
};

Plant::Plant()
{
}

void Plant::PlantInitialize(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
    mPlantCol = theGridX;
    mRow = theGridY;
    if (mBoard)
    {
        mX = mBoard->GridToPixelX(theGridX, theGridY);
        mY = mBoard->GridToPixelY(theGridX, theGridY);
    }
    mAnimCounter = 0;
    mAnimPing = true;
    mFrame = 0;
    mShootingCounter = 0;
    mShakeOffsetX = 0.0f;
    mShakeOffsetY = 0.0f;
    mFrameLength = RandRangeInt(12, 18);
    mTargetX = -1;
    mTargetY = -1;
    mStartRow = mRow;
    mNumFrames = 5;
    mState = STATE_NOTREADY;
    mDead = false;
    mSquished = false;
    mSeedType = theSeedType;
    mImitaterType = theImitaterType;
    mPlantHealth = 300;
    mDoSpecialCountdown = 0;
    mDisappearCountdown = 200;
    mStateCountdown = 0;
    mParticleID = PARTICLESYSTEMID_NULL;
    mBodyReanimID = REANIMATIONID_NULL;
    mHeadReanimID = REANIMATIONID_NULL;
    mHeadReanimID2 = REANIMATIONID_NULL;
    mHeadReanimID3 = REANIMATIONID_NULL;
    mBlinkReanimID = REANIMATIONID_NULL;
    mLightReanimID = REANIMATIONID_NULL;
    mSleepingReanimID = REANIMATIONID_NULL;
    mBlinkCountdown = 0;
    mRecentlyEatenCountdown = 0;
    mEatenFlashCountdown = 0;
    mBeghouledFlashCountdown = 0;
    mWidth = 80;
    mHeight = 80;
    memset(mMagnetItems, 0, sizeof(mMagnetItems));
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    mIsAsleep = false;
    mWakeUpCounter = 0;
    mOnBungeeState = NOT_ON_BUNGEE;
    mPottedPlantIndex = -1;
    mLaunchRate = aPlantDef.mLaunchRate;
    mSubclass = aPlantDef.mSubClass;
    mRenderOrder = CalcRenderOrder();

    Reanimation* aBodyReanim = NULL;
    if (aPlantDef.mReanimationType != REANIM_NONE)
    {
        float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
        aBodyReanim = mApp->AddReanimation(0.0f, aOffsetY, mRenderOrder + 1, aPlantDef.mReanimationType);
        aBodyReanim->mLoopType = REANIM_LOOP;
        aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);

        if (aBodyReanim->TrackExists("anim_idle"))
            aBodyReanim->SetFramesForLayer("anim_idle");

        if (mApp->IsWallnutBowlingLevel() && aBodyReanim->TrackExists("_ground"))
        {
            aBodyReanim->SetFramesForLayer("_ground");
            if (mSeedType == SEED_WALLNUT || mSeedType == SEED_EXPLODE_O_NUT)
                aBodyReanim->mAnimRate = RandRangeFloat(12.0f, 18.0f);
            else if (mSeedType == SEED_GIANT_WALLNUT)
                aBodyReanim->mAnimRate = RandRangeFloat(6.0f, 10.0f);
        }

        aBodyReanim->mIsAttachment = true;
        mBodyReanimID = mApp->ReanimationGetID(aBodyReanim);
        mBlinkCountdown = 400 + Rand(400);
    }

    if (IsNocturnal(mSeedType) && mBoard && !mBoard->StageIsNight())
        SetSleeping(true);

    if (mLaunchRate > 0)
    {
        if (MakesSun())
            mLaunchCounter = RandRangeInt(300, mLaunchRate / 2);
        else
            mLaunchCounter = RandRangeInt(0, mLaunchRate);
    }
    else
        mLaunchCounter = 0;

    switch (theSeedType)
    {
    case SEED_BLOVER:
    {
        mDoSpecialCountdown = 50;

        if (IsInPlay())
        {
            aBodyReanim->SetFramesForLayer("anim_blow");
            aBodyReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
            aBodyReanim->mAnimRate = 20.0f;
        }
        else
        {
            aBodyReanim->SetFramesForLayer("anim_idle");
            aBodyReanim->mAnimRate = 10.0f;
        }

        break;
    }
    case SEED_PEASHOOTER:
    case SEED_SNOW_PEA:
    case SEED_REPEATER:
    case SEED_LEFTPEATER:
    case SEED_UPGRADE_REPEATER:
        if (aBodyReanim)
        {
            aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
            Reanimation* aHeadReanim = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
            aHeadReanim->mLoopType = REANIM_LOOP;
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            mHeadReanimID = mApp->ReanimationGetID(aHeadReanim);

            if (aBodyReanim->TrackExists("anim_stem"))
                aHeadReanim->AttachToAnotherReanimation(aBodyReanim, "anim_stem");
            else if (aBodyReanim->TrackExists("anim_idle"))
                aHeadReanim->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        }
        break;
    case SEED_SPLIT_PEA:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        Reanimation* aHeadReanim1 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim1->mLoopType = REANIM_LOOP;
        aHeadReanim1->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim1->SetFramesForLayer("anim_head_idle");
        aHeadReanim1->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        mHeadReanimID = mApp->ReanimationGetID(aHeadReanim1);

        Reanimation* aHeadReanim2 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim2->mLoopType = REANIM_LOOP;
        aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim2->SetFramesForLayer("anim_splitpea_idle");
        aHeadReanim2->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        mHeadReanimID2 = mApp->ReanimationGetID(aHeadReanim2);

        break;
    }
    case SEED_THREEPEATER:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        Reanimation* aHeadReanim1 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim1->mLoopType = REANIM_LOOP;
        aHeadReanim1->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim1->SetFramesForLayer("anim_head_idle1");
        aHeadReanim1->AttachToAnotherReanimation(aBodyReanim, "anim_head1");
        mHeadReanimID = mApp->ReanimationGetID(aHeadReanim1);

        Reanimation* aHeadReanim2 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim2->mLoopType = REANIM_LOOP;
        aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim2->SetFramesForLayer("anim_head_idle2");
        aHeadReanim2->AttachToAnotherReanimation(aBodyReanim, "anim_head2");
        mHeadReanimID2 = mApp->ReanimationGetID(aHeadReanim2);

        Reanimation* aHeadReanim3 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim3->mLoopType = REANIM_LOOP;
        aHeadReanim3->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim3->SetFramesForLayer("anim_head_idle3");
        aHeadReanim3->AttachToAnotherReanimation(aBodyReanim, "anim_head3");
        mHeadReanimID3 = mApp->ReanimationGetID(aHeadReanim3);

        break;
    }
    case SEED_WALLNUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Rand(1000);
        break;
    case SEED_EXPLODE_O_NUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Rand(1000);
        aBodyReanim->mColorOverride = Color(255, 64, 64);
        break;
    case SEED_GIANT_WALLNUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Rand(1000);
        break;
    case SEED_TALLNUT:
        mPlantHealth = 8000;
        mHeight = 80;
        mBlinkCountdown = 1000 + Rand(1000);
        break;
    case SEED_GARLIC:
        TOD_ASSERT(aBodyReanim);
        mPlantHealth = 400;
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SEED_GOLD_MAGNET:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SEED_IMITATER:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->mAnimRate = RandRangeFloat(25.0f, 30.0f);
        mStateCountdown = 200;
        break;
    case SEED_CHERRYBOMB:
    case SEED_JALAPENO:
    {
        TOD_ASSERT(aBodyReanim);

        if (IsInPlay())
        {
            mDoSpecialCountdown = 100;

            aBodyReanim->SetFramesForLayer("anim_explode");
            aBodyReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;

            mApp->PlayFoley(FOLEY_REVERSE_EXPLOSION);
        }

        break;
    }
    case SEED_POTATOE_MINE:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = 12.0f;

        if (IsInPlay())
        {
            aBodyReanim->AssignRenderGroupToTrack("anim_glow", RENDER_GROUP_HIDDEN);
            mStateCountdown = 1500;
        }
        else
        {
            aBodyReanim->SetFramesForLayer("anim_armed");
            mState = STATE_POTATO_ARMED;
        }

        break;
    }
    case SEED_GRAVE_BUSTER:
    {
        TOD_ASSERT(aBodyReanim);

        if (IsInPlay())
        {
            aBodyReanim->SetFramesForLayer("anim_land");
            aBodyReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;

            mState = STATE_GRAVEBUSTER_LANDING;
            mApp->PlayFoley(FOLEY_GRAVEBUSTERCHOMP);
        }

        break;
    }
    case SEED_SUNSHROOM:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mFrameBasePose = 6;

        if (IsInPlay())
        {
            mX += Rand(10) - 5;
            mY += Rand(10) - 5;
        }
        else if (mIsAsleep)
            aBodyReanim->SetFramesForLayer("anim_bigsleep");
        else
            aBodyReanim->SetFramesForLayer("anim_bigidle");

        mState = STATE_SUNSHROOM_SMALL;
        mStateCountdown = 12000;

        break;
    }
    case SEED_PUFFSHROOM:
    case SEED_SEASHROOM:
        if (IsInPlay())
        {
            mX += Rand(10) - 5;
            mY += Rand(6) - 3;
        }
        break;
    case SEED_PUMPKIN:
    {
        mPlantHealth = 4000;
        mWidth = 120;

        TOD_ASSERT(aBodyReanim);
        aBodyReanim->AssignRenderGroupToTrack("Pumpkin_back", 1);
        break;
    }
    case SEED_CHOMPER:
        mState = STATE_READY;
        break;
    case SEED_PLANTERN:
    {
        mStateCountdown = 50;
        break;
    }
    case SEED_TORCHWOOD:
        break;
    case SEED_MARIGOLD:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        break;
    case SEED_CACTUS:
        mState = STATE_CACTUS_LOW;
        break;
    case SEED_INSTANT_COFFEE:
        mDoSpecialCountdown = 100;
        break;
    case SEED_SCAREDY_SHROOM:
        mState = STATE_READY;
        break;
    case SEED_COB_CANNON:
        if (IsInPlay())
        {
            mState = STATE_COBCANNON_ARMING;
            mStateCountdown = 500;

            TOD_ASSERT(aBodyReanim);
            aBodyReanim->SetFramesForLayer("anim_unarmed_idle");
        }
        break;
    case SEED_KERNEL_PULT:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_HIDDEN);
        break;
    case SEED_MAGNETSHROOM:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SEED_SPIKEROCK:
        mPlantHealth = 450;
        TOD_ASSERT(aBodyReanim);
        break;
    case SEED_SPROUT:
        break;
    case SEED_FLOWERPOT:
        if (IsInPlay())
        {
            mState = STATE_FLOWERPOT_INVULNERABLE;
            mStateCountdown = 100;
        }
        break;
    case SEED_LILY_PAD:
        if (IsInPlay())
        {
            mState = STATE_LILYPAD_INVULNERABLE;
            mStateCountdown = 100;
        }
        break;
    case SEED_TANGLE_KELP:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
    default:
        break;
    }

    if ((mApp->mGameMode == GAMEMODE_CHALLENGE_BIG_TIME) &&
        (theSeedType == SEED_WALLNUT || theSeedType == SEED_SUNFLOWER || theSeedType == SEED_MARIGOLD))
    {
        mPlantHealth *= 2;
    }
    mPlantMaxHealth = mPlantHealth;

    if (mSeedType != SEED_FLOWERPOT && IsOnBoard())
    {
        TOD_ASSERT(mBoard);
        Plant* aFlowerPot = mBoard->GetFlowerPotAt(mPlantCol, mRow);
        if (aFlowerPot)
            mApp->ReanimationGet(aFlowerPot->mBodyReanimID)->mAnimRate = 0.0f;
    }
}

int Plant::CalcRenderOrder()
{
    PLANT_ORDER anOrder = PLANT_ORDER_NORMAL;
    int aLayer = RENDER_LAYER_PLANT;

    SeedType aSeedType = mSeedType;
    if (mSeedType == SEED_IMITATER && mImitaterType != SEED_NONE)
        aSeedType = mImitaterType;

    if (mApp->IsWallnutBowlingLevel())
    {
        aLayer = RENDER_LAYER_PROJECTILE;
    }
    else if (aSeedType == SEED_PUMPKIN)
    {
        anOrder = PLANT_ORDER_PUMPKIN;
    }
    else if (IsFlying(aSeedType))
    {
        anOrder = PLANT_ORDER_FLYER;
    }
    else if (aSeedType == SEED_FLOWERPOT || (aSeedType == SEED_LILY_PAD && mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN))
    {
        anOrder = PLANT_ORDER_LILYPAD;
    }

    return Board::MakeRenderOrder(static_cast<RenderLayer>(aLayer), mRow, anOrder * 5 - mX + 800);
}

void Plant::SetSleeping(bool theIsAsleep)
{
    if (mIsAsleep == theIsAsleep || NotOnGround())
        return;

    mIsAsleep = theIsAsleep;
    if (theIsAsleep)
    {
        float aPosX = mX + 50.0f;
        float aPosY = mY + 40.0f;
        if (mSeedType == SEED_FUMESHROOM)
            aPosX += 12.0f;
        else if (mSeedType == SEED_SCAREDY_SHROOM)
            aPosY -= 20.0f;
        else if (mSeedType == SEED_GLOOMSHROOM)
            aPosY -= 12.0f;

        Reanimation* aSleepReanim = mApp->AddReanimation(aPosX, aPosY, mRenderOrder + 2, REANIM_SLEEPING);
        aSleepReanim->mLoopType = REANIM_LOOP;
        aSleepReanim->mAnimRate = RandRangeFloat(6.0f, 8.0f);
        aSleepReanim->mAnimTime = RandRangeFloat(0.0f, 0.9f);
        mSleepingReanimID = mApp->ReanimationGetID(aSleepReanim);
    }
    else
    {
        mApp->RemoveReanimation(mSleepingReanimID);
        mSleepingReanimID = REANIMATIONID_NULL;
    }

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == NULL)
        return;

    if (theIsAsleep)
    {
        if (!IsInPlay() && mSeedType == SEED_SUNSHROOM)
        {
            aBodyReanim->SetFramesForLayer("anim_bigsleep");
        }
        else if (aBodyReanim->TrackExists("anim_sleep"))
        {
            float aAnimTime = aBodyReanim->mAnimTime;
            aBodyReanim->StartBlend(20);
            aBodyReanim->SetFramesForLayer("anim_sleep");
            aBodyReanim->mAnimTime = aAnimTime;
        }
        else
        {
            aBodyReanim->mAnimRate = 1.0f;
        }

        EndBlink();
    }
    else
    {
        if (!IsInPlay() && mSeedType == SEED_SUNSHROOM)
        {
            aBodyReanim->SetFramesForLayer("anim_bigidle");
        }
        else if (aBodyReanim->TrackExists("anim_idle"))
        {
            float aAnimTime = aBodyReanim->mAnimTime;
            aBodyReanim->StartBlend(20);
            aBodyReanim->SetFramesForLayer("anim_idle");
            aBodyReanim->mAnimTime = aAnimTime;
        }

        if (aBodyReanim->mAnimRate < 2.0f && IsInPlay())
            aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
    }
}

int Plant::GetDamageRangeFlags(PlantWeapon thePlantWeapon)
{
    switch (mSeedType)
    {
    case SEED_CACTUS:
        return thePlantWeapon == WEAPON_SECONDARY ? 1 : 2;
    case SEED_CHERRYBOMB:
    case SEED_JALAPENO:
    case SEED_COB_CANNON:
    case SEED_DOOM_SHROOM:
        return 127;
    case SEED_MELON_PULT:
    case SEED_CABBAGE_PULT:
    case SEED_KERNEL_PULT:
    case SEED_WINTERMELON:
        return 13;
    case SEED_POTATOE_MINE:
        return 77;
    case SEED_SQUASH:
        return 13;
    case SEED_PUFFSHROOM:
    case SEED_SEASHROOM:
    case SEED_FUMESHROOM:
    case SEED_GLOOMSHROOM:
    case SEED_CHOMPER:
        return 9;
    case SEED_CATTAIL:
        return 11;
    case SEED_TANGLE_KELP:
        return 5;
    case SEED_GIANT_WALLNUT:
        return 17;
    default:
        return 1;
    }
}

bool Plant::IsOnHighGround()
{
    return mBoard && mBoard->mGridSquareType[mPlantCol][mRow] == GRIDSQUARE_HIGH_GROUND;
}

void Plant::SpikeRockTakeDamage()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    SpikeweedAttack();

    mPlantHealth -= 50;
    if (mPlantHealth <= 300)
    {
        aBodyReanim->AssignRenderGroupToTrack("bigspike3", RENDER_GROUP_HIDDEN);
    }
    if (mPlantHealth <= 150)
    {
        aBodyReanim->AssignRenderGroupToTrack("bigspike2", RENDER_GROUP_HIDDEN);
    }
    if (mPlantHealth <= 0)
    {
        mApp->PlayFoley(FOLEY_SQUISH);
        Die();
    }
}

bool Plant::IsSpiky()
{
    return mSeedType == SEED_SPIKEWEED || mSeedType == SEED_SPIKEROCK;
}

void Plant::DoRowAreaDamage(int theDamage, unsigned int theDamageFlags)
{
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(WEAPON_PRIMARY);

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        int aDiffY = (aZombie->mZombieType == ZOMBIE_BOSS) ? 0 : (aZombie->mRow - mRow);
        if (mSeedType == SEED_GLOOMSHROOM)
        {
            if (aDiffY < -1 || aDiffY > 1)
                continue;
        }
        else if (aDiffY)
            continue;

        if (aZombie->mOnHighGround == IsOnHighGround() && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aAttackRect, aZombieRect) > 0)
            {
                int aDamage = theDamage;
                if ((aZombie->mZombieType == ZOMBIE_ZAMBONI || aZombie->mZombieType == ZOMBIE_CATAPULT) &&
                    (TestBit(theDamageFlags, DAMAGE_SPIKE)))
                {
                    aDamage = 1800;
                    if (mSeedType == SEED_SPIKEROCK)
                    {
                        SpikeRockTakeDamage();
                    }
                    else
                    {
                        Die();
                    }
                }

                aZombie->TakeDamage(aDamage, theDamageFlags);
                mApp->PlayFoley(FOLEY_SPLAT);
            }
        }
    }
}

TodParticleSystem* Plant::AddAttachedParticle(int thePosX, int thePosY, int theRenderPosition, ParticleEffect theEffect)
{
    TodParticleSystem* aParticle = mApp->ParticleTryToGet(mParticleID);
    if (aParticle)
        aParticle->ParticleSystemDie();

    TodParticleSystem* aNewParticle = mApp->AddTodParticle(thePosX, thePosY, theRenderPosition, theEffect);
    if (aNewParticle)
        mParticleID = mApp->ParticleGetID(aNewParticle);

    return aNewParticle;
}

bool Plant::FindTargetAndFire(int theRow, PlantWeapon thePlantWeapon)
{
    Zombie* aZombie = FindTargetZombie(theRow, thePlantWeapon);
    if (aZombie == NULL)
        return false;

    EndBlink();
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(mHeadReanimID);

    if (mSeedType == SEED_SPLIT_PEA && thePlantWeapon == WEAPON_SECONDARY)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        aHeadReanim2->StartBlend(20);
        aHeadReanim2->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim2->mAnimRate = 35.0f;
        aHeadReanim2->SetFramesForLayer("anim_splitpea_shooting");
        mShootingCounter = 26;
    }
    else if (aHeadReanim && aHeadReanim->TrackExists("anim_shooting"))
    {
        aHeadReanim->StartBlend(20);
        aHeadReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim->mAnimRate = 35.0f;
        aHeadReanim->SetFramesForLayer("anim_shooting");

        mShootingCounter = 33;
        if (mSeedType == SEED_REPEATER || mSeedType == SEED_SPLIT_PEA || mSeedType == SEED_LEFTPEATER)
        {
            aHeadReanim->mAnimRate = 45.0f;
            mShootingCounter = 26;
        }
        else if (mSeedType == SEED_UPGRADE_REPEATER)
        {
            aHeadReanim->mAnimRate = 38.0f;
            mShootingCounter = 100;
        }
    }
    else if (mState == STATE_CACTUS_HIGH)
    {
        PlayBodyReanim("anim_shootinghigh", REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);
        mShootingCounter = 23;
    }
    else if (mSeedType == SEED_GLOOMSHROOM)
    {
        PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 14.0f);
        mShootingCounter = 200;
    }
    else if (mSeedType == SEED_CATTAIL)
    {
        PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 30.0f);
        mShootingCounter = 50;
    }
    else if (aBodyReanim && aBodyReanim->TrackExists("anim_shooting"))
    {
        PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);

        switch (mSeedType)
        {
        case SEED_FUMESHROOM:       mShootingCounter = 50;  break;
        case SEED_PUFFSHROOM:       mShootingCounter = 29;  break;
        case SEED_SCAREDY_SHROOM:   mShootingCounter = 25;  break;
        case SEED_CABBAGE_PULT:     mShootingCounter = 32;  break;
        case SEED_MELON_PULT:
        case SEED_WINTERMELON:      mShootingCounter = 36;  break;
        case SEED_KERNEL_PULT:
        {
            if (Rand(4) == 0)
            {
                aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_NORMAL);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_kernal", RENDER_GROUP_HIDDEN);
                mState = STATE_KERNELPULT_BUTTER;
            }

            mShootingCounter = 30;
            break;
        }
        case SEED_CACTUS:           mShootingCounter = 35;  break;
        default:                    mShootingCounter = 29;  break;
        }
    }
    else
        Fire(aZombie, theRow, thePlantWeapon);

    return true;
}

void Plant::LaunchThreepeater()
{
    int rowAbove = mRow - 1;
    int rowBelow = mRow + 1;

    if ((FindTargetZombie(mRow, WEAPON_PRIMARY)) ||
        (mBoard->RowCanHaveZombies(rowAbove) && FindTargetZombie(rowAbove, WEAPON_PRIMARY)) ||
        (mBoard->RowCanHaveZombies(rowBelow) && FindTargetZombie(rowBelow, WEAPON_PRIMARY)))
    {
        Reanimation* aHeadReanim1 = mApp->ReanimationGet(mHeadReanimID);
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);

        if (mBoard->RowCanHaveZombies(rowBelow))
        {
            aHeadReanim1->StartBlend(10);
            aHeadReanim1->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
            aHeadReanim1->mAnimRate = 20.0f;
            aHeadReanim1->SetFramesForLayer("anim_shooting1");
        }

        aHeadReanim2->StartBlend(10);
        aHeadReanim2->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim2->mAnimRate = 20.0f;
        aHeadReanim2->SetFramesForLayer("anim_shooting2");

        if (mBoard->RowCanHaveZombies(rowAbove))
        {
            aHeadReanim3->StartBlend(10);
            aHeadReanim3->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
            aHeadReanim3->mAnimRate = 20.0f;
            aHeadReanim3->SetFramesForLayer("anim_shooting3");
        }

        mShootingCounter = 35;
    }
}

bool Plant::FindStarFruitTarget()
{
    if (mRecentlyEatenCountdown > 0)
        return true;

    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
    int aCenterStarX = mX + 40;
    int aCenterStarY = mY + 40;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        Rect aZombieRect = aZombie->GetZombieRect();
        if (aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            if (aZombie->mZombieType == ZOMBIE_BOSS && mPlantCol >= 5)
                return true;

            if (aZombie->mRow == mRow)
            {
                if (aZombieRect.mX + aZombieRect.mWidth < aCenterStarX)
                    return true;
            }
            else
            {
                if (aZombie->mZombieType == ZOMBIE_DIGGER)
                    aZombieRect.mX += 10;

                float aProjectileTime = Distance2D(aCenterStarX, aCenterStarY, aZombieRect.mX + aZombieRect.mWidth / 2, aZombieRect.mY + aZombieRect.mHeight / 2) / 3.33f;
                int aZombieHitX = aZombie->ZombieTargetLeadX(aProjectileTime) - aZombieRect.mWidth / 2;
                if ((aZombieHitX + aZombieRect.mWidth > aCenterStarX) && (aZombieHitX < aCenterStarX))
                    return true;

                int aCenterZombieX = aZombieHitX + aZombieRect.mWidth / 2;
                int aCenterZombieY = aZombieRect.mY + aZombieRect.mHeight / 2;
                float angle = RAD_TO_DEG(atan2(aCenterZombieY - aCenterStarY, aCenterZombieX - aCenterStarX));
                if (abs(aZombie->mRow - mRow) < 2)
                {
                    if ((angle > 20.0f && angle < 40.0f) || (angle < -25.0f && angle > -45.0f))
                        return true;
                }
                else
                {
                    if ((angle > 25.0f && angle < 35.0f) || (angle < -28.0f && angle > -38.0f))
                        return true;
                }
            }
        }
    }

    return false;
}

void Plant::LaunchStarFruit()
{
    if (FindStarFruitTarget())
    {
        PlayBodyReanim("anim_shoot", REANIM_PLAY_ONCE_AND_HOLD, 20, 28.0f);
        mShootingCounter = 40;
    }
}

void Plant::StarFruitFire()
{
    mApp->PlayFoley(FOLEY_THROW);

    float aShootAngleX = cos(DEG_TO_RAD(30.0f)) * 3.33f;
    float aShootAngleY = sin(DEG_TO_RAD(30.0f)) * 3.33f;
    for (int i = 0; i < 5; i++)
    {
        Projectile* aProjectile = mBoard->AddProjectile(mX + 25, mY + 25, mRenderOrder - 1, mRow, PROJECTILE_STAR);
        aProjectile->mDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
        aProjectile->mMotionType = MOTION_STAR;

        switch (i)
        {
        case 0:     aProjectile->mVelX = -3.33f;         aProjectile->mVelY = 0.0f;             break;
        case 1:     aProjectile->mVelX = 0.0f;          aProjectile->mVelY = 3.33f;             break;
        case 2:     aProjectile->mVelX = 0.0f;          aProjectile->mVelY = -3.33f;            break;
        case 3:     aProjectile->mVelX = aShootAngleX;  aProjectile->mVelY = aShootAngleY;      break;
        case 4:     aProjectile->mVelX = aShootAngleX;  aProjectile->mVelY = -aShootAngleY;     break;
        default:    TOD_ASSERT(false);                                                               break;
        }
    }
}

void Plant::UpdateShooter()
{
    mLaunchCounter--;
    if (mLaunchCounter <= 0)
    {
        mLaunchCounter = mLaunchRate - Rand(15);

        if (mSeedType == SEED_THREEPEATER)
        {
            LaunchThreepeater();
        }
        else if (mSeedType == SEED_STARFRUIT)
        {
            LaunchStarFruit();
        }
        else if (mSeedType == SEED_SPLIT_PEA)
        {
            FindTargetAndFire(mRow, WEAPON_PRIMARY);
            FindTargetAndFire(mRow, WEAPON_SECONDARY);
        }
        else if (mSeedType == SEED_CACTUS)
        {
            if (mState == STATE_CACTUS_HIGH)
            {
                FindTargetAndFire(mRow, WEAPON_PRIMARY);
            }
            else if (mState == STATE_CACTUS_LOW)
            {
                FindTargetAndFire(mRow, WEAPON_SECONDARY);
            }
        }
        else
        {
            FindTargetAndFire(mRow, WEAPON_PRIMARY);
        }
    }

    if (mLaunchCounter == 50 && mSeedType == SEED_CATTAIL)
    {
        FindTargetAndFire(mRow, WEAPON_PRIMARY);
    }
    if (mLaunchCounter == 25)
    {
        if (mSeedType == SEED_REPEATER || mSeedType == SEED_LEFTPEATER)
        {
            FindTargetAndFire(mRow, WEAPON_PRIMARY);
        }
        else if (mSeedType == SEED_SPLIT_PEA)
        {
            FindTargetAndFire(mRow, WEAPON_SECONDARY);
        }
    }
}

bool Plant::MakesSun()
{
    return mSeedType == SEED_SUNFLOWER || mSeedType == SEED_TWINSUNFLOWER || mSeedType == SEED_SUNSHROOM;
}

void Plant::UpdateProductionPlant()
{
    if (!IsInPlay() || mApp->IsIZombieLevel() || mApp->mGameMode == GAMEMODE_UPSELL || mApp->mGameMode == GAMEMODE_INTRO)
        return;

    if (mBoard->HasLevelAwardDropped())
        return;

    if (mSeedType == SEED_MARIGOLD && mBoard->mCurrentWave == mBoard->mNumWaves)
    {
        if (mState != STATE_MARIGOLD_ENDING)
        {
            mState = STATE_MARIGOLD_ENDING;
            mStateCountdown = 6000;
        }
        else if (mStateCountdown <= 0)
            return;
    }

    if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND && mBoard->mChallenge->mChallengeState != STATECHALLENGE_LAST_STAND_ONSLAUGHT)
        return;

    mLaunchCounter--;
    if (mLaunchCounter <= 100)
    {
        int aFlashCountdown = TodAnimateCurve(100, 0, mLaunchCounter, 0, 100, CURVE_LINEAR);
        mEatenFlashCountdown = std::max(mEatenFlashCountdown, aFlashCountdown);
    }
    if (mLaunchCounter <= 0)
    {
        mLaunchCounter = RandRangeInt(mLaunchRate - 150, mLaunchRate);
        mApp->PlayFoley(FOLEY_SPAWN_SUN);

        if (mSeedType == SEED_SUNSHROOM)
        {
            if (mState == STATE_SUNSHROOM_SMALL)
            {
                mBoard->AddCoin(mX, mY, COIN_SMALLSUN, COIN_MOTION_FROM_PLANT);
            }
            else
            {
                mBoard->AddCoin(mX, mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
            }
        }
        else if (mSeedType == SEED_SUNFLOWER)
        {
            mBoard->AddCoin(mX, mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
        }
        else if (mSeedType == SEED_TWINSUNFLOWER)
        {
            mBoard->AddCoin(mX, mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
            mBoard->AddCoin(mX, mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
        }
        else if (mSeedType == SEED_MARIGOLD)
        {
            mBoard->AddCoin(mX, mY, (Rand(100) < 10) ? COIN_GOLD : COIN_SILVER, COIN_MOTION_COIN);
        }

        if (mApp->mGameMode == GAMEMODE_CHALLENGE_BIG_TIME)
        {
            if (mSeedType == SEED_SUNFLOWER)
            {
                mBoard->AddCoin(mX, mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
            }
            else if (mSeedType == SEED_MARIGOLD)
            {
                mBoard->AddCoin(mX, mY, COIN_SILVER, COIN_MOTION_COIN);
            }
        }
    }
}

void Plant::UpdateSunShroom()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == STATE_SUNSHROOM_SMALL)
    {
        if (mStateCountdown == 0)
        {
            PlayBodyReanim("anim_grow", REANIM_PLAY_ONCE_AND_HOLD, 10, 12.0f);
            mState = STATE_SUNSHROOM_GROWING;
            mApp->PlayFoley(FOLEY_PLANTGROW);
        }

        UpdateProductionPlant();
    }
    else if (mState == STATE_SUNSHROOM_GROWING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_bigidle", REANIM_LOOP, 10, RandRangeFloat(12.0f, 15.0f));
            mState = STATE_SUNSHROOM_BIG;
        }
    }
    else
    {
        UpdateProductionPlant();
    }
}

void Plant::UpdateGraveBuster()
{
    if (mState == STATE_GRAVEBUSTER_LANDING)
    {
        if (mApp->ReanimationGet(mBodyReanimID)->mLoopCount > 0)
        {
            PlayBodyReanim("anim_idle", REANIM_LOOP, 10, 12.0f);
            mStateCountdown = 400;
            mState = STATE_GRAVEBUSTER_EATING;
            AddAttachedParticle(mX + 40, mY + 40, mRenderOrder + 4, PARTICLE_GRAVE_BUSTER);
        }
    }
    else if (mState == STATE_GRAVEBUSTER_EATING && mStateCountdown == 0)
    {
        GridItem* aGraveStone = mBoard->GetGraveStoneAt(mPlantCol, mRow);
        if (aGraveStone)
        {
            aGraveStone->GridItemDie();
            mBoard->mGravesCleared++;
        }

        mApp->AddTodParticle(mX + 40, mY + 40, mRenderOrder + 4, PARTICLE_GRAVE_BUSTER_DIE);
        Die();
        mBoard->DropLootPiece(mX + 40, mY, static_cast<CoinType>(12));
    }
}

void Plant::PlayBodyReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate)
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    if (theBlendTime > 0)
        aBodyReanim->StartBlend(theBlendTime);
    if (theAnimRate > 0.0f)
        aBodyReanim->mAnimRate = theAnimRate;

    aBodyReanim->mLoopType = theLoopType;
    aBodyReanim->mLoopCount = 0;
    aBodyReanim->SetFramesForLayer(theTrackName);
}

void Plant::UpdatePotato()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    if (mState == STATE_NOTREADY)
    {
        if (mStateCountdown == 0)
        {
            mApp->AddTodParticle(mX + mWidth / 2, mY + mHeight / 2, mRenderOrder, PARTICLE_POTATO_MINE_RISE);
            PlayBodyReanim("anim_rise", REANIM_PLAY_ONCE_AND_HOLD, 20, 18.0f);
            mState = STATE_POTATO_RISING;
            mApp->PlayFoley(FOLEY_DIRT_RISE);
        }
    }
    else if (mState == STATE_POTATO_RISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            float aRate = RandRangeFloat(12.0f, 15.0f);
            PlayBodyReanim("anim_armed", REANIM_LOOP, 0, aRate);

            Reanimation* aLightReanim = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, GetPlantDefinition(mSeedType).mReanimationType);
            aLightReanim->mLoopType = REANIM_LOOP;
            aLightReanim->mAnimRate = aRate - 2.0f;
            aLightReanim->SetFramesForLayer("anim_glow");
            aLightReanim->mFrameCount = 10;
            aLightReanim->ShowOnlyTrack("anim_glow");
            aLightReanim->SetTruncateDisappearingFrames("anim_glow", false);
            mLightReanimID = mApp->ReanimationGetID(aLightReanim);
            aLightReanim->AttachToAnotherReanimation(aBodyReanim, "anim_light");

            mState = STATE_POTATO_ARMED;
            mBlinkCountdown = 400 + Rand(4000);
        }
    }
    else if (mState == STATE_POTATO_ARMED)
    {
        if (FindTargetZombie(mRow, WEAPON_PRIMARY))
        {
            DoSpecial();
        }
        else
        {
            Reanimation* aLightReanim = mApp->ReanimationTryToGet(mLightReanimID);
            if (aLightReanim)
            {
                aLightReanim->mFrameCount = TodAnimateCurve(200, 50, DistanceToClosestZombie(), 10, 3, CURVE_LINEAR);
            }
        }
    }
}

void Plant::UpdateTanglekelp()
{
    if (mState != STATE_TANGLEKELP_GRABBING)
    {
        Zombie* aZombie = FindTargetZombie(mRow, WEAPON_PRIMARY);
        if (aZombie)
        {
            mApp->PlayFoley(FOLEY_FLOOP);
            mState = STATE_TANGLEKELP_GRABBING;
            mStateCountdown = 100;
            aZombie->PoolSplash(false);

            float aVinesPosX = -13.0f;
            float aVinesPosY = 15.0f;
            if (aZombie->mZombieType == ZOMBIE_SNORKEL)
            {
                aVinesPosX = -43.0f;
                aVinesPosY = 55.0f;
            }
            if (aZombie->mZombiePhase == PHASE_DOLPHIN_RIDING)
            {
                aVinesPosX = -20.0f;
                aVinesPosY = 37.0f;
            }
            Reanimation* aGrabReanim = aZombie->AddAttachedReanim(aVinesPosX, aVinesPosY, REANIM_TANGLEKELP);
            if (aGrabReanim)
            {
                aGrabReanim->SetFramesForLayer("anim_grab");
                aGrabReanim->mAnimRate = 24.0f;
                aGrabReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
            }

            mTargetZombieID = static_cast<ZombieID>(mBoard->ZombieGetID(aZombie));
        }
    }
    else
    {
        if (mStateCountdown == 50)
        {
            Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
            if (aZombie)
            {
                aZombie->DragUnder();
                aZombie->PoolSplash(false);
            }
        }

        if (mStateCountdown == 20)
        {
            int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, mRow, 0);
            Reanimation* aSplashReanim = mApp->AddReanimation(mX - 23, mY + 7, aRenderPosition, REANIM_SPLASH);
            aSplashReanim->OverrideScale(1.3f, 1.3f);

            mApp->AddTodParticle(mX + 31, mY + 64, aRenderPosition, PARTICLE_PLANTING_POOL);
            mApp->PlayFoley(FOLEY_ZOMBIE_ENTERING_WATER);
        }

        if (mStateCountdown == 0)
        {
            Die();

            Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
            if (aZombie)
            {
                aZombie->DieWithLoot();
            }
        }
    }
}

void Plant::SpikeweedAttack()
{
    TOD_ASSERT(IsSpiky());

    if (mState != STATE_SPIKEWEED_ATTACKING)
    {
        PlayBodyReanim("anim_attack", REANIM_PLAY_ONCE_AND_HOLD, 20, 18.0f);
        mApp->PlaySample(SOUND_THROW);

        mState = STATE_SPIKEWEED_ATTACKING;
        mStateCountdown = 100;
    }
}

void Plant::UpdateSpikeweed()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == STATE_SPIKEWEED_ATTACKING)
    {
        if (mStateCountdown == 0)
        {
            mState = STATE_NOTREADY;
        }
        else if (mSeedType == SEED_SPIKEROCK)
        {
            if (mStateCountdown == 69 || mStateCountdown == 33)
            {
                DoRowAreaDamage(20, 33U);
            }
        }
        else if (mStateCountdown == 75)
        {
            DoRowAreaDamage(20, 33U);
        }

        if (aBodyReanim->mLoopCount > 0)
        {
            PlayIdleAnim(RandRangeFloat(12.0f, 15.0f));
        }
    }
    else if (FindTargetZombie(mRow, WEAPON_PRIMARY))
    {
        SpikeweedAttack();
    }
}

void Plant::UpdateScaredyShroom()
{
    if (mShootingCounter > 0)
        return;

    bool aHasZombieNearby = false;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        Rect aZombieRect = aZombie->GetZombieRect();
        int aDiffY = (aZombie->mZombieType == ZOMBIE_BOSS) ? 0 : (aZombie->mRow - mRow);
        if (!aZombie->mMindControlled && !aZombie->IsDeadOrDying() && aDiffY <= 1 && aDiffY >= -1 && GetCircleRectOverlap(mX, mY + 20.0f, 120, aZombieRect))
        {
            aHasZombieNearby = true;
            break;
        }
    }

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == STATE_READY)
    {
        if (aHasZombieNearby)
        {
            mState = STATE_SCAREDYSHROOM_LOWERING;
            PlayBodyReanim("anim_scared", REANIM_PLAY_ONCE_AND_HOLD, 10, 10.0f);
        }
    }
    else if (mState == STATE_SCAREDYSHROOM_LOWERING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_SCAREDYSHROOM_SCARED;
            PlayBodyReanim("anim_scaredidle", REANIM_LOOP, 10, 0.0f);
        }
    }
    else if (mState == STATE_SCAREDYSHROOM_SCARED)
    {
        if (!aHasZombieNearby)
        {
            mState = STATE_SCAREDYSHROOM_RAISING;

            float aAnimRate = RandRangeFloat(7.0f, 12.0f);
            PlayBodyReanim("anim_grow", REANIM_PLAY_ONCE_AND_HOLD, 10, aAnimRate);
        }
    }
    else if (mState == STATE_SCAREDYSHROOM_RAISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_READY;

            float aAnimRate = RandRangeFloat(10.0f, 15.0f);
            PlayIdleAnim(aAnimRate);
        }
    }

    if (mState != STATE_READY)
    {
        mLaunchCounter = mLaunchRate;
    }
}

void Plant::UpdateTorchwood()
{
    Rect aAttackRect = GetPlantAttackRect(WEAPON_PRIMARY);

    Projectile* aProjectile = NULL;
    while (mBoard->IterateProjectiles(aProjectile))
    {
        if ((aProjectile->mRow == mRow) &&
            (aProjectile->mProjectileType == PROJECTILE_PEA || aProjectile->mProjectileType == PROJECTILE_SNOWPEA))
        {
            Rect aProjectileRect = aProjectile->GetProjectileRect();
            if (GetRectOverlap(aAttackRect, aProjectileRect) >= 10)
            {
                if (aProjectile->mProjectileType == PROJECTILE_PEA)
                {
                    aProjectile->ConvertToFireball(mPlantCol);
                }
                else if (aProjectile->mProjectileType == PROJECTILE_SNOWPEA)
                {
                    aProjectile->ConvertToPea(mPlantCol);
                }
            }
        }
    }
}

void Plant::DoSquashDamage()
{
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(WEAPON_PRIMARY);

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mRow == mRow || aZombie->mZombieType == ZOMBIE_BOSS) && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aAttackRect, aZombieRect) > (aZombie->mZombieType == ZOMBIE_FOOTBALL ? -20 : 0))
            {
                aZombie->TakeDamage(1800, 18U);
            }
        }
    }
}

Zombie* Plant::FindSquashTarget()
{
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(WEAPON_PRIMARY);

    int aClosestRange = 0;
    Zombie* aClosestZombie = NULL;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mRow == mRow || aZombie->mZombieType == ZOMBIE_BOSS) &&
            aZombie->mHasHead && !aZombie->IsTangleKelpTarget() && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();

            if ((
                    aZombie->mZombiePhase == PHASE_POLEVAULTER_PRE_VAULT && aZombieRect.mX < mX + 20
                ) || (
                    aZombie->mZombiePhase != PHASE_POLEVAULTER_PRE_VAULT &&
                    aZombie->mZombiePhase != PHASE_POLEVAULTER_IN_VAULT &&
                    aZombie->mZombiePhase != PHASE_SNORKEL_INTO_POOL &&
                    aZombie->mZombiePhase != PHASE_DOLPHIN_INTO_POOL &&
                    aZombie->mZombiePhase != PHASE_DOLPHIN_RIDING &&
                    aZombie->mZombiePhase != PHASE_DOLPHIN_IN_JUMP &&
                    !aZombie->IsBobsledTeamWithSled()
                ))
            {
                int aRange = -GetRectOverlap(aAttackRect, aZombieRect);
                if (aRange <= (aZombie->mIsEating ? 110 : 70))
                {
                    int aPlantX = aAttackRect.mX;
                    if (aZombie->mZombiePhase == PHASE_POLEVAULTER_POST_VAULT || aZombie->mZombiePhase == PHASE_POLEVAULTER_PRE_VAULT ||
                        aZombie->mZombiePhase == PHASE_DOLPHIN_WALKING_IN_POOL || aZombie->mZombieType == ZOMBIE_IMP ||
                        aZombie->mZombieType == ZOMBIE_FOOTBALL || mApp->IsScaryPotterLevel())
                    {
                        aPlantX = aAttackRect.mX - 60;
                    }

                    if (aZombie->IsWalkingBackwards() || aZombieRect.mX + aZombieRect.mWidth >= aPlantX)
                    {
                        if (mBoard->ZombieGetID(aZombie) == mTargetZombieID)
                            return aZombie;

                        if (aClosestZombie == NULL || aRange < aClosestRange)
                        {
                            aClosestZombie = aZombie;
                            aClosestRange = aRange;
                        }
                    }
                }
            }
        }
    }

    return aClosestZombie;
}

void Plant::UpdateSquash()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    (void)aBodyReanim;
    TOD_ASSERT(aBodyReanim);

    if (mState == STATE_NOTREADY)
    {
        Zombie* aZombie = FindSquashTarget();
        if (aZombie)
        {
            mTargetZombieID = static_cast<ZombieID>(mBoard->ZombieGetID(aZombie));
            mTargetX = aZombie->ZombieTargetLeadX(0.0f) - mWidth / 2;
            mState = STATE_SQUASH_LOOK;
            mStateCountdown = 80;
            PlayBodyReanim(mTargetX < mX ? "anim_lookleft" : "anim_lookright", REANIM_PLAY_ONCE_AND_HOLD, 10, 24.0f);
            mApp->PlayFoley(FOLEY_SQUASH_HMM);
        }
    }
    else if (mState == STATE_SQUASH_LOOK)
    {
        if (mStateCountdown <= 0)
        {
            PlayBodyReanim("anim_jumpup", REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
            mState = STATE_SQUASH_PRE_LAUNCH;
            mStateCountdown = 45;
        }
    }
    else if (mState == STATE_SQUASH_PRE_LAUNCH)
    {
        if (mStateCountdown <= 0)
        {
            Zombie* aZombie = FindSquashTarget();
            if (aZombie)
            {
                mTargetX = aZombie->ZombieTargetLeadX(30.0f) - mWidth / 2;
            }

            mState = STATE_SQUASH_RISING;
            mStateCountdown = 50;
            mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, mRow, 0);
        }
    }
    else
    {
        int aTargetCol = mBoard->PixelToGridXKeepOnBoard(mTargetX, mY);
        int aDestY = mBoard->GridToPixelY(aTargetCol, mRow) + 8;

        if (mState == STATE_SQUASH_RISING)
        {
            mX = TodAnimateCurve(50, 20, mStateCountdown, mBoard->GridToPixelX(mPlantCol, mStartRow), mTargetX, CURVE_EASE_IN_OUT);
            mY = TodAnimateCurve(50, 20, mStateCountdown, mBoard->GridToPixelY(mPlantCol, mStartRow), aDestY - 120, CURVE_EASE_IN_OUT);

            if (mStateCountdown == 0)
            {
                PlayBodyReanim("anim_jumpdown", REANIM_PLAY_ONCE_AND_HOLD, 0, 60.0f);
                mState = STATE_SQUASH_FALLING;
                mStateCountdown = 10;
            }
        }
        else if (mState == STATE_SQUASH_FALLING)
        {
            mY = TodAnimateCurve(10, 0, mStateCountdown, aDestY - 120, aDestY, CURVE_EASE_IN_OUT);

            if (mStateCountdown == 5)
            {
                DoSquashDamage();
            }

            if (mStateCountdown == 0)
            {
                if (mBoard->IsPoolSquare(aTargetCol, mRow))
                {
                    mApp->AddReanimation(mX - 11, mY + 20, mRenderOrder + 1, REANIM_SPLASH);
                    mApp->PlayFoley(FOLEY_SPLAT);
                    mApp->PlaySample(SOUND_ZOMBIESPLASH);

                    Die();
                }
                else
                {
                    mState = STATE_SQUASH_DONE_FALLING;
                    mStateCountdown = 100;

                    mBoard->ShakeBoard(1, 4);
                    mApp->PlayFoley(FOLEY_THUMP);
                    float aOffsetY = mBoard->StageHasRoof() ? 69.0f : 80.0f;
                    mApp->AddTodParticle(mX + 40, mY + aOffsetY, mRenderOrder + 4, PARTICLE_DUST_SQUASH);
                }
            }
        }
        else if (mState == STATE_SQUASH_DONE_FALLING)
        {
            if (mStateCountdown == 0)
            {
                Die();
            }
        }
    }
}

void Plant::UpdateDoomShroom()
{
    if (mIsAsleep || mState == STATE_DOINGSPECIAL)
        return;

    mState = STATE_DOINGSPECIAL;
    mDoSpecialCountdown = 100;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    TOD_ASSERT(aBodyReanim);

    aBodyReanim->SetFramesForLayer("anim_explode");
    aBodyReanim->mAnimRate = 23.0f;
    aBodyReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
    aBodyReanim->SetShakeOverride("DoomShroom_head1", 1.0f);
    aBodyReanim->SetShakeOverride("DoomShroom_head2", 2.0f);
    aBodyReanim->SetShakeOverride("DoomShroom_head3", 2.0f);
    mApp->PlayFoley(FOLEY_REVERSE_EXPLOSION);
}

void Plant::UpdateIceShroom()
{
    if (!mIsAsleep && mState != STATE_DOINGSPECIAL)
    {
        mState = STATE_DOINGSPECIAL;
        mDoSpecialCountdown = 100;
    }
}

void Plant::UpdateBlover()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (aBodyReanim->mLoopCount > 0 && aBodyReanim->mLoopType != REANIM_LOOP)
    {
        aBodyReanim->SetFramesForLayer("anim_loop");
        aBodyReanim->mLoopType = REANIM_LOOP;
    }
}

void Plant::UpdateFlowerPot()
{
    if (mState == STATE_FLOWERPOT_INVULNERABLE && mStateCountdown == 0)
        mState = STATE_NOTREADY;
}

void Plant::UpdateLilypad()
{
    if (mState == STATE_LILYPAD_INVULNERABLE && mStateCountdown == 0)
        mState = STATE_NOTREADY;
}

void Plant::UpdateCoffeeBean()
{
    if (mState == STATE_DOINGSPECIAL)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            Die();
        }
    }
}

void Plant::UpdateUmbrella()
{
    if (mState == STATE_UMBRELLA_TRIGGERED)
    {
        if (mStateCountdown == 0)
        {
            mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PROJECTILE, mRow + 1, 0);
            mState = STATE_UMBRELLA_REFLECTING;
        }
    }
    else if (mState == STATE_UMBRELLA_REFLECTING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayIdleAnim(0.0f);
            mState = STATE_NOTREADY;
            mRenderOrder = CalcRenderOrder();
        }
    }
}

void Plant::UpdateCobCannon()
{
    if (mState == STATE_COBCANNON_ARMING)
    {
        if (mStateCountdown == 0)
        {
            mState = STATE_COBCANNON_LOADING;
            PlayBodyReanim("anim_charge", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
        }
    }
    else if (mState == STATE_COBCANNON_LOADING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.5f))
        {
            mApp->PlayFoley(FOLEY_SHOOP);
        }
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_COBCANNON_READY;
            PlayIdleAnim(12.0f);
        }
    }
    else if (mState == STATE_COBCANNON_READY)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        ReanimatorTrackInstance* aCobTrack = aBodyReanim->GetTrackInstanceByName("CobCannon_cob");
        aCobTrack->mTrackColor = static_cast<int>(GetFlashingColor(mBoard->mMainCounter, 75).ToInt());
    }
    else if (mState == STATE_COBCANNON_FIRING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.48f))
        {
            mApp->PlayFoley(FOLEY_COB_LAUNCH);
        }
    }
}

void Plant::UpdateCactus()
{
    if (mShootingCounter > 0)
        return;

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == STATE_CACTUS_RISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_CACTUS_HIGH;
            PlayBodyReanim("anim_idlehigh", REANIM_LOOP, 20, 0.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0;
            }

            mLaunchCounter = 1;
        }
    }
    else if (mState == STATE_CACTUS_HIGH)
    {
        if (FindTargetZombie(mRow, WEAPON_PRIMARY) == NULL)
        {
            mState = STATE_CACTUS_LOWERING;
            PlayBodyReanim("anim_lower", REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
        }
    }
    else if (mState == STATE_CACTUS_LOWERING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_CACTUS_LOW;
            PlayIdleAnim(0.0f);
        }
    }
    else if (FindTargetZombie(mRow, WEAPON_PRIMARY))
    {
        mState = STATE_CACTUS_RISING;
        PlayBodyReanim("anim_rise", REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
        mApp->PlayFoley(FOLEY_PLANTGROW);
    }
}

void Plant::UpdateChomper()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (mState == STATE_READY)
    {
        if (FindTargetZombie(mRow, WEAPON_PRIMARY))
        {
            PlayBodyReanim("anim_bite", REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
            mState = STATE_CHOMPER_BITING;
            mStateCountdown = 70;
        }
    }
    else if (mState == STATE_CHOMPER_BITING)
    {
        if (mStateCountdown == 0)
        {
            mApp->PlayFoley(FOLEY_BIGCHOMP);

            Zombie* aZombie = FindTargetZombie(mRow, WEAPON_PRIMARY);
            bool doBite = false;
            if (aZombie)
            {
                if (aZombie->mZombieType == ZOMBIE_GARGANTUAR || aZombie->mZombieType == ZOMBIE_REDEYE_GARGANTUAR ||
                    aZombie->mZombieType == ZOMBIE_BOSS)
                {
                    doBite = true;
                }
            }
            bool doMiss = false;
            if (aZombie == NULL)
            {
                doMiss = true;
            }
            else if (!aZombie->IsImmobilizied())
            {
                if (aZombie->IsBouncingPogo() ||
                    aZombie->mZombiePhase == PHASE_POLEVAULTER_IN_VAULT || aZombie->mZombiePhase == PHASE_POLEVAULTER_PRE_VAULT)
                {
                    doMiss = true;
                }
            }

            if (doBite)
            {
                mApp->PlayFoley(FOLEY_SPLAT);
                aZombie->TakeDamage(40, 0U);
                mState = STATE_CHOMPER_BITING_MISSED;
            }
            else if (doMiss)
            {
                mState = STATE_CHOMPER_BITING_MISSED;
            }
            else
            {
                aZombie->DieWithLoot();
                mState = STATE_CHOMPER_BITING_GOT_ONE;
            }
        }
    }
    else if (mState == STATE_CHOMPER_BITING_GOT_ONE)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_chew", REANIM_LOOP, 0, 15.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0;
            }

            mState = STATE_CHOMPER_DIGESTING;
            mStateCountdown = 4000;
        }
    }
    else if (mState == STATE_CHOMPER_DIGESTING)
    {
        if (mStateCountdown == 0)
        {
            PlayBodyReanim("anim_swallow", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
            mState = STATE_CHOMPER_SWALLOWING;
        }
    }
    else if ((mState == STATE_CHOMPER_SWALLOWING || mState == STATE_CHOMPER_BITING_MISSED) && aBodyReanim->mLoopCount > 0)
    {
        PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
        mState = STATE_READY;
    }
}

MagnetItem* Plant::GetFreeMagnetItem()
{
    if (mSeedType == SEED_GOLD_MAGNET)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            if (mMagnetItems[i].mItemType == MAGNET_ITEM_NONE)
            {
                return &mMagnetItems[i];
            }
        }

        return NULL;
    }

    return &mMagnetItems[0];
}

void Plant::MagnetShroomAttactItem(Zombie* theZombie)
{
    mState = STATE_MAGNETSHROOM_SUCKING;
    mStateCountdown = 1500;
    PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
    mApp->PlayFoley(FOLEY_MAGNETSHROOM);

    MagnetItem* aMagnetItem = GetFreeMagnetItem();
    if (theZombie->mHelmType == HELMTYPE_PAIL)
    {
        int aDamageIndex = theZombie->GetHelmDamageIndex();

        theZombie->mHelmHealth = 0;
        theZombie->mHelmType = HELMTYPE_NONE;
        theZombie->GetTrackPosition("anim_bucket", aMagnetItem->mPosX, aMagnetItem->mPosY);
        theZombie->ReanimShowPrefix("anim_bucket", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_NORMAL);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_BUCKET1->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_BUCKET1->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 25.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MAGNET_ITEM_PAIL_1) + aDamageIndex);
    }
    else if (theZombie->mHelmType == HELMTYPE_FOOTBALL)
    {
        int aDamageIndex = theZombie->GetHelmDamageIndex();

        theZombie->mHelmHealth = 0;
        theZombie->mHelmType = HELMTYPE_NONE;
        theZombie->GetTrackPosition("zombie_football_helmet", aMagnetItem->mPosX, aMagnetItem->mPosY);
        theZombie->ReanimShowPrefix("zombie_football_helmet", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_NORMAL);

        aMagnetItem->mPosX += 37.0f;
        aMagnetItem->mPosY -= 60.0f;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MAGNET_ITEM_FOOTBALL_HELMET_1) + aDamageIndex);
    }
    else if (theZombie->mShieldType == SHIELDTYPE_DOOR)
    {
        int aDamageIndex = theZombie->GetShieldDamageIndex();

        theZombie->DetachShield();
        theZombie->mZombiePhase = PHASE_ZOMBIE_NORMAL;
        if (!theZombie->mIsEating)
        {
            TOD_ASSERT(theZombie->mZombieHeight == HEIGHT_ZOMBIE_NORMAL);
            theZombie->StartWalkAnim(0);
        }
        theZombie->GetTrackPosition("anim_screendoor", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_SCREENDOOR1->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_SCREENDOOR1->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MAGNET_ITEM_DOOR_1) + aDamageIndex);
    }
    else if (theZombie->mShieldType == SHIELDTYPE_LADDER)
    {
        int aDamageIndex = theZombie->GetShieldDamageIndex();

        theZombie->DetachShield();

        aMagnetItem->mPosX = theZombie->mPosX + 31.0f;
        aMagnetItem->mPosY = theZombie->mPosY + 20.0f;
        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_LADDER_5->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_LADDER_5->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MAGNET_ITEM_LADDER_1) + aDamageIndex);
    }
    else if (theZombie->mZombieType == ZOMBIE_POGO)
    {
        theZombie->PogoBreak(16U);
        theZombie->GetTrackPosition("Zombie_pogo_stick", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX += 40.0f - IMAGE_REANIM_ZOMBIE_LADDER_5->GetWidth() / 2;
        aMagnetItem->mPosY += 84.0f - IMAGE_REANIM_ZOMBIE_LADDER_5->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = theZombie->mHasArm ? MAGNET_ITEM_POGO_1 : MAGNET_ITEM_POGO_3;
    }
    else if (theZombie->mZombiePhase == PHASE_JACK_IN_THE_BOX_RUNNING)
    {
        theZombie->StopZombieSound();
        theZombie->PickRandomSpeed();
        theZombie->mZombiePhase = PHASE_ZOMBIE_NORMAL;
        theZombie->ReanimShowPrefix("Zombie_jackbox_box", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("Zombie_jackbox_handle", RENDER_GROUP_HIDDEN);
        theZombie->GetTrackPosition("Zombie_jackbox_box", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_JACKBOX_BOX->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_JACKBOX_BOX->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 15.0f;
        aMagnetItem->mItemType = MAGNET_ITEM_JACK_IN_THE_BOX;
    }
    else if (theZombie->mZombieType == ZOMBIE_DIGGER)
    {
        theZombie->DiggerLoseAxe();
        theZombie->GetTrackPosition("Zombie_digger_pickaxe", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 45.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 15.0f;
        aMagnetItem->mItemType = MAGNET_ITEM_PICK_AXE;
    }
}

bool Plant::DrawMagnetItemsOnTop()
{
    if (mSeedType == SEED_GOLD_MAGNET)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            if (mMagnetItems[i].mItemType != MAGNET_ITEM_NONE)
            {
                return true;
            }
        }

        return false;
    }

    if (mSeedType == SEED_MAGNETSHROOM)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            MagnetItem* aMagnetItem = &mMagnetItems[i];
            if (aMagnetItem->mItemType != MAGNET_ITEM_NONE)
            {
                SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
                if (aVectorToPlant.Magnitude() > 20.0f)
                {
                    return true;
                }
            }
        }

        return false;
    }

    return false;
}

void Plant::UpdateMagnetShroom()
{
    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MAGNET_ITEM_NONE)
        {
            SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
            if (aVectorToPlant.Magnitude() > 20.0f)
            {
                aMagnetItem->mPosX += aVectorToPlant.x * 0.05f;
                aMagnetItem->mPosY += aVectorToPlant.y * 0.05f;
            }
        }
    }

    if (mState == STATE_MAGNETSHROOM_CHARGING)
    {
        if (mStateCountdown == 0)
        {
            mState = STATE_READY;

            float aAnimRate = RandRangeFloat(10.0f, 15.0f);
            PlayBodyReanim("anim_idle", REANIM_LOOP, 30, aAnimRate);
            if (mApp->IsIZombieLevel())
            {
                Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
                aBodyReanim->mAnimRate = 0.0f;
            }

            mMagnetItems[0].mItemType = MAGNET_ITEM_NONE;
        }
    }
    else if (mState == STATE_MAGNETSHROOM_SUCKING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_nonactive_idle2", REANIM_LOOP, 20, 2.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
                aBodyReanim->mAnimRate = 0.0f;
            }

            mState = STATE_MAGNETSHROOM_CHARGING;
        }
    }
    else
    {
        float aClosestDistance = 0.0f;
        Zombie* aClosestZombie = NULL;

        Zombie* aZombie = NULL;
        while (mBoard->IterateZombies(aZombie))
        {
            int aDiffY = aZombie->mRow - mRow;
            Rect aZombieRect = aZombie->GetZombieRect();

            if (aZombie->mMindControlled)
                continue;

            if (!aZombie->mHasHead)
                continue;

            if (aZombie->mZombieHeight != HEIGHT_ZOMBIE_NORMAL || aZombie->mZombiePhase == PHASE_RISING_FROM_GRAVE)
                continue;

            if (aZombie->IsDeadOrDying())
                continue;

            if (aZombieRect.mX > BOARD_WIDTH || aDiffY > 2 || aDiffY < -2)
                continue;

            if (aZombie->mZombiePhase == PHASE_DIGGER_TUNNELING ||
                aZombie->mZombiePhase == PHASE_DIGGER_STUNNED ||
                aZombie->mZombiePhase == PHASE_DIGGER_WALKING ||
                aZombie->mZombieType == ZOMBIE_POGO)
            {
                if (!aZombie->mHasObject)
                    continue;
            }
            else if (!(aZombie->mHelmType == HELMTYPE_PAIL ||
                aZombie->mHelmType == HELMTYPE_FOOTBALL ||
                aZombie->mShieldType == SHIELDTYPE_DOOR ||
                aZombie->mShieldType == SHIELDTYPE_LADDER ||
                aZombie->mZombiePhase == PHASE_JACK_IN_THE_BOX_RUNNING))
                continue;

            int aRadius = aZombie->mIsEating ? 320 : 270;
            if (GetCircleRectOverlap(mX, mY + 20, aRadius, aZombieRect))
            {
                float aDistance = Distance2D(mX, mY, aZombieRect.mX, aZombieRect.mY);
                aDistance += abs(aDiffY) * 80.0f;

                if (aClosestZombie == NULL || aDistance < aClosestDistance)
                {
                    aClosestZombie = aZombie;
                    aClosestDistance = aDistance;
                }
            }
        }

        if (aClosestZombie)
        {
            MagnetShroomAttactItem(aClosestZombie);
            return;
        }

        float aClosestLadderDist = 0.0f;
        GridItem* aClosestLadder = NULL;

        GridItem* aGridItem = NULL;
        while (mBoard->IterateGridItems(aGridItem))
        {
            if (aGridItem->mGridItemType == GRIDITEM_LADDER)
            {
                int aDiffX = abs(aGridItem->mGridX - mPlantCol);
                int aDiffY = abs(aGridItem->mGridY - mRow);
                int aSquareDistance = std::max(aDiffX, aDiffY);
                if (aSquareDistance <= 2)
                {
                    float aDistance = aSquareDistance + aDiffY * 0.05f;
                    if (aClosestLadder == NULL || aDistance < aClosestLadderDist)
                    {
                        aClosestLadder = aGridItem;
                        aClosestLadderDist = aDistance;
                    }
                }
            }
        }

        if (aClosestLadder)
        {
            mState = STATE_MAGNETSHROOM_SUCKING;
            mStateCountdown = 1500;
            PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
            mApp->PlayFoley(FOLEY_MAGNETSHROOM);

            aClosestLadder->GridItemDie();

            MagnetItem* aMagnetItem = GetFreeMagnetItem();
            aMagnetItem->mPosX = mBoard->GridToPixelX(aClosestLadder->mGridX, aClosestLadder->mGridY) + 40;
            aMagnetItem->mPosY = mBoard->GridToPixelY(aClosestLadder->mGridX, aClosestLadder->mGridY);
            aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 10.0f;
            aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
            aMagnetItem->mItemType = MAGNET_ITEM_LADDER_PLACED;
        }
    }
}

Coin* Plant::FindGoldMagnetTarget()
{
    Coin* aClosestCoin = NULL;
    float aClosestDistance = 0.0f;

    Coin* aCoin = NULL;
    while (mBoard->IterateCoins(aCoin))
    {
        if (aCoin->IsMoney() && aCoin->mCoinMotion != COIN_MOTION_FROM_PRESENT && !aCoin->mIsBeingCollected && aCoin->mCoinAge >= 50)
        {
            float aDistance = Distance2D(mX + mWidth / 2, mY + mHeight / 2, aCoin->mPosX + aCoin->mWidth / 2, aCoin->mPosY + aCoin->mHeight / 2);
            if (aClosestCoin == NULL || aDistance < aClosestDistance)
            {
                aClosestCoin = aCoin;
                aClosestDistance = aDistance;
            }
        }
    }

    return aClosestCoin;
}

void Plant::GoldMagnetFindTargets()
{
    if (GetFreeMagnetItem() == NULL)
    {
        TOD_ASSERT(false);
        return;
    }

    for (;;)
    {
        MagnetItem* aMagnetItem = GetFreeMagnetItem();
        if (aMagnetItem == NULL)
            break;

        Coin* aCoin = FindGoldMagnetTarget();
        if (aCoin == NULL)
            break;

        aMagnetItem->mPosX = aCoin->mPosX + 15.0f;
        aMagnetItem->mPosY = aCoin->mPosY + 15.0f;
        aMagnetItem->mDestOffsetX = RandRangeFloat(20.0f, 40.0f);
        aMagnetItem->mDestOffsetY = RandRangeFloat(-20.0f, 0.0f) + 20.0f;

        switch (aCoin->mType)
        {
        case COIN_SILVER:     aMagnetItem->mItemType = MAGNET_ITEM_SILVER_COIN;   break;
        case COIN_GOLD:       aMagnetItem->mItemType = MAGNET_ITEM_GOLD_COIN;     break;
        case COIN_DIAMOND:    aMagnetItem->mItemType = MAGNET_ITEM_DIAMOND;       break;
        default:              TOD_ASSERT(false);                                        return;
        }

        aCoin->Die();
    }
}

bool Plant::IsAGoldMagnetAboutToSuck()
{
    Plant* aPlant = NULL;
    while (mBoard->IteratePlants(aPlant))
    {
        if (!aPlant->NotOnGround() && aPlant->mSeedType == SEED_GOLD_MAGNET && aPlant->mState == STATE_MAGNETSHROOM_SUCKING)
        {
            Reanimation* aBodyReanim = mApp->ReanimationGet(aPlant->mBodyReanimID);
            if (aBodyReanim->mAnimTime < 0.5f)
            {
                return true;
            }
        }
    }

    return false;
}

void Plant::UpdateGoldMagnetShroom()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    bool aIsSuckingCoin = false;
    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MAGNET_ITEM_NONE)
        {
            SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
            float aDistance = aVectorToPlant.Magnitude();
            if (aDistance < 20.0f)
            {
                CoinType aCoinType;
                switch (aMagnetItem->mItemType)
                {
                case MAGNET_ITEM_SILVER_COIN:   aCoinType = COIN_SILVER;      break;
                case MAGNET_ITEM_GOLD_COIN:     aCoinType = COIN_GOLD;        break;
                case MAGNET_ITEM_DIAMOND:       aCoinType = COIN_DIAMOND;     break;
                default:                        TOD_ASSERT(false);                 return;
                }

                int aValue = Coin::GetCoinValue(aCoinType);
                mApp->mPlayerInfo->AddCoins(aValue);
                mBoard->mCoinsCollected += aValue;
                mApp->PlayFoley(FOLEY_COIN);

                aMagnetItem->mItemType = MAGNET_ITEM_NONE;
            }
            else
            {
                float aSpeed = TodAnimateCurveFloatTime(30.0f, 0.0f, aDistance, 0.02f, 0.05f, CURVE_LINEAR);
                aMagnetItem->mPosX += aVectorToPlant.x * aSpeed;
                aMagnetItem->mPosY += aVectorToPlant.y * aSpeed;

                aIsSuckingCoin = true;
            }
        }
    }

    if (mState == STATE_MAGNETSHROOM_CHARGING)
    {
        if (mStateCountdown == 0)
        {
            mState = STATE_READY;
        }
    }
    else if (mState == STATE_MAGNETSHROOM_SUCKING)
    {
        if (aBodyReanim->ShouldTriggerTimedEvent(0.4f))
        {
            mApp->PlayFoley(FOLEY_MAGNETSHROOM);
            GoldMagnetFindTargets();
        }

        if (aBodyReanim->mLoopCount > 0 && !aIsSuckingCoin)
        {
            PlayIdleAnim(14.0f);
            mState = STATE_MAGNETSHROOM_CHARGING;
            mStateCountdown = RandRangeInt(200, 300);
        }
    }
    else if (!IsAGoldMagnetAboutToSuck() && Rand(50) == 0 && FindGoldMagnetTarget())
    {
        mBoard->ShowCoinBank();
        mState = STATE_MAGNETSHROOM_SUCKING;
        PlayBodyReanim("anim_attract", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
    }
}

void Plant::RemoveEffects()
{
    mApp->RemoveParticle(mParticleID);
    mApp->RemoveReanimation(mBodyReanimID);
    mApp->RemoveReanimation(mHeadReanimID);
    mApp->RemoveReanimation(mHeadReanimID2);
    mApp->RemoveReanimation(mHeadReanimID3);
    mApp->RemoveReanimation(mLightReanimID);
    mApp->RemoveReanimation(mBlinkReanimID);
    mApp->RemoveReanimation(mSleepingReanimID);
}

void Plant::Squish()
{
    if (NotOnGround())
        return;

    if (!mIsAsleep)
    {
        if (mSeedType == SEED_CHERRYBOMB || mSeedType == SEED_JALAPENO ||
            mSeedType == SEED_DOOM_SHROOM || mSeedType == SEED_ICE_SHROOM)
        {
            DoSpecial();
            return;
        }
        else if (mSeedType == SEED_POTATOE_MINE && mState != STATE_NOTREADY)
        {
            DoSpecial();
            return;
        }
    }

    if (mSeedType == SEED_SQUASH && mState != STATE_NOTREADY)
        return;

    mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, mRow, 8);
    mSquished = true;
    mDisappearCountdown = 500;
    mApp->PlayFoley(FOLEY_SQUISH);
    RemoveEffects();

    GridItem* aLadder = mBoard->GetLadderAt(mPlantCol, mRow);
    if (aLadder)
    {
        aLadder->GridItemDie();
    }

    if (mApp->IsIZombieLevel())
    {
        mBoard->mChallenge->IZombiePlantDropRemainingSun(this);
    }
}

void Plant::UpdateBowling()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim && aBodyReanim->TrackExists("_ground"))
    {
        float aSpeed = aBodyReanim->GetTrackVelocity("_ground");
        if (mSeedType == SEED_GIANT_WALLNUT)
        {
            aSpeed *= 2;
        }

        mX -= aSpeed;
        if (mX > 800)
            Die();
    }

    if (mState == STATE_BOWLING_UP)
    {
        mY -= 2;
    }
    else if (mState == STATE_BOWLING_DOWN)
    {
        mY += 2;
    }
    int aDistToGrid = mBoard->GridToPixelY(0, mRow) - mY;
    if (aDistToGrid < -2 || aDistToGrid > 2)
        return;

    PlantState aNewState = mState;
    if (mState == STATE_BOWLING_UP && mRow <= 0)
    {
        aNewState = STATE_BOWLING_DOWN;
    }
    else if (mState == STATE_BOWLING_DOWN && mRow >= 4)
    {
        aNewState = STATE_BOWLING_UP;
    }

    Zombie* aZombie = FindTargetZombie(mRow, WEAPON_PRIMARY);
    if (aZombie)
    {
        int aPosX = mX + mWidth / 2;
        int aPosY = mY + mHeight / 2;

        if (mSeedType == SEED_EXPLODE_O_NUT)
        {
            mApp->PlayFoley(FOLEY_CHERRYBOMB);
            mApp->PlaySample(SOUND_BOWLINGIMPACT2);

            int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY) | 32U;
            mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 90, 1, true, aDamageRangeFlags);
            mApp->AddTodParticle(aPosX, aPosY, RENDER_LAYER_TOP, PARTICLE_POWIE);
            mBoard->ShakeBoard(3, -4);

            Die();

            return;
        }

        mApp->PlayFoley(FOLEY_BOWLINGIMPACT);
        mBoard->ShakeBoard(1, -2);

        if (mSeedType == SEED_GIANT_WALLNUT)
        {
            aZombie->TakeDamage(1800, 0U);
        }
        else if (aZombie->mShieldType == SHIELDTYPE_DOOR && mState != STATE_NOTREADY)
        {
            aZombie->TakeDamage(1800, 0U);
        }
        else if (aZombie->mShieldType != SHIELDTYPE_NONE)
        {
            aZombie->TakeShieldDamage(400, 0U);
        }
        else if (aZombie->mHelmType != HELMTYPE_NONE)
        {
            if (aZombie->mHelmType == HELMTYPE_PAIL)
            {
                mApp->PlayFoley(FOLEY_SHIELD_HIT);
            }
            else if (aZombie->mHelmType == HELMTYPE_TRAFFIC_CONE)
            {
                mApp->PlayFoley(FOLEY_PLASTIC_HIT);
            }

            aZombie->TakeHelmDamage(900, 0U);
        }
        else
        {
            aZombie->TakeDamage(1800, 0U);
        }

        if ((!mApp->IsFirstTimeAdventureMode() || mApp->mPlayerInfo->GetLevel() > 10) && mSeedType == SEED_WALLNUT)
        {
            mLaunchCounter++;
            if (mLaunchCounter == 2)
            {
                mApp->PlayFoley(FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX, aPosY, COIN_SILVER, COIN_MOTION_COIN);
            }
            else if (mLaunchCounter == 3)
            {
                mApp->PlayFoley(FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX - 5.0f, aPosY, COIN_SILVER, COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX + 5.0f, aPosY, COIN_SILVER, COIN_MOTION_COIN);
            }
            else if (mLaunchCounter == 4)
            {
                mApp->PlayFoley(FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX - 10.0f, aPosY, COIN_SILVER, COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX, aPosY, COIN_SILVER, COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX + 10.0f, aPosY, COIN_SILVER, COIN_MOTION_COIN);
            }
            else if (mLaunchCounter >= 5)
            {
                mApp->PlayFoley(FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX, aPosY, COIN_GOLD, COIN_MOTION_COIN);
                ReportAchievement::GiveAchievement(mApp, static_cast<AchievementId>(RollSomeHeads), true);
            }
        }

        if (mSeedType != SEED_GIANT_WALLNUT)
        {
            if (mRow == 4 || mState == STATE_BOWLING_DOWN)
            {
                aNewState = STATE_BOWLING_UP;
            }
            else if (mRow == 0 || mState == STATE_BOWLING_UP)
            {
                aNewState = STATE_BOWLING_DOWN;
            }
            else
            {
                aNewState = Rand(2) ? STATE_BOWLING_UP : STATE_BOWLING_DOWN;
            }
        }
    }

    if (aNewState == STATE_BOWLING_UP)
    {
        mRow--;
        mState = STATE_BOWLING_UP;
        mRenderOrder = CalcRenderOrder();
    }
    else if (aNewState == STATE_BOWLING_DOWN)
    {
        mState = STATE_BOWLING_DOWN;
        mRenderOrder = CalcRenderOrder();
        mRow++;
    }
}

void Plant::UpdateAbilities()
{
    if (!IsInPlay())
        return;

    if (mState == STATE_DOINGSPECIAL || mSquished)
    {
        mDisappearCountdown--;
        if (mDisappearCountdown < 0)
        {
            Die();
            return;
        }
    }

    if (mWakeUpCounter > 0)
    {
        mWakeUpCounter--;
        if (mWakeUpCounter == 60)
        {
            mApp->PlayFoley(FOLEY_WAKEUP);
        }
        if (mWakeUpCounter == 0)
        {
            SetSleeping(false);
        }
    }

    if (mIsAsleep || mSquished || mOnBungeeState != NOT_ON_BUNGEE)
        return;

    UpdateShooting();

    if (mStateCountdown > 0)
        mStateCountdown--;

    if (mApp->IsWallnutBowlingLevel())
    {
        UpdateBowling();
        return;
    }

    if (mSeedType == SEED_SQUASH)                                                        UpdateSquash();
    else if (mSeedType == SEED_DOOM_SHROOM)                                              UpdateDoomShroom();
    else if (mSeedType == SEED_ICE_SHROOM)                                               UpdateIceShroom();
    else if (mSeedType == SEED_CHOMPER)                                                  UpdateChomper();
    else if (mSeedType == SEED_BLOVER)                                                   UpdateBlover();
    else if (mSeedType == SEED_FLOWERPOT)                                                UpdateFlowerPot();
    else if (mSeedType == SEED_LILY_PAD)                                                 UpdateLilypad();
    else if (mSeedType == SEED_IMITATER)                                                 UpdateImitater();
    else if (mSeedType == SEED_INSTANT_COFFEE)                                           UpdateCoffeeBean();
    else if (mSeedType == SEED_UMBRELLA)                                                 UpdateUmbrella();
    else if (mSeedType == SEED_COB_CANNON)                                               UpdateCobCannon();
    else if (mSeedType == SEED_CACTUS)                                                   UpdateCactus();
    else if (mSeedType == SEED_MAGNETSHROOM)                                             UpdateMagnetShroom();
    else if (mSeedType == SEED_GOLD_MAGNET)                                              UpdateGoldMagnetShroom();
    else if (mSeedType == SEED_SUNSHROOM)                                                UpdateSunShroom();
    else if (MakesSun() || mSeedType == SEED_MARIGOLD)                                   UpdateProductionPlant();
    else if (mSeedType == SEED_GRAVE_BUSTER)                                             UpdateGraveBuster();
    else if (mSeedType == SEED_TORCHWOOD)                                                UpdateTorchwood();
    else if (mSeedType == SEED_POTATOE_MINE)                                             UpdatePotato();
    else if (mSeedType == SEED_SPIKEWEED || mSeedType == SEED_SPIKEROCK)                 UpdateSpikeweed();
    else if (mSeedType == SEED_TANGLE_KELP)                                              UpdateTanglekelp();
    else if (mSeedType == SEED_SCAREDY_SHROOM)                                           UpdateScaredyShroom();

    if (mSubclass == SUBCLASS_SHOOTER)
    {
        UpdateShooter();
    }
    if (mDoSpecialCountdown > 0)
    {
        mDoSpecialCountdown--;
        if (mDoSpecialCountdown == 0)
        {
            DoSpecial();
        }
    }
}

bool Plant::IsPartOfUpgradableTo(SeedType theUpgradedType)
{
    if (theUpgradedType == SEED_COB_CANNON && mSeedType == SEED_KERNEL_PULT)
    {
        return mBoard->IsValidCobCannonSpot(mPlantCol, mRow) || mBoard->IsValidCobCannonSpot(mPlantCol - 1, mRow);
    }

    return IsUpgradableTo(theUpgradedType);
}

bool Plant::IsUpgradableTo(SeedType theUpgradedType)
{
    if (theUpgradedType == SEED_UPGRADE_REPEATER && mSeedType == SEED_REPEATER)
    {
        return true;
    }
    if (theUpgradedType == SEED_WINTERMELON && mSeedType == SEED_MELON_PULT)
    {
        return true;
    }
    if (theUpgradedType == SEED_TWINSUNFLOWER && mSeedType == SEED_SUNFLOWER)
    {
        return true;
    }
    if (theUpgradedType == SEED_SPIKEROCK && mSeedType == SEED_SPIKEWEED)
    {
        return true;
    }
    if (theUpgradedType == SEED_COB_CANNON && mSeedType == SEED_KERNEL_PULT)
    {
        return mBoard->IsValidCobCannonSpot(mPlantCol, mRow);
    }
    if (theUpgradedType == SEED_GOLD_MAGNET && mSeedType == SEED_MAGNETSHROOM)
    {
        return true;
    }
    if (theUpgradedType == SEED_GLOOMSHROOM && mSeedType == SEED_FUMESHROOM)
    {
        return true;
    }
    if (theUpgradedType == SEED_CATTAIL && mSeedType == SEED_LILY_PAD)
    {
        Plant* aPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_NORMAL_POSITION);
        return aPlant == NULL || aPlant->mSeedType != SEED_CATTAIL;
    }
    return false;
}

void Plant::UpdateReanimColor()
{
    if (!IsOnBoard())
        return;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == NULL)
        return;

    SeedType aSeedType = mBoard->GetSeedTypeInCursor();
    Color aColorOverride;

    bool isOnGlove = false;
    if (mBoard->mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_GLOVE)
    {
        Plant* aPlant = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mBoard->mCursorObject->mGlovePlantID));
        if (aPlant && aPlant->mPlantCol == mPlantCol && aPlant->mRow == mRow)
        {
            isOnGlove = true;
        }
    }

    if (isOnGlove)
    {
        aColorOverride = Color(128, 128, 128);
    }
    else if (IsPartOfUpgradableTo(aSeedType) && mBoard->CanPlantAt(mPlantCol, mRow, aSeedType) == PLANTING_OK)
    {
        aColorOverride = GetFlashingColor(mBoard->mMainCounter, 90);
    }
    else if (aSeedType == SEED_COB_CANNON && mSeedType == SEED_KERNEL_PULT && mBoard->CanPlantAt(mPlantCol - 1, mRow, aSeedType) == PLANTING_OK)
    {
        aColorOverride = GetFlashingColor(mBoard->mMainCounter, 90);
    }
    else if (mSeedType == SEED_EXPLODE_O_NUT)
    {
        aColorOverride = Color(255, 64, 64);
    }
    else
    {
        aColorOverride = Color(255, 255, 255);
    }

    aBodyReanim->mColorOverride = aColorOverride;

    if (mHighlighted)
    {
        aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, 196);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
        if (mImitaterType == SEED_IMITATER)
        {
            aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, 92);
        }
    }
    else if (mBeghouledFlashCountdown > 0)
    {
        int anAlpha = TodAnimateCurve(50, 0, mBeghouledFlashCountdown % 50, 1, 128, CURVE_BOUNCE);
        aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, anAlpha);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
    }
    else if (mEatenFlashCountdown > 0)
    {
        int aGrayness = ClampInt(mEatenFlashCountdown * 3, 0, mImitaterType == SEED_IMITATER ? 128 : 255);
        aBodyReanim->mExtraAdditiveColor = Color(aGrayness, aGrayness, aGrayness);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
    }
    else
    {
        aBodyReanim->mEnableExtraAdditiveDraw = false;
    }

    if (mBeghouledFlashCountdown > 0)
    {
        int anAlpha = TodAnimateCurve(50, 0, mBeghouledFlashCountdown % 50, 1, 128, CURVE_BOUNCE);
        aBodyReanim->mExtraOverlayColor = Color(255, 255, 255, anAlpha);
        aBodyReanim->mEnableExtraOverlayDraw = true;
    }
    else
    {
        aBodyReanim->mEnableExtraOverlayDraw = false;
    }

    aBodyReanim->PropogateColorToAttachments();
}

bool Plant::IsOnBoard()
{
    if (!mIsOnBoard)
        return false;

    TOD_ASSERT(mBoard);
    return true;
}

bool Plant::IsInPlay()
{
    return IsOnBoard() && mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM;
}

void Plant::UpdateReanim()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == NULL)
        return;

    UpdateReanimColor();

    float aOffsetX = mShakeOffsetX;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
    float aScaleX = 1.0f, aScaleY = 1.0f;
    if ((mApp->mGameMode == GAMEMODE_CHALLENGE_BIG_TIME) &&
        (mSeedType == SEED_WALLNUT || mSeedType == SEED_SUNFLOWER || mSeedType == SEED_MARIGOLD))
    {
        aScaleX = 1.5f;
        aScaleY = 1.5f;
        aOffsetX -= 20.0f;
        aOffsetY -= 40.0f;
    }
    if (mSeedType == SEED_GIANT_WALLNUT)
    {
        aScaleX = 2.0f;
        aScaleY = 2.0f;
        aOffsetX -= 76.0f;
        aOffsetY -= 64.0f;
    }
    if (mSeedType == SEED_INSTANT_COFFEE)
    {
        aScaleX = 0.8f;
        aScaleY = 0.8f;
        aOffsetX += 12.0f;
        aOffsetY += 10.0f;
    }
    if (mSeedType == SEED_POTATOE_MINE)
    {
        aScaleX = 0.8f;
        aScaleY = 0.8f;
        aOffsetX += 12.0f;
        aOffsetY += 12.0f;
    }
    if (mState == STATE_GRAVEBUSTER_EATING)
    {
        aOffsetY += TodAnimateCurveFloat(400, 0, mStateCountdown, 0.0f, 30.0f, CURVE_LINEAR);
    }
    if (mWakeUpCounter > 0)
    {
        float aScaleFactor = TodAnimateCurveFloat(70, 0, mWakeUpCounter, 1.0f, 0.8f, CURVE_EASE_SIN_WAVE);
        aScaleY *= aScaleFactor;
        aOffsetY += 80.0f - 80.0f * aScaleFactor;
    }

    aBodyReanim->Update();

    if (mSeedType == SEED_LEFTPEATER)
    {
        aOffsetX += 80.0f * aScaleX;
        aScaleX *= -1.0f;
    }

    if (mPottedPlantIndex != -1)
    {
        PottedPlant* aPottedPlant = &mApp->mPlayerInfo->mPottedPlant[mPottedPlantIndex];

        if (aPottedPlant->mFacing == FACING_LEFT)
        {
            aOffsetX += 80.0f * aScaleX;
            aScaleX *= -1.0f;
        }

        float aOffsetXStart, aOffsetXEnd;
        float aOffsetYStart, aOffsetYEnd;
        float aScaleStart, aScaleEnd;
        if (aPottedPlant->mPlantAge == PLANTAGE_SMALL)
        {
            aOffsetXStart = 20.0f;
            aOffsetXEnd = 20.0f;
            aOffsetYStart = 40.0f;
            aOffsetYEnd = 40.0f;
            aScaleStart = 0.5f;
            aScaleEnd = 0.5f;
        }
        else if (aPottedPlant->mPlantAge == PLANTAGE_MEDIUM)
        {
            aOffsetXStart = 20.0f;
            aOffsetXEnd = 10.0f;
            aOffsetYStart = 40.0f;
            aOffsetYEnd = 20.0f;
            aScaleStart = 0.5f;
            aScaleEnd = 0.75f;
        }
        else
        {
            aOffsetXStart = 10.0f;
            aOffsetXEnd = 0.0f;
            aOffsetYStart = 20.0f;
            aOffsetYEnd = 0.0f;
            aScaleStart = 0.75f;
            aScaleEnd = 1.0f;
        }

        float aAnimatedOffsetX = TodAnimateCurveFloat(100, 0, mStateCountdown, aOffsetXStart, aOffsetXEnd, CURVE_LINEAR);
        float aAnimatedOffsetY = TodAnimateCurveFloat(100, 0, mStateCountdown, aOffsetYStart, aOffsetYEnd, CURVE_LINEAR);
        float aAnimatedScale = TodAnimateCurveFloat(100, 0, mStateCountdown, aScaleStart, aScaleEnd, CURVE_LINEAR);

        aOffsetX += aAnimatedOffsetX * aScaleX;
        aOffsetY += aAnimatedOffsetY * aScaleY;
        aScaleX *= aAnimatedScale;
        aScaleY *= aAnimatedScale;
        aOffsetX += mApp->mZenGarden->ZenPlantOffsetX(mPottedPlantIndex);
        aOffsetY += mApp->mZenGarden->PlantPottedDrawHeightOffset(mPottedPlantIndex);
    }

    aBodyReanim->SetPosition(aOffsetX, aOffsetY);
    aBodyReanim->OverrideScale(aScaleX, aScaleY);
}

void Plant::Update()
{
    bool doUpdate = false;
    if (IsOnBoard() && mApp->mGameScene == SCENE_LEVEL_INTRO && mApp->IsWallnutBowlingLevel())
        doUpdate = true;
    else if (IsOnBoard() && mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
        doUpdate = true;
    else if (IsOnBoard() && mBoard->mCutScene->ShouldRunUpsellBoard())
        doUpdate = true;
    else if (!IsOnBoard() || mApp->mGameScene == SCENE_PLAYING)
        doUpdate = true;

    if (doUpdate)
    {
        UpdateAbilities();
        Animate();

        if (mPlantHealth < 0)
            Die();

        UpdateReanim();
    }
}

bool Plant::NotOnGround()
{
    if (mSeedType == SEED_SQUASH)
    {
        if (mState == STATE_SQUASH_RISING || mState == STATE_SQUASH_FALLING || mState == STATE_SQUASH_DONE_FALLING)
            return true;
    }

    return mSquished || mOnBungeeState == RISING_WITH_BUNGEE || mDead;
}

Reanimation* Plant::AttachBlinkAnim(Reanimation* theReanimBody)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(mSeedType);
    Reanimation* aAnimToAttach = theReanimBody;
    const char* aTrackToPlay = "anim_blink";
    const char* aTrackToAttach = NULL;

    if (mSeedType == SEED_WALLNUT || mSeedType == SEED_TALLNUT ||
        mSeedType == SEED_EXPLODE_O_NUT || mSeedType == SEED_GIANT_WALLNUT)
    {
        int aHit = Rand(10);
        if (aHit < 1 && theReanimBody->TrackExists("anim_blink_twitch"))
        {
            aTrackToPlay = "anim_blink_twitch";
        }
        else
        {
            aTrackToPlay = aHit < 7 ? "anim_blink_twice" : "anim_blink_thrice";
        }
    }
    else if (mSeedType == SEED_THREEPEATER)
    {
        int aHit = Rand(3);
        if (aHit == 0)
        {
            aTrackToPlay = "anim_blink1";
            aTrackToAttach = "anim_face1";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head1");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else if (aHit == 1)
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head2");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else
        {
            aTrackToPlay = "anim_blink3";
            aTrackToAttach = "anim_face3";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head3");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
    }
    else if (mSeedType == SEED_SPLIT_PEA)
    {
        if (Rand(2) == 0)
        {
            aTrackToPlay = "anim_blink";
            aTrackToAttach = "anim_face";
            aAnimToAttach = mApp->ReanimationTryToGet(mHeadReanimID);
        }
        else
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
            aAnimToAttach = mApp->ReanimationTryToGet(mHeadReanimID2);
        }
    }
    else if (mSeedType == SEED_TWINSUNFLOWER)
    {
        if (Rand(2) == 0)
        {
            aTrackToPlay = "anim_blink";
            aTrackToAttach = "anim_face";
        }
        else
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
        }
    }
    else if (mSeedType == SEED_PEASHOOTER || mSeedType == SEED_SNOW_PEA || mSeedType == SEED_REPEATER || mSeedType == SEED_LEFTPEATER || mSeedType == SEED_UPGRADE_REPEATER)
    {
        if (theReanimBody->TrackExists("anim_stem"))
        {
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_stem");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else if (theReanimBody->TrackExists("anim_idle"))
        {
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_idle");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
    }

    if (aAnimToAttach == NULL)
    {
        TodTrace("Missing head anim");
        return NULL;
    }

    if (!theReanimBody->TrackExists(aTrackToPlay))
        return NULL;

    Reanimation* aBlinkReanim = mApp->mEffectSystem->mReanimationHolder->AllocReanimation(0.0f, 0.0f, 0, aPlantDef.mReanimationType);
    aBlinkReanim->SetFramesForLayer(aTrackToPlay);
    aBlinkReanim->mLoopType = REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD;
    aBlinkReanim->mAnimRate = 15.0f;
    aBlinkReanim->mColorOverride = theReanimBody->mColorOverride;

    if (aTrackToAttach && aAnimToAttach->TrackExists(aTrackToAttach))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, aTrackToAttach);
    }
    else if (aAnimToAttach->TrackExists("anim_face"))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, "anim_face");
    }
    else if (aAnimToAttach->TrackExists("anim_idle"))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, "anim_idle");
    }
    else
    {
        TodTrace("Missing anim_idle for blink");
    }

    aBlinkReanim->mFilterEffect = theReanimBody->mFilterEffect;
    return aBlinkReanim;
}

void Plant::DoBlink()
{
    mBlinkCountdown = 400 + Rand(400);

    if (NotOnGround() || mShootingCounter != 0)
        return;

    if (mSeedType == SEED_POTATOE_MINE && mState != STATE_POTATO_ARMED)
        return;

    if (mState == STATE_CACTUS_RISING || mState == STATE_CACTUS_HIGH || mState == STATE_CACTUS_LOWERING ||
        mState == STATE_MAGNETSHROOM_SUCKING || mState == STATE_MAGNETSHROOM_CHARGING)
        return;

    EndBlink();
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == NULL)
        return;

    if ((mSeedType == SEED_TALLNUT && aBodyReanim->GetImageOverride("anim_idle") == IMAGE_REANIM_TALLNUT_CRACKED2) ||
        (mSeedType == SEED_GARLIC && aBodyReanim->GetImageOverride("anim_face") == IMAGE_REANIM_GARLIC_BODY3))
        return;

    if (mSeedType == SEED_WALLNUT || mSeedType == SEED_TALLNUT ||
        mSeedType == SEED_EXPLODE_O_NUT || mSeedType == SEED_GIANT_WALLNUT)
    {
        mBlinkCountdown = 1000 + Rand(1000);
    }

    Reanimation* aBlinkReanim = AttachBlinkAnim(aBodyReanim);
    if (aBlinkReanim)
    {
        mBlinkReanimID = mApp->ReanimationGetID(aBlinkReanim);
    }
    aBodyReanim->AssignRenderGroupToPrefix("anim_eye", RENDER_GROUP_HIDDEN);
}

void Plant::EndBlink()
{
    if (mBlinkReanimID != REANIMATIONID_NULL)
    {
        mApp->RemoveReanimation(mBlinkReanimID);
        mBlinkReanimID = REANIMATIONID_NULL;

        Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
        if (aBodyReanim)
        {
            aBodyReanim->AssignRenderGroupToPrefix("anim_eye", RENDER_GROUP_NORMAL);
        }
    }
}

void Plant::UpdateBlink()
{
    if (mBlinkReanimID != REANIMATIONID_NULL)
    {
        Reanimation* aBlinkReanim = mApp->ReanimationTryToGet(mBlinkReanimID);
        if (aBlinkReanim == NULL || aBlinkReanim->mLoopCount > 0)
        {
            EndBlink();
        }
    }

    if (mIsAsleep)
        return;

    if (mBlinkCountdown > 0)
    {
        mBlinkCountdown--;
        if (mBlinkCountdown == 0)
        {
            DoBlink();
        }
    }
}

void Plant::AnimateNuts()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == NULL)
        return;

    Image* aCracked1;
    Image* aCracked2;
    const char* aTrackToOverride;
    if (mSeedType == SEED_WALLNUT)
    {
        aCracked1 = IMAGE_REANIM_WALLNUT_CRACKED1;
        aCracked2 = IMAGE_REANIM_WALLNUT_CRACKED2;
        aTrackToOverride = "anim_face";
    }
    else if (mSeedType == SEED_TALLNUT)
    {
        aCracked1 = IMAGE_REANIM_TALLNUT_CRACKED1;
        aCracked2 = IMAGE_REANIM_TALLNUT_CRACKED2;
        aTrackToOverride = "anim_idle";
    }
    else return;

    int aPosX = mX + 40;
    int aPosY = mY + 10;
    if (mSeedType == SEED_TALLNUT)
    {
        aPosY -= 32;
    }

    Image* aImageOverride = aBodyReanim->GetImageOverride(aTrackToOverride);
    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != aCracked2)
        {
            aBodyReanim->SetImageOverride(aTrackToOverride, aCracked2);
            mApp->AddTodParticle(aPosX, aPosY, mRenderOrder + 4, PARTICLE_WALLNUT_EAT_LARGE);
        }
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != aCracked1)
        {
            aBodyReanim->SetImageOverride(aTrackToOverride, aCracked1);
            mApp->AddTodParticle(aPosX, aPosY, mRenderOrder + 4, PARTICLE_WALLNUT_EAT_LARGE);
        }
    }
    else
    {
        aBodyReanim->SetImageOverride(aTrackToOverride, NULL);
    }

    if (IsInPlay() && !mApp->IsIZombieLevel())
    {
        if (mRecentlyEatenCountdown > 0)
        {
            aBodyReanim->mAnimRate = 0.1f;
            return;
        }

        if (aBodyReanim->mAnimRate < 1.0f && mOnBungeeState != RISING_WITH_BUNGEE)
        {
            aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
        }
    }
}

void Plant::AnimateGarlic()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    Image* aImageOverride = aBodyReanim->GetImageOverride("anim_face");

    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != IMAGE_REANIM_GARLIC_BODY3)
        {
            aBodyReanim->SetImageOverride("anim_face", IMAGE_REANIM_GARLIC_BODY3);
            aBodyReanim->AssignRenderGroupToPrefix("Garlic_stem", RENDER_GROUP_HIDDEN);
        }
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != IMAGE_REANIM_GARLIC_BODY2)
        {
            aBodyReanim->SetImageOverride("anim_face", IMAGE_REANIM_GARLIC_BODY2);
        }
    }
    else
    {
        aBodyReanim->SetImageOverride("anim_face", NULL);
    }
}

void Plant::AnimatePumpkin()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    Image* aImageOverride = aBodyReanim->GetImageOverride("Pumpkin_front");

    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != IMAGE_REANIM_PUMPKIN_DAMAGE3)
            aBodyReanim->SetImageOverride("Pumpkin_front", IMAGE_REANIM_PUMPKIN_DAMAGE3);
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != IMAGE_REANIM_PUMPKIN_DAMAGE1)
            aBodyReanim->SetImageOverride("Pumpkin_front", IMAGE_REANIM_PUMPKIN_DAMAGE1);
    }
    else
    {
        aBodyReanim->SetImageOverride("Pumpkin_front", NULL);
    }
}

void Plant::UpdateShooting()
{
    if (NotOnGround() || mShootingCounter == 0)
        return;

    mShootingCounter--;

    if (mSeedType == SEED_FUMESHROOM && mShootingCounter == 15)
    {
        int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, mRow, 0);
        AddAttachedParticle(mX + 85, mY + 31, aRenderPosition, PARTICLE_FUMECLOUD);
    }

    if (mSeedType == SEED_GLOOMSHROOM)
    {
        if (mShootingCounter == 136 || mShootingCounter == 108 || mShootingCounter == 80 || mShootingCounter == 52)
        {
            int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, mRow, 0);
            AddAttachedParticle(mX + 40, mY + 40, aRenderPosition, PARTICLE_GLOOMCLOUD);
        }
        if (mShootingCounter == 126 || mShootingCounter == 98 || mShootingCounter == 70 || mShootingCounter == 42)
        {
            Fire(NULL, mRow, WEAPON_PRIMARY);
        }
    }
    else if (mSeedType == SEED_UPGRADE_REPEATER)
    {
        if (mShootingCounter == 18 || mShootingCounter == 35 || mShootingCounter == 51 || mShootingCounter == 68)
        {
            Fire(NULL, mRow, WEAPON_PRIMARY);
        }
    }
    else if (mSeedType == SEED_CATTAIL)
    {
        if (mShootingCounter == 19)
        {
            Zombie* aZombie = FindTargetZombie(mRow, WEAPON_PRIMARY);
            if (aZombie)
            {
                Fire(aZombie, mRow, WEAPON_PRIMARY);
            }
        }
    }
    else if (mShootingCounter == 1)
    {
        if (mSeedType == SEED_THREEPEATER)
        {
            int rowAbove = mRow - 1;
            int rowBelow = mRow + 1;
            Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
            Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);
            Reanimation* aHeadReanim1 = mApp->ReanimationGet(mHeadReanimID);

            if (aHeadReanim1->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(NULL, rowBelow, WEAPON_PRIMARY);
            }
            if (aHeadReanim2->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(NULL, mRow, WEAPON_PRIMARY);
            }
            if (aHeadReanim3->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(NULL, rowAbove, WEAPON_PRIMARY);
            }
        }
        else if (mSeedType == SEED_SPLIT_PEA)
        {
            Reanimation* aHeadBackReanim = mApp->ReanimationTryToGet(mHeadReanimID2);
            Reanimation* aHeadFrontReanim = mApp->ReanimationTryToGet(mHeadReanimID);
            if (aHeadFrontReanim->mLoopType == REANIM_PLAY_ONCE_AND_HOLD && mLaunchCounter > 25)
            {
                Fire(NULL, mRow, WEAPON_PRIMARY);
            }
            if (aHeadBackReanim->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(NULL, mRow, WEAPON_SECONDARY);
            }
        }
        else if (mState == STATE_CACTUS_LOW)
        {
            Fire(NULL, mRow, WEAPON_SECONDARY);
        }
        else if (mSeedType == SEED_CABBAGE_PULT || mSeedType == SEED_KERNEL_PULT || mSeedType == SEED_MELON_PULT || mSeedType == SEED_WINTERMELON)
        {
            PlantWeapon aPlantWeapon = WEAPON_PRIMARY;
            if (mState == STATE_KERNELPULT_BUTTER)
            {
                Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_HIDDEN);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_kernal", RENDER_GROUP_NORMAL);
                mState = STATE_NOTREADY;
                aPlantWeapon = WEAPON_SECONDARY;
            }

            Zombie* aZombie = FindTargetZombie(mRow, aPlantWeapon);
            Fire(aZombie, mRow, aPlantWeapon);
        }
        else
        {
            Fire(NULL, mRow, WEAPON_PRIMARY);
        }

        return;
    }

    if (mShootingCounter != 0)
        return;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(mHeadReanimID);
    if (mSeedType == SEED_THREEPEATER)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);

        if (aHeadReanim2->mLoopCount > 0)
        {
            if (aHeadReanim->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                aHeadReanim->StartBlend(20);
                aHeadReanim->mLoopType = REANIM_LOOP;
                aHeadReanim->SetFramesForLayer("anim_head_idle1");
                aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
                aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
            }

            aHeadReanim2->StartBlend(20);
            aHeadReanim2->mLoopType = REANIM_LOOP;
            aHeadReanim2->SetFramesForLayer("anim_head_idle2");
            aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim2->mAnimTime = aBodyReanim->mAnimTime;

            if (aHeadReanim3->mLoopType == REANIM_PLAY_ONCE_AND_HOLD)
            {
                aHeadReanim3->StartBlend(20);
                aHeadReanim3->mLoopType = REANIM_LOOP;
                aHeadReanim3->SetFramesForLayer("anim_head_idle3");
                aHeadReanim3->mAnimRate = aBodyReanim->mAnimRate;
                aHeadReanim3->mAnimTime = aBodyReanim->mAnimTime;
            }

            return;
        }
    }
    else if (mSeedType == SEED_SPLIT_PEA)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);

        if (aHeadReanim->mLoopCount > 0)
        {
            aHeadReanim->StartBlend(20);
            aHeadReanim->mLoopType = REANIM_LOOP;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
        }

        if (aHeadReanim2->mLoopCount > 0)
        {
            aHeadReanim2->StartBlend(20);
            aHeadReanim2->mLoopType = REANIM_LOOP;
            aHeadReanim2->SetFramesForLayer("anim_splitpea_idle");
            aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim2->mAnimTime = aBodyReanim->mAnimTime;
        }

        return;
    }
    else if (mState == STATE_CACTUS_HIGH)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_idlehigh", REANIM_LOOP, 20, 0.0f);

            aBodyReanim->mAnimRate = aBodyReanim->mDefinition->mFPS;
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0.0f;
            }

            return;
        }
    }
    else if (aHeadReanim)
    {
        if (aHeadReanim->mLoopCount > 0)
        {
            aHeadReanim->StartBlend(20);
            aHeadReanim->mLoopType = REANIM_LOOP;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
            return;
        }
    }
    else if (mSeedType == SEED_COB_CANNON)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = STATE_COBCANNON_ARMING;
            mStateCountdown = 3000;
            PlayBodyReanim("anim_unarmed_idle", REANIM_LOOP, 20, aBodyReanim->mDefinition->mFPS);
            return;
        }
    }
    else if (aBodyReanim && aBodyReanim->mLoopCount > 0)
    {
        PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
        return;
    }

    mShootingCounter = 1;
}

void Plant::Animate()
{
    if ((mSeedType == SEED_CHERRYBOMB || mSeedType == SEED_JALAPENO) && mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN)
    {
        mShakeOffsetX = RandRangeFloat(-1.0f, 1.0f);
        mShakeOffsetY = RandRangeFloat(-1.0f, 1.0f);
    }

    if (mRecentlyEatenCountdown > 0)
    {
        mRecentlyEatenCountdown--;
    }
    if (mEatenFlashCountdown > 0)
    {
        mEatenFlashCountdown--;
    }
    if (mBeghouledFlashCountdown > 0)
    {
        mBeghouledFlashCountdown--;
    }

    if (mSquished)
    {
        mFrame = 0;
        return;
    }

    if (mSeedType == SEED_WALLNUT || mSeedType == SEED_TALLNUT)
    {
        AnimateNuts();
    }
    else if (mSeedType == SEED_GARLIC)
    {
        AnimateGarlic();
    }
    else if (mSeedType == SEED_PUMPKIN)
    {
        AnimatePumpkin();
    }

    UpdateBlink();

    if (mAnimPing)
    {
        if (mAnimCounter < mFrameLength * mNumFrames - 1)
        {
            mAnimCounter++;
        }
        else
        {
            mAnimPing = false;
            mAnimCounter -= mFrameLength;
        }
    }
    else if (mAnimCounter > 0)
    {
        mAnimCounter--;
    }
    else
    {
        mAnimPing = true;
        mAnimCounter += mFrameLength;
    }
    mFrame = mAnimCounter / mFrameLength;
}

float PlantFlowerPotHeightOffset(SeedType theSeedType, float theFlowerPotScale)
{
    float aHeightOffset = -5.0f * theFlowerPotScale;
    float aScaleOffsetFix = 0.0f;

    switch (theSeedType)
    {
    case SEED_CHOMPER:
    case SEED_PLANTERN:
        aHeightOffset -= 5.0f;
        break;
    case SEED_SCAREDY_SHROOM:
        aHeightOffset += 5.0f;
        aScaleOffsetFix -= 8.0f;
        break;
    case SEED_SUNSHROOM:
    case SEED_PUFFSHROOM:
        aScaleOffsetFix -= 4.0f;
        break;
    case SEED_HYPNOSHROOM:
    case SEED_MAGNETSHROOM:
    case SEED_PEASHOOTER:
    case SEED_REPEATER:
    case SEED_LEFTPEATER:
    case SEED_SNOW_PEA:
    case SEED_THREEPEATER:
    case SEED_SUNFLOWER:
    case SEED_MARIGOLD:
    case SEED_CABBAGE_PULT:
    case SEED_MELON_PULT:
    case SEED_TANGLE_KELP:
    case SEED_BLOVER:
    case SEED_SPIKEWEED:
        aScaleOffsetFix -= 8.0f;
        break;
    case SEED_SEASHROOM:
    case SEED_POTATOE_MINE:
        aScaleOffsetFix -= 4.0f;
        break;
    case SEED_LILY_PAD:
        aScaleOffsetFix -= 16.0f;
        break;
    case SEED_INSTANT_COFFEE:
        aScaleOffsetFix -= 20.0f;
        break;
    default:
        break;
    }

    return aHeightOffset + (theFlowerPotScale * aScaleOffsetFix - aScaleOffsetFix);
}

float PlantDrawHeightOffset(Board* theBoard, Plant* thePlant, SeedType theSeedType, int theCol, int theRow)
{
    float aHeightOffset = 0.0f;

    bool doFloating = false;
    if (Plant::IsFlying(theSeedType))
    {
        doFloating = false;
    }
    else if (theBoard == NULL)
    {
        if (Plant::IsAquatic(theSeedType))
        {
            doFloating = true;
        }
    }
    else if (theBoard->IsPoolSquare(theCol, theRow))
    {
        doFloating = true;
    }
    else if (thePlant != NULL && theBoard->mBackground == BACKGROUND_ZOMBIQUARIUM)
    {
        doFloating = true;
    }

    if (doFloating)
    {
        unsigned int aCounter = theBoard ? theBoard->mMainCounter : gLawnApp->mAppCounter;

        float aPos = theRow * PI + theCol * 0.25f * PI;
        float aTime = static_cast<float>(aCounter % 200) * (2.0f * PI / 200.0f);
        float aFloatingHeight = sin(aPos + aTime) * 2.0f;
        aHeightOffset += aFloatingHeight;
    }

    if (theBoard && (thePlant == NULL || !thePlant->mSquished))
    {
        Plant* aPot = theBoard->GetFlowerPotAt(theCol, theRow);
        if (aPot && !aPot->mSquished && theSeedType != SEED_FLOWERPOT)
        {
            aHeightOffset += PlantFlowerPotHeightOffset(theSeedType, 1.0f);
        }
    }

    if (theSeedType == SEED_FLOWERPOT)
    {
        aHeightOffset += 26.0f;
    }
    else if (theSeedType == SEED_LILY_PAD)
    {
        aHeightOffset += 25.0f;
    }
    else if (theSeedType == SEED_STARFRUIT)
    {
        aHeightOffset += 10.0f;
    }
    else if (theSeedType == SEED_TANGLE_KELP)
    {
        aHeightOffset += 24.0f;
    }
    else if (theSeedType == SEED_SEASHROOM)
    {
        aHeightOffset += 28.0f;
    }
    else if (theSeedType == SEED_INSTANT_COFFEE)
    {
        aHeightOffset -= 20.0f;
    }
    else if (theSeedType == SEED_CACTUS)
    {
        return aHeightOffset;
    }
    else if (theSeedType == SEED_PUMPKIN)
    {
        aHeightOffset += 15.0f;
    }
    else if (theSeedType == SEED_PUFFSHROOM)
    {
        aHeightOffset += 5.0f;
    }
    else if (theSeedType == SEED_SCAREDY_SHROOM)
    {
        aHeightOffset -= 14.0f;
    }
    else if (theSeedType == SEED_GRAVE_BUSTER)
    {
        aHeightOffset -= 40.0f;
    }
    else if (theSeedType == SEED_SPIKEWEED || theSeedType == SEED_SPIKEROCK)
    {
        int aBottomRow = 4;
        if (theBoard && theBoard->StageHas6Rows())
        {
            aBottomRow = 5;
        }

        if (theSeedType == SEED_SPIKEROCK)
        {
            aHeightOffset += 6.0f;
        }

        if (theBoard && theBoard->GetFlowerPotAt(theCol, theRow) && gLawnApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN)
        {
            aHeightOffset += 5.0f;
        }
        else if (theBoard && theBoard->StageHasRoof())
        {
            aHeightOffset += 15.0f;
        }
        else if (theBoard && theBoard->IsPoolSquare(theCol, theRow))
        {
            aHeightOffset += 0.0f;
        }
        else if (theRow == aBottomRow && theCol >= 7 && theBoard->StageHas6Rows())
        {
            aHeightOffset += 1.0f;
        }
        else if (theRow == aBottomRow && theCol < 7)
        {
            aHeightOffset += 12.0f;
        }
        else
        {
            aHeightOffset += 15.0f;
        }
    }

    return aHeightOffset;
}

void Plant::GetPeaHeadOffset(int& theOffsetX, int& theOffsetY)
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);

    int aTrackIndex = 0;
    if (aBodyReanim->TrackExists("anim_stem"))
    {
        aTrackIndex = aBodyReanim->FindTrackIndex("anim_stem");
    }
    else if (aBodyReanim->TrackExists("anim_idle"))
    {
        aTrackIndex = aBodyReanim->FindTrackIndex("anim_idle");
    }

    ReanimatorTransform aTransform;
    aBodyReanim->GetCurrentTransform(aTrackIndex, &aTransform);
    theOffsetX = aTransform.mTransX;
    theOffsetY = aTransform.mTransY;
}

void Plant::DrawMagnetItems(Graphics* g)
{
    float aOffsetX = 0.0f;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);

    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MAGNET_ITEM_NONE)
        {
            int aCelRow = 0, aCelCol = 0;
            Image* aImage = NULL;
            float aScale = 0.8f;

            if (aMagnetItem->mItemType == MAGNET_ITEM_PAIL_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET1;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_PAIL_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET2;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_PAIL_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET3;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_FOOTBALL_HELMET_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_FOOTBALL_HELMET_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_FOOTBALL_HELMET_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_DOOR_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR1;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_DOOR_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR2;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_DOOR_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR3;
            }
            else if (aMagnetItem->mItemType >= MAGNET_ITEM_POGO_1 && aMagnetItem->mItemType <= MAGNET_ITEM_POGO_3)
            {
                aCelCol = static_cast<int>(aMagnetItem->mItemType) - static_cast<int>(MAGNET_ITEM_POGO_1);
                aImage = IMAGE_ZOMBIEPOGO;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_LADDER_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_LADDER_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_LADDER_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_LADDER_PLACED)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_5;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_JACK_IN_THE_BOX)
            {
                aImage = IMAGE_REANIM_ZOMBIE_JACKBOX_BOX;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_PICK_AXE)
            {
                aImage = IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_SILVER_COIN)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_COIN_SILVER_DOLLAR;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_GOLD_COIN)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_COIN_GOLD_DOLLAR;
            }
            else if (aMagnetItem->mItemType == MAGNET_ITEM_DIAMOND)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_DIAMOND;
            }
            else
            {
                TOD_ASSERT(false);
            }

            if (aScale == 1.0f)
            {
                g->DrawImageCel(aImage, aMagnetItem->mPosX - mX + aOffsetX, aMagnetItem->mPosY - mY + aOffsetY, aCelCol, aCelRow);
            }
            else
            {
                TodDrawImageCelScaledF(g, aImage, aMagnetItem->mPosX - mX + aOffsetX, aMagnetItem->mPosY - mY + aOffsetY, aCelCol, aCelRow, aScale, aScale);
            }
        }
    }
}

Image* Plant::GetImage(SeedType theSeedType)
{
    Image** aImages = GetPlantDefinition(theSeedType).mPlantImage;
    return aImages ? aImages[0] : NULL;
}

void Plant::DrawShadow(Graphics* g, float theOffsetX, float theOffsetY)
{
    if (mSeedType == SEED_LILY_PAD || mSeedType == SEED_STARFRUIT || mSeedType == SEED_TANGLE_KELP ||
        mSeedType == SEED_SEASHROOM || mSeedType == SEED_COB_CANNON || mSeedType == SEED_SPIKEWEED ||
        mSeedType == SEED_SPIKEROCK || mSeedType == SEED_GRAVE_BUSTER || mSeedType == SEED_CATTAIL ||
        mOnBungeeState == RISING_WITH_BUNGEE)
        return;

    if (IsOnBoard() && mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mZenGarden->mGardenType == GARDEN_MAIN)
        return;

    int aShadowType = 0;
    float aShadowOffsetX = -3.0f;
    float aShadowOffsetY = 51.0f;
    float aScale = 1.0f;
    if (mBoard && mBoard->StageIsNight())
    {
        aShadowType = 1;
    }

    if (mSeedType == SEED_SQUASH)
    {
        if (mBoard)
        {
            aShadowOffsetY += mBoard->GridToPixelY(mPlantCol, mRow) - mY;
        }
        aShadowOffsetY += 5.0f;
    }
    else if (mSeedType == SEED_PUFFSHROOM)
    {
        aScale = 0.5f;
        aShadowOffsetY = 42.0f;
    }
    else if (mSeedType == SEED_SUNSHROOM)
    {
        aShadowOffsetY = 42.0f;
        if (mState == STATE_SUNSHROOM_SMALL)
        {
            aScale = 0.5f;
        }
        else if (mState == STATE_SUNSHROOM_GROWING)
        {
            Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
            aScale = 0.5f + 0.5f * aBodyReanim->mAnimTime;
        }
    }
    else if (mSeedType == SEED_UMBRELLA)
    {
        aScale = 0.5f;
        aShadowOffsetX = -7.0f;
        aShadowOffsetY = 52.0f;
    }
    else if (mSeedType == SEED_FUMESHROOM || mSeedType == SEED_GLOOMSHROOM)
    {
        aScale = 1.3f;
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SEED_CABBAGE_PULT || mSeedType == SEED_MELON_PULT || mSeedType == SEED_WINTERMELON)
    {
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SEED_KERNEL_PULT)
    {
        aShadowOffsetX = 0.0f;
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SEED_SCAREDY_SHROOM)
    {
        aShadowOffsetX = -9.0f;
        aShadowOffsetY = 55.0f;
    }
    else if (mSeedType == SEED_CHOMPER)
    {
        aShadowOffsetX = -21.0f;
        aShadowOffsetY = 57.0f;
    }
    else if (mSeedType == SEED_FLOWERPOT)
    {
        aShadowOffsetX = -4.0f;
        aShadowOffsetY = 46.0f;
    }
    else if (mSeedType == SEED_TALLNUT)
    {
        aShadowOffsetY = 54.0f;
        aScale = 1.3f;
    }
    else if (mSeedType == SEED_PUMPKIN)
    {
        aShadowOffsetY = 46.0f;
        aScale = 1.4f;
    }
    else if (mSeedType == SEED_CACTUS)
    {
        aShadowOffsetX = -8.0f;
        aShadowOffsetY = 50.0f;
    }
    else if (mSeedType == SEED_PLANTERN)
    {
        aShadowOffsetY = 57.0f;
    }
    else if (mSeedType == SEED_INSTANT_COFFEE)
    {
        aShadowOffsetY = 71.0f;
    }
    else if (mSeedType == SEED_GIANT_WALLNUT)
    {
        aShadowOffsetX = -33.0f;
        aShadowOffsetY = 56.0f;
        aScale = 1.7f;
    }

    if (Plant::IsFlying(mSeedType))
    {
        aShadowOffsetY += 10.0f;
        if (mBoard && (mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_NORMAL_POSITION) || mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_PUMPKIN)))
            return;
    }

    if (aShadowType == 0)
    {
        TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW, theOffsetX + aShadowOffsetX, theOffsetY + aShadowOffsetY, 0, aScale, aScale);
    }
    else
    {
        TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW2, theOffsetX + aShadowOffsetX, theOffsetY + aShadowOffsetY, 0, aScale, aScale);
    }
}

void Plant::Draw(Graphics* g)
{
    float aOffsetX = 0.0f;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
    if (Plant::IsFlying(mSeedType) && mSquished)
    {
        aOffsetY += 30.0f;
    }

    int aImageIndex = mFrame;
    Image* aPlantImage = Plant::GetImage(mSeedType);

    if (mSquished)
    {
        if (mSeedType == SEED_FLOWERPOT)
        {
            aOffsetY -= 15.0f;
        }
        if (mSeedType == SEED_INSTANT_COFFEE)
        {
            aOffsetY -= 20.0f;
        }

        g->SetScale(1.0f, 0.25f, 0.0f, 0.0f);
        DrawSeedType(g, mSeedType, mImitaterType, VARIATION_NORMAL, aOffsetX, 60.0f + aOffsetY);
        g->SetScale(1.0f, 1.0f, 0.0f, 0.0f);
    }
    else
    {
        bool aDrawPumpkinBack = false;
        Plant* aPumpkin = NULL;

        if (IsOnBoard())
        {
            aPumpkin = mBoard->GetPumpkinAt(mPlantCol, mRow);
            if (aPumpkin)
            {
                Plant* aPlantInPumpkin = mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_NORMAL_POSITION);
                if (aPlantInPumpkin)
                {
                    if (aPlantInPumpkin->mRenderOrder > aPumpkin->mRenderOrder || aPlantInPumpkin->mOnBungeeState == GETTING_GRABBED_BY_BUNGEE)
                    {
                        aPlantInPumpkin = NULL;
                    }
                }

                if (aPlantInPumpkin == this)
                {
                    aDrawPumpkinBack = true;
                }
                if (aPlantInPumpkin == NULL && mSeedType == SEED_PUMPKIN)
                {
                    aDrawPumpkinBack = true;
                }
            }
            else if (mSeedType == SEED_PUMPKIN)
            {
                aDrawPumpkinBack = true;
                aPumpkin = this;
            }
        }
        else if (mSeedType == SEED_PUMPKIN)
        {
            aDrawPumpkinBack = true;
            aPumpkin = this;
        }

        DrawShadow(g, aOffsetX, aOffsetY);

        if (Plant::IsFlying(mSeedType))
        {
            unsigned int aCounter = IsOnBoard() ? mBoard->mMainCounter : mApp->mAppCounter;

            float aTime = static_cast<float>(fmod((mRow * 97.0 + mPlantCol * 61.0 + static_cast<double>(aCounter)) * 0.03, 2.0 * PI));
            float aWave = sin(aTime) * 2.0f;
            aOffsetY += aWave;
        }

        if (aDrawPumpkinBack)
        {
            Reanimation* aPumpkinReanim = mApp->ReanimationGet(aPumpkin->mBodyReanimID);
            Graphics aPumpkinGraphics(*g);
            aPumpkinGraphics.mTransX += aPumpkin->mX - mX;
            aPumpkinGraphics.mTransY += aPumpkin->mY - mY;
            aPumpkinReanim->DrawRenderGroup(&aPumpkinGraphics, 1);
        }

        aOffsetX += mShakeOffsetX;
        aOffsetY += mShakeOffsetY;
        if (IsInPlay() && mApp->IsIZombieLevel())
        {
            mBoard->mChallenge->IZombieDrawPlant(g, this);
        }
        else if (mBodyReanimID != REANIMATIONID_NULL)
        {
            Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
            if (aBodyReanim)
            {
                if (!mApp->Is3DAccelerated() && mSeedType == SEED_FLOWERPOT && IsOnBoard() &&
                    aBodyReanim->mAnimRate == 0.0f && aBodyReanim->IsAnimPlaying("anim_idle"))
                {
                    mApp->mReanimatorCache->DrawCachedPlant(g, aOffsetX, aOffsetY, mSeedType, VARIATION_NORMAL);
                }
                else
                {
                    aBodyReanim->Draw(g);
                }
            }
        }
        else
        {
            SeedType aSeedType = SEED_NONE;
            if (mBoard)
            {
                aSeedType = mBoard->GetSeedTypeInCursor();
            }

            if (IsPartOfUpgradableTo(aSeedType) && mBoard->CanPlantAt(mPlantCol, mRow, aSeedType) == PLANTING_OK)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }
            else if (aSeedType == SEED_COB_CANNON && mBoard->CanPlantAt(mPlantCol - 1, mRow, aSeedType) == PLANTING_OK)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }
            else if (mBoard && mBoard->mTutorialState == TUTORIAL_SHOVEL_DIG)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }

            TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
            g->SetColorizeImages(false);
            if (mHighlighted)
            {
                g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
                g->SetColorizeImages(true);
                g->SetColor(Color(255, 255, 255, 196));
                TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
                g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
                g->SetColorizeImages(false);
            }
            else if (mEatenFlashCountdown > 0)
            {
                g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
                g->SetColorizeImages(true);
                g->SetColor(Color(255, 255, 255, ClampInt(mEatenFlashCountdown * 3, 0, 255)));
                TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
                g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
                g->SetColorizeImages(false);
            }
        }

        if (mSeedType == SEED_MAGNETSHROOM && !DrawMagnetItemsOnTop())
        {
            DrawMagnetItems(g);
        }
    }
}

void Plant::DrawSeedType(Graphics* g, SeedType theSeedType, SeedType theImitaterType, DrawVariation theDrawVariation, float thePosX, float thePosY)
{
    Graphics aSeedG(*g);
    int aCelRow = 0;
    int aCelCol = 2;
    float aOffsetX = 0.0f;
    float aOffsetY = 0.0f;
    SeedType aSeedType = theSeedType;
    DrawVariation aDrawVariation = theDrawVariation;

    if (theSeedType == SEED_IMITATER && theImitaterType != SEED_NONE)
    {
        aSeedType = theImitaterType;
        aDrawVariation = VARIATION_IMITATER;
        if (theImitaterType == SEED_HYPNOSHROOM || theImitaterType == SEED_SQUASH || theImitaterType == SEED_POTATOE_MINE ||
            theImitaterType == SEED_GARLIC || theImitaterType == SEED_LILY_PAD)
            aDrawVariation = VARIATION_IMITATER_LESS;
    }
    else if (theDrawVariation == VARIATION_NORMAL && theSeedType == SEED_TANGLE_KELP)
    {
        aDrawVariation = VARIATION_AQUARIUM;
    }

    if (gLawnApp->mGameMode == GAMEMODE_CHALLENGE_BIG_TIME &&
        (aSeedType == SEED_WALLNUT || aSeedType == SEED_SUNFLOWER || aSeedType == SEED_MARIGOLD))
    {
        aSeedG.mScaleX *= 1.5f;
        aSeedG.mScaleY *= 1.5f;
        aOffsetX = -20.0f;
        aOffsetY = -40.0f;
    }
    if (aSeedType == SEED_LEFTPEATER)
    {
        aOffsetX += aSeedG.mScaleX * 80.0f;
        aSeedG.mScaleX *= -1.0f;
    }

    if (Challenge::IsZombieSeedType(aSeedType))
    {
        ZombieType aZombieType = Challenge::IZombieSeedTypeToZombieType(aSeedType);
        if (aZombieType == ZOMBIE_DANCER)
        {
            aSeedG.mScaleX *= 0.8f;
            aSeedG.mScaleY *= 0.8f;
            aOffsetX = 20.0f;
            aOffsetY = 42.0f;
        }
        gLawnApp->mReanimatorCache->DrawCachedZombie(&aSeedG, thePosX + aOffsetX, thePosY + aOffsetY, aZombieType);
    }
    else
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(aSeedType);

        if (aSeedType == SEED_GIANT_WALLNUT)
        {
            aSeedG.mScaleX *= 1.4f;
            aSeedG.mScaleY *= 1.4f;
            TodDrawImageScaledF(&aSeedG, IMAGE_REANIM_WALLNUT_BODY, thePosX - 53.0f, thePosY - 56.0f, aSeedG.mScaleX, aSeedG.mScaleY);
        }
        else if (aPlantDef.mReanimationType != REANIM_NONE)
        {
            gLawnApp->mReanimatorCache->DrawCachedPlant(&aSeedG, thePosX + aOffsetX, thePosY + aOffsetY, aSeedType, aDrawVariation);
        }
        else
        {
            if (aSeedType == SEED_KERNEL_PULT)
            {
                aCelRow = 2;
            }
            else if (aSeedType == SEED_TWINSUNFLOWER)
            {
                aCelRow = 1;
            }

            Image* aPlantImage = Plant::GetImage(aSeedType);
            if (aPlantImage->mNumCols <= 2)
            {
                aCelCol = aPlantImage->mNumCols - 1;
            }

            TodDrawImageCelScaledF(&aSeedG, aPlantImage, thePosX + aOffsetX, thePosY + aOffsetY, aCelCol, aCelRow, aSeedG.mScaleX, aSeedG.mScaleY);
        }
    }
}

void Plant::MouseDown(int x, int y, int theClickCount)
{
    if (theClickCount < 0)
        return;

    if (mState == STATE_COBCANNON_READY)
    {
        mBoard->ClearCursor();
        mBoard->mCursorObject->mType = SEED_NONE;
        mBoard->mCursorObject->mCursorType = CURSOR_TYPE_COBCANNON_TARGET;
        mBoard->mCursorObject->mSeedBankIndex = -1;
        mBoard->mCursorObject->mCoinID = COINID_NULL;
        mBoard->mCursorObject->mCobCannonPlantID = static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(this));
        mBoard->mCobCannonCursorDelayCounter = 30;
        mBoard->mCobCannonMouseX = x;
        mBoard->mCobCannonMouseY = y;
    }
}

void Plant::IceZombies()
{
    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        aZombie->HitIceTrap();
    }

    mBoard->mIceTrapCounter = 300;
    TodParticleSystem* aPoolSparklyParticle = mApp->ParticleTryToGet(mBoard->mPoolSparklyParticleID);
    if (aPoolSparklyParticle)
    {
        aPoolSparklyParticle->mDontUpdate = false;
    }

    Zombie* aBossZombie = mBoard->GetBossZombie();
    if (aBossZombie)
    {
        aBossZombie->BossDestroyFireball();
    }
}

void Plant::BurnRow(int theRow)
{
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mZombieType == ZOMBIE_BOSS || aZombie->mRow == theRow) && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            aZombie->RemoveColdEffects();
            aZombie->ApplyBurn();
        }
    }

    GridItem* aGridItem = NULL;
    while (mBoard->IterateGridItems(aGridItem))
    {
        if (aGridItem->mGridY == theRow && aGridItem->mGridItemType == GRIDITEM_LADDER)
        {
            aGridItem->GridItemDie();
        }
    }

    Zombie* aBossZombie = mBoard->GetBossZombie();
    if (aBossZombie && aBossZombie->mFireballRow == theRow)
    {
        aBossZombie->BossDestroyIceballInRow();
    }
}

void Plant::BlowAwayFliers()
{
    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if (!aZombie->IsDeadOrDying())
        {
            if (aZombie->IsFlying())
            {
                aZombie->mBlowingAway = true;
            }
        }
    }

    mApp->PlaySample(SOUND_BLOVER);
    mBoard->mFogBlownCountDown = 4000;
}

void Plant::KillAllPlantsNearDoom()
{
    Plant* aPlant = NULL;
    while (mBoard->IteratePlants(aPlant))
    {
        if (aPlant->mRow == mRow && aPlant->mPlantCol == mPlantCol)
        {
            aPlant->Die();
        }
    }
}

void Plant::DoSpecial()
{
    int aPosX = mX + mWidth / 2;
    int aPosY = mY + mHeight / 2;
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);

    switch (mSeedType)
    {
    case SEED_BLOVER:
    {
        if (mState != STATE_DOINGSPECIAL)
        {
            mState = STATE_DOINGSPECIAL;
            BlowAwayFliers();
        }
        break;
    }
    case SEED_CHERRYBOMB:
    {
        mApp->PlayFoley(FOLEY_CHERRYBOMB);
        mApp->PlayFoley(FOLEY_JUICY);

        if (mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 115, 1, true, aDamageRangeFlags) >= 10)
            ReportAchievement::GiveAchievement(mApp, static_cast<AchievementId>(Explodonator), true);

        mApp->AddTodParticle(aPosX, aPosY, RENDER_LAYER_TOP, PARTICLE_POWIE);
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SEED_DOOM_SHROOM:
    {
        mApp->PlaySample(SOUND_DOOMSHROOM);

        mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 250, 3, true, aDamageRangeFlags);
        KillAllPlantsNearDoom();

        mApp->AddTodParticle(aPosX, aPosY, RENDER_LAYER_TOP, PARTICLE_DOOM);
        mBoard->AddACrater(mPlantCol, mRow)->mGridItemCounter = 18000;
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SEED_JALAPENO:
    {
        mApp->PlayFoley(FOLEY_JALAPENO_IGNITE);
        mApp->PlayFoley(FOLEY_JUICY);

        mBoard->DoFwoosh(mRow);
        mBoard->ShakeBoard(3, -4);

        BurnRow(mRow);
        mBoard->mIceTimer[mRow] = 20;

        Die();
        break;
    }
    case SEED_UMBRELLA:
    {
        if (mState != STATE_UMBRELLA_TRIGGERED && mState != STATE_UMBRELLA_REFLECTING)
        {
            mState = STATE_UMBRELLA_TRIGGERED;
            mStateCountdown = 5;

            PlayBodyReanim("anim_block", REANIM_PLAY_ONCE_AND_HOLD, 0, 22.0f);
        }

        break;
    }
    case SEED_ICE_SHROOM:
    {
        mApp->PlayFoley(FOLEY_FROZEN);
        IceZombies();
        mApp->AddTodParticle(aPosX, aPosY, RENDER_LAYER_TOP, PARTICLE_ICE_TRAP);

        Die();
        break;
    }
    case SEED_POTATOE_MINE:
    {
        aPosX = mX + mWidth / 2 - 20;
        aPosY = mY + mHeight / 2;

        mApp->PlaySample(SOUND_POTATO_MINE);
        if (mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 60, 0, false, aDamageRangeFlags) >= 1)
            ReportAchievement::GiveAchievement(mApp, static_cast<AchievementId>(Spudow), true);

        int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, mRow, 0);
        mApp->AddTodParticle(aPosX + 20.0f, aPosY, aRenderPosition, PARTICLE_POTATO_MINE);
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SEED_INSTANT_COFFEE:
    {
        Plant* aPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_NORMAL_POSITION);
        if (aPlant && aPlant->mIsAsleep)
        {
            aPlant->mWakeUpCounter = 100;
        }

        mState = STATE_DOINGSPECIAL;
        PlayBodyReanim("anim_crumble", REANIM_PLAY_ONCE_AND_HOLD, 20, 22.0f);
        mApp->PlayFoley(FOLEY_COFFEE);

        break;
    }
    default:
        break;
    }
}

void Plant::ImitaterMorph()
{
    Die();
    Plant* aPlant = mBoard->AddPlant(mPlantCol, mRow, mImitaterType, SEED_IMITATER);

    int aFilter = FILTER_EFFECT_WASHED_OUT;
    if (mImitaterType == SEED_HYPNOSHROOM || mImitaterType == SEED_SQUASH || mImitaterType == SEED_POTATOE_MINE ||
        mImitaterType == SEED_GARLIC || mImitaterType == SEED_LILY_PAD)
        aFilter = FILTER_EFFECT_LESS_WASHED_OUT;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(aPlant->mBodyReanimID);
    if (aBodyReanim)
    {
        aBodyReanim->mFilterEffect = static_cast<int>(aFilter);
    }
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(aPlant->mHeadReanimID);
    if (aHeadReanim)
    {
        aHeadReanim->mFilterEffect = static_cast<int>(aFilter);
    }
    Reanimation* aHeadReanim2 = mApp->ReanimationTryToGet(aPlant->mHeadReanimID2);
    if (aHeadReanim2)
    {
        aHeadReanim2->mFilterEffect = static_cast<int>(aFilter);
    }
    Reanimation* aHeadReanim3 = mApp->ReanimationTryToGet(aPlant->mHeadReanimID3);
    if (aHeadReanim3)
    {
        aHeadReanim3->mFilterEffect = aFilter;
    }
}

void Plant::UpdateImitater()
{
    if (mState != STATE_IMITATER_MORPHING)
    {
        if (mStateCountdown == 0)
        {
            mState = STATE_IMITATER_MORPHING;
            PlayBodyReanim("anim_explode", REANIM_PLAY_ONCE_AND_HOLD, 0, 26.0f);
        }
    }
    else
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.8f))
        {
            mApp->AddTodParticle(mX + 40, mY + 40, RENDER_LAYER_TOP, PARTICLE_IMITATER_MORPH);
        }
        if (aBodyReanim->mLoopCount > 0)
        {
            ImitaterMorph();
        }
    }
}

void Plant::CobCannonFire(int theTargetX, int theTargetY)
{
    TOD_ASSERT(mState == STATE_COBCANNON_READY);

    mState = STATE_COBCANNON_FIRING;
    mShootingCounter = 206;
    PlayBodyReanim("anim_shooting", REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);

    mTargetX = theTargetX - 47.0f;
    mTargetY = theTargetY;

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    ReanimatorTrackInstance* aTrackInstance = aBodyReanim->GetTrackInstanceByName("CobCannon_Cob");
    aTrackInstance->mTrackColor = static_cast<int>(Color::White.ToInt());
}

void Plant::Fire(Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon)
{
    if (mSeedType == SEED_FUMESHROOM)
    {
        DoRowAreaDamage(20, 2U);
        mApp->PlayFoley(FOLEY_FUME);
        return;
    }
    if (mSeedType == SEED_GLOOMSHROOM)
    {
        DoRowAreaDamage(20, 2U);
        return;
    }
    if (mSeedType == SEED_STARFRUIT)
    {
        StarFruitFire();
        return;
    }

    ProjectileType aProjectileType;
    switch (mSeedType)
    {
    case SEED_PEASHOOTER:
    case SEED_REPEATER:
    case SEED_THREEPEATER:
    case SEED_SPLIT_PEA:
    case SEED_UPGRADE_REPEATER:
    case SEED_LEFTPEATER:
        aProjectileType = PROJECTILE_PEA;
        break;
    case SEED_SNOW_PEA:
        aProjectileType = PROJECTILE_SNOWPEA;
        break;
    case SEED_PUFFSHROOM:
    case SEED_SCAREDY_SHROOM:
    case SEED_SEASHROOM:
        aProjectileType = PROJECTILE_PUFF;
        break;
    case SEED_CACTUS:
    case SEED_CATTAIL:
        aProjectileType = PROJECTILE_SPIKE;
        break;
    case SEED_CABBAGE_PULT:
        aProjectileType = PROJECTILE_CABBAGE;
        break;
    case SEED_KERNEL_PULT:
        aProjectileType = PROJECTILE_KERNEL;
        break;
    case SEED_MELON_PULT:
        aProjectileType = PROJECTILE_MELON;
        break;
    case SEED_WINTERMELON:
        aProjectileType = PROJECTILE_WINTERMELON;
        break;
    case SEED_COB_CANNON:
        aProjectileType = PROJECTILE_COBBIG;
        break;
    default:
        TOD_ASSERT(false);
        break;
    }
    if (mSeedType == SEED_KERNEL_PULT && thePlantWeapon == WEAPON_SECONDARY)
    {
        aProjectileType = PROJECTILE_BUTTER;
    }

    mApp->PlayFoley(FOLEY_THROW);
    if (mSeedType == SEED_SNOW_PEA || mSeedType == SEED_WINTERMELON)
    {
        mApp->PlayFoley(FOLEY_SNOW_PEA_SPARKLES);
    }
    else if (mSeedType == SEED_PUFFSHROOM || mSeedType == SEED_SCAREDY_SHROOM || mSeedType == SEED_SEASHROOM)
    {
        mApp->PlayFoley(FOLEY_PUFF);
    }

    int aOriginX, aOriginY;
    if (mSeedType == SEED_PUFFSHROOM)
    {
        aOriginX = mX + 40;
        aOriginY = mY + 40;
    }
    else if (mSeedType == SEED_SEASHROOM)
    {
        aOriginX = mX + 45;
        aOriginY = mY + 63;
    }
    else if (mSeedType == SEED_CABBAGE_PULT)
    {
        aOriginX = mX + 5;
        aOriginY = mY - 12;
    }
    else if (mSeedType == SEED_MELON_PULT || mSeedType == SEED_WINTERMELON)
    {
        aOriginX = mX + 25;
        aOriginY = mY - 46;
    }
    else if (mSeedType == SEED_CATTAIL)
    {
        aOriginX = mX + 20;
        aOriginY = mY - 3;
    }
    else if (mSeedType == SEED_KERNEL_PULT && thePlantWeapon == WEAPON_PRIMARY)
    {
        aOriginX = mX + 19;
        aOriginY = mY - 37;
    }
    else if (mSeedType == SEED_KERNEL_PULT && thePlantWeapon == WEAPON_SECONDARY)
    {
        aOriginX = mX + 12;
        aOriginY = mY - 56;
    }
    else if (mSeedType == SEED_PEASHOOTER || mSeedType == SEED_SNOW_PEA || mSeedType == SEED_REPEATER)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX + 24;
        aOriginY = mY + aOffsetY - 33;
    }
    else if (mSeedType == SEED_LEFTPEATER)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX - 57;
        aOriginY = mY + aOffsetY - 33;
    }
    else if (mSeedType == SEED_UPGRADE_REPEATER)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX + 34;
        aOriginY = mY + aOffsetY - 33;
    }
    else if (mSeedType == SEED_SPLIT_PEA)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginY = mY + aOffsetY - 33;

        if (thePlantWeapon == WEAPON_SECONDARY)
        {
            aOriginX = mX + aOffsetX - 64;
        }
        else
        {
            aOriginX = mX + aOffsetX + 24;
        }
    }
    else if (mSeedType == SEED_THREEPEATER)
    {
        aOriginX = mX + 45;
        aOriginY = mY + 10;
    }
    else if (mSeedType == SEED_SCAREDY_SHROOM)
    {
        aOriginX = mX + 29;
        aOriginY = mY + 21;
    }
    else if (mSeedType == SEED_CACTUS)
    {
        if (thePlantWeapon == WEAPON_PRIMARY)
        {
            aOriginX = mX + 93;
            aOriginY = mY - 50;
        }
        else
        {
            aOriginX = mX + 70;
            aOriginY = mY + 23;
        }
    }
    else if (mSeedType == SEED_COB_CANNON)
    {
        aOriginX = mX - 44;
        aOriginY = mY - 184;
    }
    else
    {
        aOriginX = mX + 10;
        aOriginY = mY + 5;
    }
    if (mBoard->GetFlowerPotAt(mPlantCol, mRow))
    {
        aOriginY -= 5;
    }

    if (mSeedType == SEED_SNOW_PEA)
    {
        int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 8, aOriginY + 13, aRenderPosition, PARTICLE_SNOWPEA_PUFF);
    }
    else if (mSeedType == SEED_PUFFSHROOM)
    {
        int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 18, aOriginY + 13, aRenderPosition, PARTICLE_PUFFSHROOM_MUZZLE);
    }
    else if (mSeedType == SEED_SCAREDY_SHROOM)
    {
        int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 27, aOriginY + 13, aRenderPosition, PARTICLE_PUFFSHROOM_MUZZLE);
    }

    Projectile* aProjectile = mBoard->AddProjectile(aOriginX, aOriginY, mRenderOrder - 1, theRow, aProjectileType);
    aProjectile->mDamageRangeFlags = GetDamageRangeFlags(thePlantWeapon);

    if (mSeedType == SEED_CABBAGE_PULT || mSeedType == SEED_KERNEL_PULT ||
        mSeedType == SEED_MELON_PULT || mSeedType == SEED_WINTERMELON)
    {
        float aRangeX, aRangeY;
        if (theTargetZombie)
        {
            Rect aZombieRect = theTargetZombie->GetZombieRect();
            aRangeX = theTargetZombie->ZombieTargetLeadX(50.0f) - aOriginX - 30.0f;
            aRangeY = aZombieRect.mY - aOriginY;

            if (theTargetZombie->mZombiePhase == PHASE_DOLPHIN_RIDING)
            {
                aRangeX -= 60.0f;
            }
            if (theTargetZombie->mZombieType == ZOMBIE_POGO && theTargetZombie->mHasObject)
            {
                aRangeX -= 60.0f;
            }
            if (theTargetZombie->mZombiePhase == PHASE_SNORKEL_WALKING_IN_POOL)
            {
                aRangeX -= 40.0f;
            }
            if (theTargetZombie->mZombieType == ZOMBIE_BOSS)
            {
                aRangeY = mBoard->GridToPixelY(8, mRow) - aOriginY;
            }
        }
        else
        {
            aRangeX = 700.0f - aOriginX;
            aRangeY = 0.0f;
        }
        if (aRangeX < 40.0f)
        {
            aRangeX = 40.0f;
        }

        aProjectile->mMotionType = MOTION_LOBBED;
        aProjectile->mVelX = aRangeX / 120.0f;
        aProjectile->mVelY = 0.0f;
        aProjectile->mVelZ = aRangeY / 120.0f - 7.0f;
        aProjectile->mAccZ = 0.115f;
    }
    else if (mSeedType == SEED_THREEPEATER)
    {
        if (theRow < mRow)
        {
            aProjectile->mMotionType = MOTION_THREEPEATER;
            aProjectile->mVelY = -3.0f;
            aProjectile->mShadowY += 80.0f;
        }
        else if (theRow > mRow)
        {
            aProjectile->mMotionType = MOTION_THREEPEATER;
            aProjectile->mVelY = 3.0f;
            aProjectile->mShadowY -= 80.0f;
        }
    }
    else if (mSeedType == SEED_PUFFSHROOM || mSeedType == SEED_SEASHROOM)
    {
        aProjectile->mMotionType = MOTION_PUFF;
    }
    else if (mSeedType == SEED_SPLIT_PEA && thePlantWeapon == WEAPON_SECONDARY)
    {
        aProjectile->mMotionType = MOTION_BACKWARDS;
    }
    else if (mSeedType == SEED_LEFTPEATER)
    {
        aProjectile->mMotionType = MOTION_BACKWARDS;
    }
    else if (mSeedType == SEED_CATTAIL)
    {
        aProjectile->mVelX = 2.0f;
        aProjectile->mMotionType = MOTION_HOMING;
        aProjectile->mTargetZombieID = static_cast<ZombieID>(mBoard->ZombieGetID(theTargetZombie));
    }
    else if (mSeedType == SEED_COB_CANNON)
    {
        aProjectile->mVelX = 0.001f;
        aProjectile->mDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
        aProjectile->mMotionType = MOTION_LOBBED;
        aProjectile->mVelY = 0.0f;
        aProjectile->mAccZ = 0.0f;
        aProjectile->mVelZ = -8.0f;
        aProjectile->mCobTargetX = mTargetX - 40;
        aProjectile->mCobTargetRow = mBoard->PixelToGridYKeepOnBoard(mTargetX, mTargetY);
    }
}

Zombie* Plant::FindTargetZombie(int theRow, PlantWeapon thePlantWeapon)
{
    int aDamageRangeFlags = GetDamageRangeFlags(thePlantWeapon);
    Rect aAttackRect = GetPlantAttackRect(thePlantWeapon);
    int aHighestWeight = 0;
    Zombie* aBestZombie = NULL;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        int aRowDeviation = aZombie->mRow - theRow;
        if (aZombie->mZombieType == ZOMBIE_BOSS)
        {
            aRowDeviation = 0;
        }

        if (!aZombie->mHasHead || aZombie->IsTangleKelpTarget())
        {
            if (mSeedType == SEED_POTATOE_MINE || mSeedType == SEED_CHOMPER || mSeedType == SEED_TANGLE_KELP)
            {
                continue;
            }
        }

        bool needPortalCheck = false;
        if (mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
        {
            if (mSeedType == SEED_PEASHOOTER || mSeedType == SEED_CACTUS || mSeedType == SEED_REPEATER)
            {
                needPortalCheck = true;
            }
        }

        if (mSeedType != SEED_CATTAIL)
        {
            if (mSeedType == SEED_GLOOMSHROOM)
            {
                if (aRowDeviation < -1 || aRowDeviation > 1)
                {
                    continue;
                }
            }
            else if (needPortalCheck)
            {
                if (!mBoard->mChallenge->CanTargetZombieWithPortals(this, aZombie))
                {
                    continue;
                }
            }
            else if (aRowDeviation)
            {
                continue;
            }
        }

        if (aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            int aExtraRange = 0;

            if (mSeedType == SEED_CHOMPER)
            {
                if (aZombie->mZombiePhase == PHASE_DIGGER_WALKING)
                {
                    aAttackRect.mX += 20;
                    aAttackRect.mWidth -= 20;
                }

                if (aZombie->mZombiePhase == PHASE_POGO_BOUNCING || (aZombie->mZombieType == ZOMBIE_BUNGEE && aZombie->mTargetCol == mPlantCol))
                {
                    continue;
                }

                if (aZombie->mIsEating || mState == STATE_CHOMPER_BITING)
                {
                    aExtraRange = 60;
                }
            }

            if (mSeedType == SEED_POTATOE_MINE)
            {
                if ((aZombie->mZombieType == ZOMBIE_POGO && aZombie->mHasObject) ||
                    aZombie->mZombiePhase == PHASE_POLEVAULTER_IN_VAULT || aZombie->mZombiePhase == PHASE_POLEVAULTER_PRE_VAULT)
                {
                    continue;
                }

                if (aZombie->mZombieType == ZOMBIE_POLEVAULTER)
                {
                    aAttackRect.mX += 40;
                    aAttackRect.mWidth -= 40;
                }

                if (aZombie->mZombieType == ZOMBIE_BUNGEE && aZombie->mTargetCol != mPlantCol)
                {
                    continue;
                }

                if (aZombie->mIsEating)
                {
                    aExtraRange = 30;
                }
            }

            if ((mSeedType == SEED_EXPLODE_O_NUT && aZombie->mZombiePhase == PHASE_POLEVAULTER_IN_VAULT) ||
                (mSeedType == SEED_TANGLE_KELP && !aZombie->mInPool))
            {
                continue;
            }

            Rect aZombieRect = aZombie->GetZombieRect();
            if (!needPortalCheck && GetRectOverlap(aAttackRect, aZombieRect) < -aExtraRange)
            {
                continue;
            }

            int aWeight = -aZombieRect.mX;
            if (mSeedType == SEED_CATTAIL)
            {
                aWeight = -Distance2D(mX + 40.0f, mY + 40.0f, aZombieRect.mX + aZombieRect.mWidth / 2, aZombieRect.mY + aZombieRect.mHeight / 2);
                if (aZombie->IsFlying())
                {
                    aWeight += 10000;
                }
            }

            if (aBestZombie == NULL || aWeight > aHighestWeight)
            {
                aHighestWeight = aWeight;
                aBestZombie = aZombie;
            }
        }
    }

    return aBestZombie;
}

int Plant::DistanceToClosestZombie()
{
    int aDamageRangeFlags = GetDamageRangeFlags(WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(WEAPON_PRIMARY);
    int aClosestDistance = 1000;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if (aZombie->mRow == mRow && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            int aDistance = -GetRectOverlap(aAttackRect, aZombieRect);
            if (aDistance < aClosestDistance)
            {
                aClosestDistance = std::max(aDistance, 0);
            }
        }
    }

    return aClosestDistance;
}

void Plant::Die()
{
    if (IsOnBoard() && mSeedType == SEED_TANGLE_KELP)
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (aZombie)
        {
            aZombie->DieWithLoot();
        }
    }

    mDead = true;
    RemoveEffects();

    if (!Plant::IsFlying(mSeedType) && IsOnBoard())
    {
        GridItem* aLadder = mBoard->GetLadderAt(mPlantCol, mRow);
        if (aLadder)
        {
            aLadder->GridItemDie();
        }
    }

    if (IsOnBoard())
    {
        Plant* aTopPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_BUNGEE_ORDER);
        Plant* aFlowerPot = mBoard->GetFlowerPotAt(mPlantCol, mRow);
        if (aFlowerPot && aTopPlant == aFlowerPot)
        {
            Reanimation* aPotReanim = mApp->ReanimationGet(aFlowerPot->mBodyReanimID);
            aPotReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
        }
    }
}

PlantDefinition& GetPlantDefinition(SeedType theSeedType)
{
    TOD_ASSERT(gPlantDefs[theSeedType].mSeedType == theSeedType);
    TOD_ASSERT(theSeedType >= 0 && theSeedType < static_cast<int>(NUM_SEED_TYPES));

    return gPlantDefs[theSeedType];
}

int Plant::GetCost(SeedType theSeedType, SeedType theImitaterType)
{
    if (gLawnApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED || gLawnApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
    {
        if (theSeedType == SEED_REPEATER)
        {
            return 1000;
        }
        else if (theSeedType == SEED_FUMESHROOM)
        {
            return 500;
        }
        else if (theSeedType == SEED_TALLNUT)
        {
            return 250;
        }
        else if (theSeedType == SEED_SPROUT)
        {
            return 100;
        }
    }

    switch (theSeedType)
    {
    case SEED_SLOT_MACHINE_SUN:           return 0;
    case SEED_SLOT_MACHINE_DIAMOND:       return 0;
    case SEED_ZOMBIQUARIUM_SNORKLE:       return 100;
    case SEED_ZOMBIQUARIUM_TROPHY:        return 1000;
    case SEED_ZOMBIE_NORMAL:              return 50;
    case SEED_ZOMBIE_TRAFFIC_CONE:        return 75;
    case SEED_ZOMBIE_POLEVAULTER:         return 75;
    case SEED_ZOMBIE_PAIL:                return 125;
    case SEED_ZOMBIE_LADDER:              return 150;
    case SEED_ZOMBIE_DIGGER:              return 125;
    case SEED_ZOMBIE_BUNGEE:              return 125;
    case SEED_ZOMBIE_FOOTBALL:            return 175;
    case SEED_ZOMBIE_BALLOON:             return 150;
    case SEED_ZOMBIE_SCREEN_DOOR:         return 100;
    case SEED_ZOMBONI:                    return 175;
    case SEED_ZOMBIE_POGO:                return 200;
    case SEED_ZOMBIE_DANCER:              return 350;
    case SEED_ZOMBIE_GARGANTUAR:          return 300;
    case SEED_ZOMBIE_IMP:                 return 50;
    default:
    {
        if (theSeedType == SEED_IMITATER && theImitaterType != SEED_NONE)
        {
            const PlantDefinition& aPlantDef = GetPlantDefinition(theImitaterType);
            return aPlantDef.mSeedCost;
        }
        else
        {
            const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
            return aPlantDef.mSeedCost;
        }
    }
    }
}

std::string Plant::GetNameString(SeedType theSeedType, SeedType theImitaterType)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    std::string aName = StrFormat("[%s]", aPlantDef.mPlantName);
    std::string aTranslatedName = TodStringTranslate(aName.c_str());

    if (theSeedType == SEED_IMITATER && theImitaterType != SEED_NONE)
    {
        const PlantDefinition& aImitaterDef = GetPlantDefinition(theImitaterType);
        std::string aImitaterName = StrFormat("[%s]", aImitaterDef.mPlantName);
        std::string aTranslatedImitaterName = TodStringTranslate(aImitaterName.c_str());
        return StrFormat("%s %s", aTranslatedName.c_str(), aTranslatedImitaterName.c_str());
    }

    return aTranslatedName;
}

std::string Plant::GetToolTip(SeedType theSeedType)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    std::string aToolTip = StrFormat("[%s_TOOLTIP]", aPlantDef.mPlantName);
    return TodStringTranslate(aToolTip.c_str());
}

int Plant::GetRefreshTime(SeedType theSeedType, SeedType theImitaterType)
{
    if (Challenge::IsZombieSeedType(theSeedType))
    {
        return 0;
    }

    if (theSeedType == SEED_IMITATER && theImitaterType != SEED_NONE)
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(theImitaterType);
        return aPlantDef.mRefreshTime;
    }
    else
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
        return aPlantDef.mRefreshTime;
    }
}

bool Plant::IsNocturnal(SeedType theSeedtype)
{
    return
        theSeedtype == SEED_PUFFSHROOM ||
        theSeedtype == SEED_SEASHROOM ||
        theSeedtype == SEED_SUNSHROOM ||
        theSeedtype == SEED_FUMESHROOM ||
        theSeedtype == SEED_HYPNOSHROOM ||
        theSeedtype == SEED_DOOM_SHROOM ||
        theSeedtype == SEED_ICE_SHROOM ||
        theSeedtype == SEED_MAGNETSHROOM ||
        theSeedtype == SEED_SCAREDY_SHROOM ||
        theSeedtype == SEED_GLOOMSHROOM;
}

bool Plant::IsFungus(SeedType theSeedtype)
{
    return
        theSeedtype == SEED_PUFFSHROOM ||
        theSeedtype == SEED_SUNSHROOM ||
        theSeedtype == SEED_FUMESHROOM ||
        theSeedtype == SEED_HYPNOSHROOM ||
        theSeedtype == SEED_SCAREDY_SHROOM ||
        theSeedtype == SEED_ICE_SHROOM ||
        theSeedtype == SEED_DOOM_SHROOM ||
        theSeedtype == SEED_SEASHROOM ||
        theSeedtype == SEED_MAGNETSHROOM ||
        theSeedtype == SEED_GLOOMSHROOM;
}

bool Plant::IsAquatic(SeedType theSeedType)
{
    return
        theSeedType == SEED_LILY_PAD ||
        theSeedType == SEED_TANGLE_KELP ||
        theSeedType == SEED_SEASHROOM ||
        theSeedType == SEED_CATTAIL;
}

bool Plant::IsFlying(SeedType theSeedtype)
{
    return theSeedtype == SEED_INSTANT_COFFEE;
}

bool Plant::IsUpgrade(SeedType theSeedtype)
{
    return
        theSeedtype == SEED_UPGRADE_REPEATER ||
        theSeedtype == SEED_WINTERMELON ||
        theSeedtype == SEED_TWINSUNFLOWER ||
        theSeedtype == SEED_SPIKEROCK ||
        theSeedtype == SEED_COB_CANNON ||
        theSeedtype == SEED_GOLD_MAGNET ||
        theSeedtype == SEED_GLOOMSHROOM ||
        theSeedtype == SEED_CATTAIL;
}

Rect Plant::GetPlantRect()
{
    Rect aRect;
    if (mSeedType == SEED_TALLNUT)
    {
        aRect = Rect(mX + 10, mY, mWidth, mHeight);
    }
    else if (mSeedType == SEED_PUMPKIN)
    {
        aRect = Rect(mX, mY, mWidth - 20, mHeight);
    }
    else if (mSeedType == SEED_COB_CANNON)
    {
        aRect = Rect(mX, mY, 140, 80);
    }
    else
    {
        aRect = Rect(mX + 10, mY, mWidth - 20, mHeight);
    }

    return aRect;
}

Rect Plant::GetPlantAttackRect(PlantWeapon thePlantWeapon)
{
    Rect aRect;
    if (mApp->IsWallnutBowlingLevel())
    {
        aRect = Rect(mX, mY, mWidth - 20, mHeight);
    }
    else if (thePlantWeapon == WEAPON_SECONDARY && mSeedType == SEED_SPLIT_PEA)
    {
        aRect = Rect(0, mY, mX + 16, mHeight);
    }
    else switch (mSeedType)
    {
    case SEED_LEFTPEATER:       aRect = Rect(0,             mY,             mX,                 mHeight);               break;
    case SEED_SQUASH:           aRect = Rect(mX + 20,       mY,             mWidth - 35,        mHeight);               break;
    case SEED_CHOMPER:          aRect = Rect(mX + 80,       mY,             40,                 mHeight);               break;
    case SEED_SPIKEWEED:
    case SEED_SPIKEROCK:        aRect = Rect(mX + 20,       mY,             mWidth - 50,        mHeight);               break;
    case SEED_POTATOE_MINE:     aRect = Rect(mX,            mY,             mWidth - 25,        mHeight);               break;
    case SEED_TORCHWOOD:        aRect = Rect(mX + 50,       mY,             30,                 mHeight);               break;
    case SEED_PUFFSHROOM:
    case SEED_SEASHROOM:        aRect = Rect(mX + 60,       mY,             230,                mHeight);               break;
    case SEED_FUMESHROOM:       aRect = Rect(mX + 60,       mY,             340,                mHeight);               break;
    case SEED_GLOOMSHROOM:      aRect = Rect(mX - 80,       mY - 80,        240,                240);                   break;
    case SEED_TANGLE_KELP:      aRect = Rect(mX,            mY,             mWidth,             mHeight);               break;
    case SEED_CATTAIL:          aRect = Rect(-BOARD_WIDTH,  -BOARD_HEIGHT,  BOARD_WIDTH * 2,    BOARD_HEIGHT * 2);      break;
    default:                    aRect = Rect(mX + 60,       mY,             BOARD_WIDTH,        mHeight);               break;
    }

    return aRect;
}

void Plant::PreloadPlantResources(SeedType theSeedType)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    if (aPlantDef.mReanimationType != REANIM_NONE)
    {
        ReanimatorEnsureDefinitionLoaded(aPlantDef.mReanimationType, true);
    }

    if (theSeedType == SEED_CHERRYBOMB)
    {
        ReanimatorEnsureDefinitionLoaded(REANIM_ZOMBIE_CHARRED, true);
    }
    else if (theSeedType == SEED_JALAPENO)
    {
        ReanimatorEnsureDefinitionLoaded(REANIM_JALAPENO_FIRE, true);
    }
    else if (theSeedType == SEED_TORCHWOOD)
    {
        ReanimatorEnsureDefinitionLoaded(REANIM_FIRE_PEA, true);
        ReanimatorEnsureDefinitionLoaded(REANIM_JALAPENO_FIRE, true);
    }
    else if (Plant::IsNocturnal(theSeedType))
    {
        ReanimatorEnsureDefinitionLoaded(REANIM_SLEEPING, true);
    }
}

void Plant::PlayIdleAnim(float theRate)
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim)
    {
        PlayBodyReanim("anim_idle", REANIM_LOOP, 20, theRate);
        if (mApp->IsIZombieLevel())
        {
            aBodyReanim->mAnimRate = 0.0f;
        }
    }
}
