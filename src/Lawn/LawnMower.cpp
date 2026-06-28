/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#include "Board.h"
#include "Cutscene.h"
#include "LawnMower.h"
#include "../LawnApp.h"
#include "../engine/SexyAppBase.h"
#include "System/ReanimationLawn.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Resources.h"

using namespace Sexy;

void LawnMower::LawnMowerInitialize(int theRow)
{
    mApp = (LawnApp*)gSexyAppBase;
    mRow = theRow;
    mPosX = -160.0f;
    mBoard = mApp->mBoard;
    mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_LAWN_MOWER, theRow, 0);
    mPosY = mBoard->GetPosYBasedOnRow(mPosX + 40.0f, theRow) + 23.0f;
    mDead = false;
    mMowerState = MOWER_READY;
    mVisible = true;
    mChompCounter = 0;
    mRollingInCounter = 0;
    mSquishedCounter = 0;
    mLastPortalX = -1;

    ReanimationType aReanimType;
    if (mBoard->StageHasRoof())
    {
        mMowerType = LAWNMOWER_ROOF;
        aReanimType = REANIM_ROOF_CLEANER;
    }
    else if (mBoard->mPlantRow[mRow] == PLANTROW_POOL && mApp->mPlayerInfo->mPurchases[STORE_ITEM_POOL_CLEANER])
    {
        mMowerType = LAWNMOWER_POOL;
        aReanimType = REANIM_POOL_CLEANER;
    }
    else
    {
        mMowerType = LAWNMOWER_LAWN;
        aReanimType = REANIM_LAWNMOWER;
    }

    Reanimation* aMowerReanim = mApp->AddReanimation(0.0f, 18.0f, mRenderOrder, aReanimType);
    aMowerReanim->mAnimRate = 0.0f;
    aMowerReanim->mLoopType = REANIM_LOOP;
    aMowerReanim->mIsAttachment = true;
    aMowerReanim->OverrideScale(0.85f, 0.85f);
    mReanimID = mApp->ReanimationGetID(aMowerReanim);

    if (mMowerType == LAWNMOWER_LAWN)
    {
        aMowerReanim->SetFramesForLayer("anim_normal");
    }
    else if (mMowerType == LAWNMOWER_POOL)
    {
        aMowerReanim->OverrideScale(0.8f, 0.8f);
        aMowerReanim->SetFramesForLayer("anim_land");
        aMowerReanim->SetTruncateDisappearingFrames(NULL, false);
    }

    if (mBoard->mSuperMowerMode && mMowerType == LAWNMOWER_LAWN)
    {
        EnableSuperMower(true);
    }
}

void LawnMower::UpdatePool()
{
    bool isPoolRange = false;
    if (mPosX > 26.0f && mPosX < 660.0f)
    {
        isPoolRange = true;
    }

    Reanimation* aMowerReanim = mApp->ReanimationGet(mReanimID);
    if (isPoolRange && mMowerHeight == MOWER_HEIGHT_LAND)
    {
        Reanimation* aSplashReanim = mApp->AddReanimation(mPosX + 0.0f, mPosY + 25.0f, mRenderOrder + 1, REANIM_SPLASH);
        aSplashReanim->OverrideScale(1.2f, 0.8f);
        mApp->AddTodParticle(mPosX + 0.0f + 50.0f, mPosY + 0.0f + 42.0f, mRenderOrder + 1, PARTICLE_PLANTING_POOL);
        mApp->PlayFoley(FOLEY_ZOMBIESPLASH);
        mMowerHeight = MOWER_HEIGHT_DOWN_TO_POOL;
    }
    else if (mMowerHeight == MOWER_HEIGHT_DOWN_TO_POOL)
    {
        mAltitude -= 2.0f;
        if (mAltitude <= -28.0f)
        {
            mAltitude = 0.0f;
            mMowerHeight = MOWER_HEIGHT_IN_POOL;
            aMowerReanim->PlayReanim("anim_water", REANIM_LOOP, 0, 0.0f);
        }
    }
    else if (mMowerHeight == MOWER_HEIGHT_IN_POOL)
    {
        if (!isPoolRange)
        {
            mAltitude = -28.0f;
            mMowerHeight = MOWER_HEIGHT_UP_TO_LAND;
            Reanimation* aSplashReanim = mApp->AddReanimation(mPosX + 0.0f, mPosY + 25.0f, mRenderOrder + 1, REANIM_SPLASH);
            aSplashReanim->OverrideScale(1.2f, 0.8f);
            mApp->AddTodParticle(mPosX + 0.0f + 50.0f, mPosY + 0.0f + 42.0f, mRenderOrder + 1, PARTICLE_PLANTING_POOL);
            mApp->PlayFoley(FOLEY_PLANT_WATER);
            aMowerReanim->PlayReanim("anim_land", REANIM_LOOP, 0, 0.0f);
        }
    }
    else if (mMowerHeight == MOWER_HEIGHT_UP_TO_LAND)
    {
        mAltitude += 2.0f;
        if (mAltitude >= 0.0f)
        {
            mAltitude = 0.0f;
            mMowerHeight = MOWER_HEIGHT_LAND;
        }
    }

    if (mMowerHeight == MOWER_HEIGHT_IN_POOL && aMowerReanim->mLoopType == REANIM_PLAY_ONCE_AND_HOLD && aMowerReanim->mLoopCount > 0)
    {
        aMowerReanim->PlayReanim("anim_water", REANIM_LOOP, 10, 35.0f);
    }
}

void LawnMower::MowZombie(Zombie* theZombie)
{
    if (mMowerState == MOWER_READY)
    {
        StartMower();
        mChompCounter = 25;
    }
    else if (mMowerState == MOWER_TRIGGERED)
    {
        mChompCounter = 50;
    }

    if (mMowerType == LAWNMOWER_POOL)
    {
        mApp->PlayFoley(FOLEY_SHOOP);
        if (mMowerHeight == MOWER_HEIGHT_IN_POOL)
        {
            Reanimation* aMowerReanim = mApp->ReanimationGet(mReanimID);
            aMowerReanim->PlayReanim("anim_suck", REANIM_PLAY_ONCE_AND_HOLD, 10, 35.0f);
        }
        else
        {
            Reanimation* aMowerReanim = mApp->ReanimationGet(mReanimID);
            aMowerReanim->PlayReanim("anim_landsuck", REANIM_PLAY_ONCE_AND_HOLD, 10, 35.0f);
        }

        theZombie->DieWithLoot();
    }
    else
    {
        mApp->PlayFoley(FOLEY_SPLAT);
        theZombie->MowDown();
    }
}

void LawnMower::Update()
{
    if (mMowerState == MOWER_SQUISHED)
    {
        mSquishedCounter--;
        if (mSquishedCounter <= 0)
        {
            Die();
        }
        return;
    }

    if (mMowerState == MOWER_ROLLING_IN)
    {
        mRollingInCounter++;
        mPosX = TodAnimateCurveFloat(0, 100, mRollingInCounter, -160.0f, -21.0f, CURVE_EASE_IN_OUT);
        if (mRollingInCounter == 100)
        {
            mMowerState = MOWER_READY;
        }
        return;
    }

    if (mApp->mGameScene != SCENE_PLAYING && !mBoard->mCutScene->ShouldRunUpsellBoard())
    {
        return;
    }

    Rect aAttackRect = GetLawnMowerAttackRect();
    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if (aZombie->mZombieType == ZOMBIE_BOSS)
            continue;

        if (aZombie->mRow - mRow)
            continue;

        if (aZombie->mZombieType != ZOMBIE_BOSS &&
            aZombie->mRow - mRow == 0 &&
            aZombie->mZombiePhase != PHASE_ZOMBIE_MOWERED &&
            !aZombie->IsTangleKelpTarget() &&
            aZombie->EffectedByDamage(127U))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            int aOverlap = GetRectOverlap(aAttackRect, aZombieRect);
            if (aOverlap > (aZombie->mZombieType == ZOMBIE_BALLOON ? 20 : 0))
            {
                if (mMowerState != MOWER_READY || (aZombie->mZombieType != ZOMBIE_BUNGEE && aZombie->mHasHead))
                {
                    MowZombie(aZombie);
                }
            }
        }
    }

    if (mMowerState != MOWER_TRIGGERED && mMowerState != MOWER_SQUISHED)
    {
        return;
    }

    float aSpeed = 3.33f;
    if (mMowerType == LAWNMOWER_POOL)
    {
        aSpeed = 2.5f;
    }
    if (mChompCounter > 0)
    {
        mChompCounter--;
        aSpeed = TodAnimateCurveFloat(50, 0, mChompCounter, aSpeed, 1.0f, CURVE_BOUNCE_SLOW_MIDDLE);
    }
    mPosX += aSpeed;
    mPosY = mBoard->GetPosYBasedOnRow(mPosX + 40.0f, mRow) + 23.0f;

    if (mMowerType == LAWNMOWER_POOL)
    {
        UpdatePool();
    }
    if (mMowerType == LAWNMOWER_LAWN && mBoard->mPlantRow[mRow] == PLANTROW_POOL && mPosX > 50.0f)
    {
        Reanimation* aSplashReanim = mApp->AddReanimation(mPosX, mPosY + 25.0f, mRenderOrder + 1, REANIM_SPLASH);
        aSplashReanim->OverrideScale(1.2f, 0.8f);
        mApp->AddTodParticle(mPosX + 50.0f, mPosY + 67.0f, mRenderOrder + 1, PARTICLE_PLANTING_POOL);
        mApp->PlaySample(SOUND_ZOMBIE_ENTERING_WATER);
        mApp->mSoundSystem->StopFoley(FOLEY_LAWNMOWER);
        Die();
    }
    if (mPosX > WIDE_BOARD_WIDTH)
    {
        Die();
    }

    mApp->ReanimationGet(mReanimID)->Update();
}

void LawnMower::Draw(Graphics* g)
{
    if (!mVisible)
        return;

    if (mMowerHeight != MOWER_HEIGHT_UP_TO_LAND &&
        mMowerHeight != MOWER_HEIGHT_DOWN_TO_POOL &&
        mMowerHeight != MOWER_HEIGHT_IN_POOL &&
        mMowerState != MOWER_SQUISHED)
    {
        int aShadowType = 0;
        float aScaleX = 1.0f;
        float aScaleY = 1.0f;
        if (mBoard->StageIsNight())
        {
            aShadowType = 1;
        }

        float aShadowX = mPosX - 7.0f;
        float aShadowY = mPosY - mAltitude + 47.0f;
        if (mMowerType == LAWNMOWER_POOL)
        {
            aShadowX -= 17.0f;
            aShadowY -= 8.0f;
        }
        if (mMowerType == LAWNMOWER_ROOF)
        {
            aShadowX -= 9.0f;
            aShadowY -= 36.0f;
            aScaleY = 1.2f;
            if (mMowerState == MOWER_TRIGGERED)
            {
                aShadowY += 36.0f;
            }
        }

        if (aShadowType == 0)
        {
            TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW, aShadowX, aShadowY, 0, aScaleX, aScaleY);
        }
        else
        {
            TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW2, aShadowX, aShadowY, 0, aScaleX, aScaleY);
        }
    }

    Graphics aMowerGraphics(*g);
    aMowerGraphics.mTransX += mPosX + 6.0f;
    aMowerGraphics.mTransY += mPosY - mAltitude;
    if (mMowerType == LAWNMOWER_POOL)
    {
        if (mMowerState == MOWER_TRIGGERED)
        {
            aMowerGraphics.mTransY -= 7.0f;
            aMowerGraphics.mTransX -= 10.0f;
        }
        else
        {
            aMowerGraphics.mTransY -= 33.0f;
        }

        if (mMowerHeight == MOWER_HEIGHT_UP_TO_LAND || mMowerHeight == MOWER_HEIGHT_DOWN_TO_POOL)
        {
            aMowerGraphics.SetClipRect(Rect(-50, -50, 150, static_cast<int>(132 + mAltitude)));
        }
    }
    else if (mMowerType == LAWNMOWER_ROOF)
    {
        if (mMowerState == MOWER_TRIGGERED)
        {
            aMowerGraphics.mTransY -= 4.0f;
            aMowerGraphics.mTransX -= 10.0f;
        }
        else
        {
            aMowerGraphics.mTransY -= 40.0f;
        }
    }

    if (mMowerState == MOWER_TRIGGERED || mMowerState == MOWER_SQUISHED)
    {
        mApp->ReanimationGet(mReanimID)->Draw(&aMowerGraphics);
    }
    else
    {
        LawnMowerType aMowerType = mMowerType;
        if (mMowerType == LAWNMOWER_LAWN && mBoard->mSuperMowerMode)
        {
            aMowerType = LAWNMOWER_SUPER_MOWER;
        }
        mApp->mReanimatorCache->DrawCachedMower(&aMowerGraphics, 0.0f, 19.0f, aMowerType);
    }
}

void LawnMower::Die()
{
    mDead = true;
    mApp->RemoveReanimation(mReanimID);
    if (mBoard->mBonusLawnMowersRemaining > 0 && !mBoard->HasLevelAwardDropped())
    {
        LawnMower* aLawnMower = mBoard->mLawnMowers.DataArrayAlloc();
        aLawnMower->LawnMowerInitialize(mRow);
        aLawnMower->mMowerState = MOWER_ROLLING_IN;
        mBoard->mBonusLawnMowersRemaining--;
    }
}

void LawnMower::StartMower()
{
    if (mMowerState == MOWER_TRIGGERED)
    {
        return;
    }

    Reanimation* aMowerReanim = mApp->ReanimationGet(mReanimID);
    if (mMowerType == LAWNMOWER_POOL)
    {
        aMowerReanim->mAnimRate = 35.0f;
        mApp->PlayFoley(FOLEY_POOL_CLEANER);
    }
    else
    {
        aMowerReanim->mAnimRate = 70.0f;
        mApp->PlayFoley(FOLEY_LAWNMOWER);
    }

    mBoard->mWaveRowGotLawnMowered[mRow] = mBoard->mCurrentWave;
    mBoard->mTriggeredLawnMowers++;
    mMowerState = MOWER_TRIGGERED;
}

void LawnMower::SquishMower()
{
    Reanimation* aMowerReanim = mApp->ReanimationGet(mReanimID);
    aMowerReanim->OverrideScale(0.85f, 0.22f);
    aMowerReanim->SetPosition(-11.0f, 65.0f);

    mMowerState = MOWER_SQUISHED;
    mSquishedCounter = 500;
    mApp->PlayFoley(FOLEY_SQUISH);
}

Rect LawnMower::GetLawnMowerAttackRect()
{
    return Rect(static_cast<int>(mPosX), static_cast<int>(mPosY), 50, 80);
}

void LawnMower::EnableSuperMower(bool theEnable)
{
    (void)theEnable;
    if (mMowerType == LAWNMOWER_LAWN)
    {
        mApp->ReanimationGet(mReanimID)->SetFramesForLayer("anim_tricked");
    }
}
