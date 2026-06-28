/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#include "Board.h"
#include "Plant.h"
#include "Zombie.h"
#include "Cutscene.h"
#include "Projectile.h"
#include "LawnApp.h"
#include "Resources.h"
#include "GameConstants.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/Attachment.h"
#include "LawnCommon.h"
#include "System/ReportAchievement.h"
#include "Widget/AchievementsScreen.h"

ProjectileDefinition gProjectileDefinition[] = {
    { PROJECTILE_PEA,           0,  20  },
    { PROJECTILE_SNOWPEA,       0,  20  },
    { PROJECTILE_CABBAGE,       0,  40  },
    { PROJECTILE_MELON,         0,  80  },
    { PROJECTILE_PUFF,          0,  20  },
    { PROJECTILE_WINTERMELON,   0,  80  },
    { PROJECTILE_FIREBALL,      0,  40  },
    { PROJECTILE_STAR,          0,  20  },
    { PROJECTILE_SPIKE,         0,  20  },
    { PROJECTILE_BASKETBALL,    0,  75  },
    { PROJECTILE_KERNEL,        0,  20  },
    { PROJECTILE_COBBIG,        0,  300 },
    { PROJECTILE_BUTTER,        0,  40  },
    { PROJECTILE_ZOMBIE_PEA,    0,  20  }
};

Projectile::Projectile()
{
}

Projectile::~Projectile()
{
    AttachmentDie(mAttachmentID);
}

void Projectile::ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType)
{
    int aGridX = mBoard->PixelToGridXKeepOnBoard(theX, theY);
    mProjectileType = theProjectileType;
    mPosX = theX;
    mPosY = theY;
    mPosZ = 0.0f;
    mVelX = 0.0f;
    mVelY = 0.0f;
    mVelZ = 0.0f;
    mAccZ = 0.0f;
    mShadowY = mBoard->GridToPixelY(aGridX, theRow) + 67.0f;
    mHitTorchwoodGridX = -1;
    mMotionType = MOTION_STRAIGHT;
    mFrame = 0;
    mNumFrames = 1;
    mRow = theRow;
    mCobTargetX = 0.0f;
    mDamageRangeFlags = 0;
    mDead = false;
    mAttachmentID = ATTACHMENTID_NULL;
    mCobTargetRow = 0;
    mTargetZombieID = ZOMBIEID_NULL;
    mOnHighGround = mBoard->mGridSquareType[aGridX][theRow] == GRIDSQUARE_HIGH_GROUND;
    if (mBoard->StageHasRoof())
    {
        mShadowY -= 12.0f;
    }
    mRenderOrder = theRenderOrder;
    mRotation = 0.0f;
    mRotationSpeed = 0.0f;
    mWidth = 40;
    mHeight = 40;
    mProjectileAge = 0;
    mClickBackoffCounter = 0;
    mAnimTicksPerFrame = 0;

    switch (mProjectileType)
    {
    case PROJECTILE_CABBAGE:
    case PROJECTILE_BUTTER:
        mRotation = -7 * PI / 25;  // DEG_TO_RAD(-50.4f);
        mRotationSpeed = RandRangeFloat(-0.08f, -0.02f);
        break;
    case PROJECTILE_MELON:
    case PROJECTILE_WINTERMELON:
        mRotation = -2 * PI / 5;  // DEG_TO_RAD(-72.0f);
        mRotationSpeed = RandRangeFloat(-0.08f, -0.02f);
        break;
    case PROJECTILE_KERNEL:
        mRotation = 0.0f;
        mRotationSpeed = RandRangeFloat(-0.2f, -0.08f);
        break;
    case PROJECTILE_SNOWPEA:
    {
        TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + 8.0f, mPosY + 13.0f, 400000, PARTICLE_SNOWPEA_TRAIL);
        AttachParticle(mAttachmentID, aParticle, 8.0f, 13.0f);
        break;
    }
    case PROJECTILE_FIREBALL:
        TOD_ASSERT(false);
        break;
    case PROJECTILE_COBBIG:
        mWidth = IMAGE_REANIM_COBCANNON_COB->GetWidth();
        mHeight = IMAGE_REANIM_COBCANNON_COB->GetHeight();
        mRotation = PI / 2;
        break;
    case PROJECTILE_PUFF:
    {
        TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + 13.0f, mPosY + 13.0f, 400000, PARTICLE_PUFFSHROOM_TRAIL);
        AttachParticle(mAttachmentID, aParticle, 13.0f, 13.0f);
        break;
    }
    case PROJECTILE_BASKETBALL:
        mRotation = RandRangeFloat(0.0f, 2 * PI);
        mRotationSpeed = RandRangeFloat(0.05f, 0.1f);
        break;
    case PROJECTILE_STAR:
        mShadowY += 15.0f;
        mRotationSpeed = RandRangeFloat(0.05f, 0.1f);
        if (Rand(2) == 0)
        {
            mRotationSpeed = -mRotationSpeed;
        }
        break;
    default:
        break;
    }

    mAnimCounter = 0;
    mX = static_cast<int>(mPosX);
    mY = static_cast<int>(mPosY);
}

Plant* Projectile::FindCollisionTargetPlant()
{
    Rect aProjectileRect = GetProjectileRect();

    Plant* aPlant = NULL;
    while (mBoard->IteratePlants(aPlant))
    {
        if (aPlant->mRow != mRow)
            continue;

        if (mProjectileType == PROJECTILE_ZOMBIE_PEA)
        {
            if (aPlant->mSeedType == SEED_PUFFSHROOM ||
                aPlant->mSeedType == SEED_SUNSHROOM ||
                aPlant->mSeedType == SEED_POTATOMINE ||
                aPlant->mSeedType == SEED_SPIKEWEED ||
                aPlant->mSeedType == SEED_SPIKEROCK ||
                aPlant->mSeedType == SEED_LILYPAD)  // Zombie peas can't hit short plants
                continue;
        }

        Rect aPlantRect = aPlant->GetPlantRect();
        if (GetRectOverlap(aProjectileRect, aPlantRect) > 8)
        {
            if (mProjectileType == PROJECTILE_ZOMBIE_PEA)
            {
                return mBoard->GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, TOPPLANT_EATING_ORDER);
            }
            else
            {
                return mBoard->GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, TOPPLANT_CATAPULT_ORDER);
            }
        }
    }

    return NULL;
}

bool Projectile::PeaAboutToHitTorchwood()
{
    if (mMotionType != MOTION_STRAIGHT)
        return false;

    if (mProjectileType != PROJECTILE_PEA && mProjectileType != PROJECTILE_SNOWPEA)
        return false;

    Plant* aPlant = NULL;
    while (mBoard->IteratePlants(aPlant))
    {
        if (aPlant->mSeedType == SEED_TORCHWOOD && aPlant->mRow == mRow && !aPlant->NotOnGround() && mHitTorchwoodGridX != aPlant->mPlantCol)
        {
            Rect aPlantAttackRect = aPlant->GetPlantAttackRect(WEAPON_PRIMARY);
            Rect aProjectileRect = GetProjectileRect();
            aProjectileRect.mX += 40;

            if (GetRectOverlap(aPlantAttackRect, aProjectileRect) > 10)
            {
                return true;
            }
        }
    }

    return false;
}

Zombie* Projectile::FindCollisionTarget()
{
    if (PeaAboutToHitTorchwood())  // The "torchwood jam" mechanic, doesn't exist in either beta version
        return NULL;

    Rect aProjectileRect = GetProjectileRect();
    Zombie* aBestZombie = NULL;
    int aMinX = 0;

    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mZombieType == ZOMBIE_BOSS || aZombie->mRow == mRow) && aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
        {
            if (aZombie->mZombiePhase == PHASE_SNORKEL_WALKING_IN_POOL && mPosZ >= 45.0f)
            {
                continue;
            }

            if (mProjectileType == PROJECTILE_STAR && mProjectileAge < 25 && mVelX >= 0.0f && aZombie->mZombieType == ZOMBIE_DIGGER)
            {
                continue;
            }

            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aProjectileRect, aZombieRect) > 0)
            {
                if (aBestZombie == NULL || aZombie->mX < aMinX)
                {
                    aBestZombie = aZombie;
                    aMinX = aZombie->mX;
                }
            }
        }
    }

    return aBestZombie;
}

void Projectile::CheckForCollision()
{
    if (mMotionType == MOTION_PUFF && mProjectileAge >= 75)
    {
        Die();
        return;
    }

    if (mPosX > WIDE_BOARD_WIDTH || mPosX + mWidth < 0.0f)
    {
        Die();
        return;
    }

    if (mMotionType == MOTION_HOMING)
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (aZombie && aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
        {
            Rect aProjectileRect = GetProjectileRect();
            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aProjectileRect, aZombieRect) >= 0 && mPosY > aZombieRect.mY && mPosY < aZombieRect.mY + aZombieRect.mHeight)
            {
                DoImpact(aZombie);
            }
        }
        return;
    }

    if (mProjectileType == PROJECTILE_STAR && (mPosY > 600.0f || mPosY < 0.0f))
    {
        Die();
        return;
    }

    if ((mProjectileType == PROJECTILE_PEA || mProjectileType == PROJECTILE_STAR) && mShadowY - mPosY > 90.0f)
    {
        return;
    }

    if (mMotionType == MOTION_FLOAT_OVER)
    {
        return;
    }

    if (mProjectileType == PROJECTILE_ZOMBIE_PEA)
    {
        Plant* aPlant = FindCollisionTargetPlant();
        if (aPlant)
        {
            const ProjectileDefinition& aProjectileDef = GetProjectileDef();
            aPlant->mPlantHealth -= aProjectileDef.mDamage;
            aPlant->mEatenFlashCountdown = MAX(aPlant->mEatenFlashCountdown, 25);

            mApp->PlayFoley(FOLEY_SPLAT);
            mApp->AddTodParticle(mPosX - 3.0f, mPosY + 17.0f, mRenderOrder + 1, PARTICLE_PEA_SPLAT);
            Die();
        }
        return;
    }

    Zombie* aZombie = FindCollisionTarget();
    if (aZombie)
    {
        if (aZombie->mOnHighGround && CantHitHighGround())
        {
            return;
        }

        DoImpact(aZombie);
    }
}

bool Projectile::CantHitHighGround()
{
    if (mMotionType == MOTION_BACKWARDS || mMotionType == MOTION_HOMING)
        return false;

    return (
        mProjectileType == PROJECTILE_PEA ||
        mProjectileType == PROJECTILE_SNOWPEA ||
        mProjectileType == PROJECTILE_STAR ||
        mProjectileType == PROJECTILE_PUFF ||
        mProjectileType == PROJECTILE_FIREBALL
        ) && !mOnHighGround;
}

void Projectile::CheckForHighGround()
{
    float aShadowDelta = mShadowY - mPosY;

    if (mProjectileType == PROJECTILE_PEA ||
        mProjectileType == PROJECTILE_SNOWPEA ||
        mProjectileType == PROJECTILE_FIREBALL ||
        mProjectileType == PROJECTILE_SPIKE ||
        mProjectileType == PROJECTILE_COBBIG)
    {
        if (aShadowDelta < 28.0f)
        {
            DoImpact(NULL);
            return;
        }
    }

    if (mProjectileType == PROJECTILE_PUFF && aShadowDelta < 0.0f)
    {
        DoImpact(NULL);
        return;
    }

    if (mProjectileType == PROJECTILE_STAR && aShadowDelta < 23.0f)
    {
        DoImpact(NULL);
        return;
    }

    if (CantHitHighGround())
    {
        int aGridX = mBoard->PixelToGridXKeepOnBoard(mPosX + 30, mPosY);
        if (mBoard->mGridSquareType[aGridX][mRow] == GRIDSQUARE_HIGH_GROUND)
        {
            DoImpact(NULL);
        }
    }
}

bool Projectile::IsSplashDamage(Zombie* theZombie)
{
    if (mProjectileType == PROJECTILE_FIREBALL && theZombie && theZombie->IsFireResistant())
        return false;

    return
        mProjectileType == PROJECTILE_MELON ||
        mProjectileType == PROJECTILE_WINTERMELON ||
        mProjectileType == PROJECTILE_FIREBALL;
}

unsigned int Projectile::GetDamageFlags(Zombie* theZombie)
{
    unsigned int aDamageFlags = 0U;

    if (IsSplashDamage(theZombie))
    {
        SetBit(aDamageFlags, static_cast<int>(DAMAGE_HITS_SHIELD_AND_BODY), true);
    }
    else if (mMotionType == MOTION_LOBBED || mMotionType == MOTION_BACKWARDS)
    {
        SetBit(aDamageFlags, static_cast<int>(DAMAGE_BYPASSES_SHIELD), true);
    }
    else if (mMotionType == MOTION_STAR && mVelX < 0.0f)
    {
        SetBit(aDamageFlags, static_cast<int>(DAMAGE_BYPASSES_SHIELD), true);
    }

    if (mProjectileType == PROJECTILE_SNOWPEA || mProjectileType == PROJECTILE_WINTERMELON)
    {
        SetBit(aDamageFlags, static_cast<int>(DAMAGE_FREEZE), true);
    }

    return aDamageFlags;
}

bool Projectile::IsZombieHitBySplash(Zombie* theZombie)
{
    Rect aProjectileRect = GetProjectileRect();
    if (mProjectileType == PROJECTILE_FIREBALL)
    {
        aProjectileRect.mWidth = 100;
    }

    int aRowDeviation = theZombie->mRow - mRow;
    Rect aZombieRect = theZombie->GetZombieRect();
    if (theZombie->IsFireResistant() && mProjectileType == PROJECTILE_FIREBALL)
    {
        return false;
    }

    if (theZombie->mZombieType == ZOMBIE_BOSS)
    {
        aRowDeviation = 0;
    }
    if (mProjectileType == PROJECTILE_FIREBALL)
    {
        if (aRowDeviation != 0)
        {
            return false;
        }
    }
    else if (aRowDeviation > 1 || aRowDeviation < -1)
    {
        return false;
    }

    return theZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)) && GetRectOverlap(aProjectileRect, aZombieRect) >= 0;
}

void Projectile::DoSplashDamage(Zombie* theZombie)
{
    const ProjectileDefinition& aProjectileDef = GetProjectileDef();

    int aZombiesGetSplashed = 0;
    Zombie* aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if (aZombie != theZombie && IsZombieHitBySplash(aZombie))
        {
            aZombiesGetSplashed++;
        }
    }

    int aOriginalDamage = aProjectileDef.mDamage;
    int aSplashDamage = aProjectileDef.mDamage / 3;
    int aMaxSplashDamageAmount = aOriginalDamage * 7;
    if (mProjectileType == PROJECTILE_FIREBALL)
    {
        aMaxSplashDamageAmount = aOriginalDamage;
    }
    int aSplashDamageAmount = aSplashDamage * aZombiesGetSplashed;
    if (aSplashDamageAmount > aMaxSplashDamageAmount)
    {
        //aSplashDamage *= aMaxSplashDamageAmount / aSplashDamage;
        aSplashDamage = aOriginalDamage * aMaxSplashDamageAmount / (aSplashDamageAmount * 3);
        aSplashDamage = MAX(aSplashDamage, 1);
    }

    aZombie = NULL;
    while (mBoard->IterateZombies(aZombie))
    {
        if (IsZombieHitBySplash(aZombie))
        {
            unsigned int aDamageFlags = GetDamageFlags(aZombie);
            if (aZombie == theZombie)
            {
                aZombie->TakeDamage(aOriginalDamage, aDamageFlags);
            }
            else
            {
                aZombie->TakeDamage(aSplashDamage, aDamageFlags);
            }
        }
    }
}

// GOTY @Patoke: 0x471B41
void Projectile::UpdateLobMotion()
{
    if (mProjectileType == PROJECTILE_COBBIG && mPosZ < -700.0f)
    {
        mVelZ = 8.0f;
        mRow = mCobTargetRow;
        mPosX = mCobTargetX;
        int aCobTargetCol = mBoard->PixelToGridXKeepOnBoard(mCobTargetX, 0);
        mPosY = mBoard->GridToPixelY(aCobTargetCol, mCobTargetRow);
        mShadowY = mPosY + 67.0f;
        mRotation = -PI / 2;
    }

    mVelZ += mAccZ;
    if (mApp->mGameMode == GAMEMODE_CHALLENGE_HIGH_GRAVITY)
    {
        mVelZ += mAccZ;
    }
    mPosX += mVelX;
    mPosY += mVelY;
    mPosZ += mVelZ;

    bool isRising = mVelZ < 0.0f;
    if (isRising && (mProjectileType == PROJECTILE_BASKETBALL || mProjectileType == PROJECTILE_COBBIG))
    {
        return;
    }
    if (mProjectileAge > 20)
    {
        if (isRising)
        {
            return;
        }

        float aMinCollisionZ = 0.0f;
        if (mProjectileType == PROJECTILE_BUTTER)
        {
            aMinCollisionZ = -32.0f;
        }
        else if (mProjectileType == PROJECTILE_BASKETBALL)
        {
            aMinCollisionZ = 60.0f;
        }
        else if (mProjectileType == PROJECTILE_MELON || mProjectileType == PROJECTILE_WINTERMELON)
        {
            aMinCollisionZ = -35.0f;
        }
        else if (mProjectileType == PROJECTILE_CABBAGE || mProjectileType == PROJECTILE_KERNEL)
        {
            aMinCollisionZ = -30.0f;
        }
        else if (mProjectileType == PROJECTILE_COBBIG)
        {
            aMinCollisionZ = -60.0f;
        }
        if (mBoard->mPlantRow[mRow] == PLANTROW_POOL)
        {
            aMinCollisionZ += 40.0f;
        }

        if (mPosZ <= aMinCollisionZ)
        {
            return;
        }
    }

    Plant* aPlant = NULL;
    Zombie* aZombie = NULL;
    if (mProjectileType == PROJECTILE_BASKETBALL || mProjectileType == PROJECTILE_ZOMBIE_PEA)
    {
        aPlant = FindCollisionTargetPlant();
    }
    else
    {
        aZombie = FindCollisionTarget();
    }

    float aGroundZ = 80.0f;
    if (mProjectileType == PROJECTILE_COBBIG)
    {
        aGroundZ = -40.0f;
    }
    bool hitGround = mPosZ > aGroundZ;
    if (aZombie == NULL && aPlant == NULL && !hitGround)
    {
        return;
    }

    if (aPlant)
    {
        Plant* aUmbrellaPlant = mBoard->FindUmbrellaPlant(aPlant->mPlantCol, aPlant->mRow);
        if (aUmbrellaPlant)
        {
            if (aUmbrellaPlant->mState == STATE_UMBRELLA_REFLECTING)
            {
                mApp->PlayFoley(FOLEY_SPLAT);
                int aRenderPosition = Board::MakeRenderOrder(RENDER_LAYER_TOP, 0, 1);
                mApp->AddTodParticle(mPosX + 20.0f, mPosY + 20.0f, aRenderPosition, PARTICLE_UMBRELLA_REFLECT);
                Die();
            }
            else if (aUmbrellaPlant->mState != STATE_UMBRELLA_TRIGGERED)
            {
                mApp->PlayFoley(FOLEY_UMBRELLA);
                aUmbrellaPlant->DoSpecial();
            }
        }
        else
        {
            aPlant->mPlantHealth -= GetProjectileDef().mDamage;
            aPlant->mEatenFlashCountdown = MAX(aPlant->mEatenFlashCountdown, 25);
            mApp->PlayFoley(FOLEY_SPLAT);
            Die();
        }
    }
    else if (mProjectileType == PROJECTILE_COBBIG)
    {
        // @Patoke: implemented
        int aBeforeGargantuarCount = mBoard->GetLiveGargantuarCount();
        mBoard->KillAllZombiesInRadius(mRow, mPosX + 80, mPosY + 40, 115, 1, true, mDamageRangeFlags);
        int aAfterGargantuarCount = mBoard->GetLiveGargantuarCount();
        mBoard->mGargantuarsKillsByCornCob += aBeforeGargantuarCount - aAfterGargantuarCount;
        if (mBoard->mGargantuarsKillsByCornCob >= 2)
            ReportAchievement::GiveAchievement(mApp, PopcornParty, true);

        DoImpact(NULL);
    }
    else
    {
        DoImpact(aZombie);
    }
}

void Projectile::UpdateNormalMotion()
{
    if (mMotionType == MOTION_BACKWARDS)
    {
        mPosX -= 3.33f;
    }
    else if (mMotionType == MOTION_HOMING)
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (aZombie && aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            SexyVector2 aTargetCenter(aZombie->ZombieTargetLeadX(0.0f), aZombieRect.mY + aZombieRect.mHeight / 2);
            SexyVector2 aProjectileCenter(mPosX + mWidth / 2, mPosY + mHeight / 2);
            SexyVector2 aToTarget = (aTargetCenter - aProjectileCenter).Normalize();
            SexyVector2 aMotion(mVelX, mVelY);

            aMotion += aToTarget * (0.001f * mProjectileAge);
            aMotion = aMotion.Normalize();
            aMotion *= 2.0f;

            mVelX = aMotion.x;
            mVelY = aMotion.y;
            mRotation = -atan2(mVelY, mVelX);
        }

        mPosY += mVelY;
        mPosX += mVelX;
        mShadowY += mVelY;
        mRow = mBoard->PixelToGridYKeepOnBoard(mPosX, mPosY);
    }
    else if (mMotionType == MOTION_STAR)
    {
        mPosY += mVelY;
        mPosX += mVelX;
        mShadowY += mVelY;

        if (mVelY != 0.0f)
        {
            mRow = mBoard->PixelToGridYKeepOnBoard(mPosX, mPosY);
        }
    }
    else if (mMotionType == MOTION_BEE)
    {
        if (mProjectileAge < 60)
        {
            mPosY -= 0.5f;
        }
        mPosX += 3.33f;
    }
    else if (mMotionType == MOTION_FLOAT_OVER)
    {
        if (mVelZ < 0.0f)
        {
            mVelZ += 0.002f;
            mVelZ = MIN(mVelZ, 0.0f);
            mPosY += mVelZ;
            mRotation = 0.3f - 0.7f * mVelZ * PI * 0.25f;
        }
        mPosX += 0.4f;
    }
    else if (mMotionType == MOTION_BEE_BACKWARDS)
    {
        if (mProjectileAge < 60)
        {
            mPosY -= 0.5f;
        }
        mPosX -= 3.33f;
    }
    else if (mMotionType == MOTION_THREEPEATER)
    {
        mPosX += 3.33f;
        mPosY += mVelY;
        mVelY *= 0.97f;
        mShadowY += mVelY;
    }
    else
    {
        mPosX += 3.33f;
    }

    if (mApp->mGameMode == GAMEMODE_CHALLENGE_HIGH_GRAVITY)
    {
        if (mMotionType == MOTION_FLOAT_OVER)
        {
            mVelZ += 0.004f;
        }
        else
        {
            mVelZ += 0.2f;
        }

        mPosY += mVelZ;
    }

    CheckForCollision();
    CheckForHighGround();
}

void Projectile::UpdateMotion()
{
    if (mAnimTicksPerFrame > 0)
    {
        mAnimCounter = (mAnimCounter + 1) % (mNumFrames * mAnimTicksPerFrame);
        mFrame = mAnimCounter / mAnimTicksPerFrame;
    }

    int aOldRow = mRow;
    float aOldY = mBoard->GetPosYBasedOnRow(mPosX, mRow);
    if (mMotionType == MOTION_LOBBED)
    {
        UpdateLobMotion();
    }
    else
    {
        UpdateNormalMotion();
    }

    float aSlopeHeightChange = mBoard->GetPosYBasedOnRow(mPosX, aOldRow) - aOldY;
    if (mProjectileType == PROJECTILE_COBBIG)
    {
        aSlopeHeightChange = 0.0f;  // Fix The Roof Offset Bug of Corn Cob
    }
    if (mMotionType == MOTION_FLOAT_OVER)
    {
        mPosY += aSlopeHeightChange;
    }
    if (mMotionType == MOTION_LOBBED)
    {
        mPosY += aSlopeHeightChange;
        mPosZ -= aSlopeHeightChange;
    }
    mShadowY += aSlopeHeightChange;
    mX = static_cast<int>(mPosX);
    mY = static_cast<int>(mPosY + mPosZ);
}

void Projectile::PlayImpactSound(Zombie* theZombie)
{
    bool aPlayHelmSound = true;
    bool aPlaySplatSound = true;
    if (mProjectileType == PROJECTILE_KERNEL)
    {
        mApp->PlayFoley(FOLEY_KERNEL_SPLAT);
        aPlayHelmSound = false;
        aPlaySplatSound = false;
    }
    else if (mProjectileType == PROJECTILE_BUTTER)
    {
        mApp->PlayFoley(FOLEY_BUTTER);
        aPlaySplatSound = false;
    }
    else if (mProjectileType == PROJECTILE_FIREBALL && IsSplashDamage(theZombie))
    {
        mApp->PlayFoley(FOLEY_IGNITE);
        aPlayHelmSound = false;
        aPlaySplatSound = false;
    }
    else if (mProjectileType == PROJECTILE_MELON || mProjectileType == PROJECTILE_WINTERMELON)
    {
        mApp->PlayFoley(FOLEY_MELONIMPACT);
        aPlaySplatSound = false;
    }

    if (aPlayHelmSound && theZombie)
    {
        if (theZombie->mHelmType == HELMTYPE_PAIL)
        {
            mApp->PlayFoley(FOLEY_SHIELD_HIT);
            aPlaySplatSound = false;
        }
        else if (theZombie->mHelmType == HELMTYPE_TRAFFIC_CONE || theZombie->mHelmType == HELMTYPE_DIGGER || theZombie->mHelmType == HELMTYPE_FOOTBALL)
        {
            mApp->PlayFoley(FOLEY_PLASTIC_HIT);
        }
    }

    if (aPlaySplatSound)
    {
        mApp->PlayFoley(FOLEY_SPLAT);
    }
}

void Projectile::DoImpact(Zombie* theZombie)
{
    PlayImpactSound(theZombie);

    if (IsSplashDamage(theZombie))
    {
        if (mProjectileType == PROJECTILE_FIREBALL && theZombie)
        {
            theZombie->RemoveColdEffects();
        }

        DoSplashDamage(theZombie);
    }
    else if (theZombie)
    {
        unsigned int aDamageFlags = GetDamageFlags(theZombie);
        theZombie->TakeDamage(GetProjectileDef().mDamage, aDamageFlags);
    }

    float aLastPosX = mPosX - mVelX;
    float aLastPosY = mPosY + mPosZ - mVelY - mVelZ;
    ParticleEffect aEffect = PARTICLE_NONE;
    float aSplatPosX = mPosX + 12.0f;
    float aSplatPosY = mPosY + 12.0f;
    switch (mProjectileType)
    {
    case PROJECTILE_MELON:
        mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, PARTICLE_MELONSPLASH);
        break;
    case PROJECTILE_WINTERMELON:
        mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, PARTICLE_WINTERMELON);
        break;
    case PROJECTILE_COBBIG:
    {
        int aRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GROUND, mCobTargetRow, 2);
        mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, aRenderOrder, PARTICLE_BLASTMARK);
        mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, mRenderOrder + 1, PARTICLE_POPCORNSPLASH);
        mApp->PlaySample(SOUND_DOOMSHROOM);
        mBoard->ShakeBoard(3, -4);
        break;
    }
    case PROJECTILE_PEA:
        aSplatPosX -= 15.0f;
        aEffect = PARTICLE_PEA_SPLAT;
        break;
    case PROJECTILE_SNOWPEA:
        aSplatPosX -= 15.0f;
        aEffect = PARTICLE_SNOWPEA_SPLAT;
        break;
    case PROJECTILE_FIREBALL:
    {
        if (IsSplashDamage(theZombie))
        {
            Reanimation* aFireReanim = mApp->AddReanimation(mPosX + 38.0f, mPosY - 20.0f, mRenderOrder + 1, REANIM_JALAPENO_FIRE);
            aFireReanim->mAnimTime = 0.25f;
            aFireReanim->mAnimRate = 24.0f;
            aFireReanim->OverrideScale(0.7f, 0.4f);
        }
        break;
    }
    case PROJECTILE_STAR:
        aEffect = PARTICLE_STAR_SPLAT;
        break;
    case PROJECTILE_PUFF:
        aSplatPosX -= 20.0f;
        aEffect = PARTICLE_PUFF_SPLAT;
        break;
    case PROJECTILE_CABBAGE:
        aSplatPosX = aLastPosX - 38.0f;
        aSplatPosY = aLastPosY + 23.0f;
        aEffect = PARTICLE_CABBAGE_SPLAT;
        break;
    case PROJECTILE_BUTTER:
        aSplatPosX = aLastPosX - 20.0f;
        aSplatPosY = aLastPosY + 63.0f;
        aEffect = PARTICLE_BUTTER_SPLAT;

        if (theZombie)
        {
            theZombie->ApplyButter();
        }
        break;
    default:
        break;
    }

    if (aEffect != PARTICLE_NONE)
    {
        if (theZombie)
        {
            float aPosX = aSplatPosX + 52.0f - theZombie->mX;
            float aPosY = aSplatPosY - theZombie->mY;
            if (theZombie->mZombiePhase == PHASE_SNORKEL_WALKING_IN_POOL || theZombie->mZombiePhase == PHASE_DOLPHIN_WALKING_IN_POOL)
            {
                aPosY += 60.0f;
            }
            if (mMotionType == MOTION_BACKWARDS)
            {
                aPosX -= 80.0f;
            }
            else if (mPosX > theZombie->mX + 40 && mMotionType != MOTION_LOBBED)
            {
                aPosX -= 60.0f;
            }

            aPosY = ClampFloat(aPosY, 20.0f, 100.0f);
            theZombie->AddAttachedParticle(aPosX, aPosY, aEffect);
        }
        else
        {
            mApp->AddTodParticle(aSplatPosX, aSplatPosY, mRenderOrder + 1, aEffect);
        }
    }

    Die();
}

void Projectile::Update()
{
    mProjectileAge++;
    if (mApp->mGameScene != SCENE_PLAYING && !mBoard->mCutScene->ShouldRunUpsellBoard())
        return;

    int aTime = 20;
    if (mProjectileType == PROJECTILE_PEA ||
        mProjectileType == PROJECTILE_SNOWPEA ||
        mProjectileType == PROJECTILE_CABBAGE ||
        mProjectileType == PROJECTILE_MELON ||
        mProjectileType == PROJECTILE_WINTERMELON ||
        mProjectileType == PROJECTILE_KERNEL ||
        mProjectileType == PROJECTILE_BUTTER ||
        mProjectileType == PROJECTILE_COBBIG ||
        mProjectileType == PROJECTILE_ZOMBIE_PEA ||
        mProjectileType == PROJECTILE_SPIKE)
    {
        aTime = 0;
    }
    if (mProjectileAge > aTime)
    {
        mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PROJECTILE, mRow, 0);
    }

    if (mClickBackoffCounter > 0)
    {
        mClickBackoffCounter--;
    }
    mRotation += mRotationSpeed;

    UpdateMotion();
    AttachmentUpdateAndMove(mAttachmentID, mPosX, mPosY + mPosZ);
}

void Projectile::Draw(Graphics* g)
{
    const ProjectileDefinition& aProjectileDef = GetProjectileDef();

    Image* aImage = NULL;
    float aScale = 1.0f;
    switch (mProjectileType)
    {
    case PROJECTILE_COBBIG:
        aImage = IMAGE_REANIM_COBCANNON_COB;
        aScale = 0.9f;
        break;
    case PROJECTILE_PEA:
    case PROJECTILE_ZOMBIE_PEA:
        aImage = IMAGE_PROJECTILEPEA;
        break;
    case PROJECTILE_SNOWPEA:
        aImage = IMAGE_PROJECTILESNOWPEA;
        break;
    case PROJECTILE_FIREBALL:
        aImage = NULL;
        break;
    case PROJECTILE_SPIKE:
        aImage = IMAGE_PROJECTILECACTUS;
        break;
    case PROJECTILE_STAR:
        aImage = IMAGE_PROJECTILE_STAR;
        break;
    case PROJECTILE_PUFF:
        aImage = IMAGE_PUFFSHROOM_PUFF1;
        aScale = TodAnimateCurveFloat(0, 30, mProjectileAge, 0.3f, 1.0f, CURVE_LINEAR);
        break;
    case PROJECTILE_BASKETBALL:
        aImage = IMAGE_REANIM_ZOMBIE_CATAPULT_BASKETBALL;
        aScale = 1.1f;
        break;
    case PROJECTILE_CABBAGE:
        aImage = IMAGE_REANIM_CABBAGEPULT_CABBAGE;
        aScale = 1.0f;
        break;
    case PROJECTILE_KERNEL:
        aImage = IMAGE_REANIM_CORNPULT_KERNAL;
        aScale = 0.95f;
        break;
    case PROJECTILE_BUTTER:
        aImage = IMAGE_REANIM_CORNPULT_BUTTER;
        aScale = 0.8f;
        break;
    case PROJECTILE_MELON:
        aImage = IMAGE_REANIM_MELONPULT_MELON;
        aScale = 1.0f;
        break;
    case PROJECTILE_WINTERMELON:
        aImage = IMAGE_REANIM_WINTERMELON_PROJECTILE;
        aScale = 1.0f;
        break;
    default:
        TOD_ASSERT(false);
        break;
    }

    bool aMirror = false;
    if (mMotionType == MOTION_BEE_BACKWARDS)
    {
        aMirror = true;
    }

    if (aImage)
    {
        TOD_ASSERT(aProjectileDef.mImageRow < aImage->mNumRows);
        TOD_ASSERT(mFrame < aImage->mNumCols);

        int aCelWidth = aImage->GetCelWidth();
        int aCelHeight = aImage->GetCelHeight();
        Rect aSrcRect(aCelWidth * mFrame, aCelHeight * aProjectileDef.mImageRow, aCelWidth, aCelHeight);
        if (FloatApproxEqual(mRotation, 0.0f) && FloatApproxEqual(aScale, 1.0f))
        {
            Rect aDestRect(0, 0, aCelWidth, aCelHeight);
            g->DrawImageMirror(aImage, aDestRect, aSrcRect, aMirror);
        }
        else
        {
            float aOffsetX = mPosX + aCelWidth * 0.5f;
            float aOffsetY = mPosZ + mPosY + aCelHeight * 0.5f;
            SexyTransform2D aTransform;
            TodScaleRotateTransformMatrix(aTransform, aOffsetX + mBoard->mX, aOffsetY + mBoard->mY, mRotation, aScale, aScale);
            TodBltMatrix(g, aImage, aTransform, g->mClipRect, Color::White, g->mDrawMode, aSrcRect);
        }
    }

    if (mAttachmentID != ATTACHMENTID_NULL)
    {
        Graphics theParticleGraphics(*g);
        MakeParentGraphicsFrame(&theParticleGraphics);
        AttachmentDraw(mAttachmentID, &theParticleGraphics, false);
    }
}

void Projectile::DrawShadow(Graphics* g)
{
    int aCelCol = 0;
    float aScale = 1.0f;
    float aStretch = 1.0f;
    float aOffsetX = mPosX - mX;
    float aOffsetY = mPosY - mY;

    int aGridX = mBoard->PixelToGridXKeepOnBoard(mX, mY);
    bool isHighGround = false;
    if (mBoard->mGridSquareType[aGridX][mRow] == GRIDSQUARE_HIGH_GROUND)
    {
        isHighGround = true;
    }
    if (mOnHighGround && !isHighGround)
    {
        aOffsetY += HIGH_GROUND_HEIGHT;
    }
    else if (!mOnHighGround && isHighGround)
    {
        aOffsetY -= HIGH_GROUND_HEIGHT;
    }

    if (mBoard->StageIsNight())
    {
        aCelCol = 1;
    }

    switch (mProjectileType)
    {
    case PROJECTILE_PEA:
    case PROJECTILE_ZOMBIE_PEA:
        aOffsetX += 3.0f;
        break;

    case PROJECTILE_SNOWPEA:
        aOffsetX += -1.0f;
        aScale = 1.3f;
        break;

    case PROJECTILE_STAR:
        aOffsetX += 7.0f;
        break;

    case PROJECTILE_CABBAGE:
    case PROJECTILE_KERNEL:
    case PROJECTILE_BUTTER:
    case PROJECTILE_MELON:
    case PROJECTILE_WINTERMELON:
        aOffsetX += 3.0f;
        aOffsetY += 10.0f;
        aScale = 1.6f;
        break;

    case PROJECTILE_PUFF:
        return;

    case PROJECTILE_COBBIG:
        aScale = 1.0f;
        aStretch = 3.0f;
        aOffsetX += 57.0f;
        break;

    case PROJECTILE_FIREBALL:
        aScale = 1.4f;
        break;
    default:
        break;
    }

    if (mMotionType == MOTION_LOBBED)
    {
        float aHeight = ClampFloat(-mPosZ, 0.0f, 200.0f);
        aScale *= 200.0f / (aHeight + 200.0f);
    }

    TodDrawImageCelScaledF(g, IMAGE_PEA_SHADOWS, aOffsetX, (mShadowY - mPosY + aOffsetY), aCelCol, 0, aScale * aStretch, aScale);
}

void Projectile::Die()
{
    mDead = true;

    if (mProjectileType == PROJECTILE_PUFF || mProjectileType == PROJECTILE_SNOWPEA)
    {
        AttachmentCrossFade(mAttachmentID, "FadeOut");
        AttachmentDetach(mAttachmentID);
    }
    else
    {
        AttachmentDie(mAttachmentID);
    }
}

Rect Projectile::GetProjectileRect()
{
    if (mProjectileType == PROJECTILE_PEA ||
        mProjectileType == PROJECTILE_SNOWPEA ||
        mProjectileType == PROJECTILE_ZOMBIE_PEA)
    {
        return Rect(mX - 15, mY, mWidth + 15, mHeight);
    }
    else if (mProjectileType == PROJECTILE_COBBIG)
    {
        return Rect(mX + mWidth / 2 - 115, mY + mHeight / 2 - 115, 230, 230);
    }
    else if (mProjectileType == PROJECTILE_MELON || mProjectileType == PROJECTILE_WINTERMELON)
    {
        return Rect(mX + 20, mY, 60, mHeight);
    }
    else if (mProjectileType == PROJECTILE_FIREBALL)
    {
        return Rect(mX, mY, mWidth - 10, mHeight);
    }
    else if (mProjectileType == PROJECTILE_SPIKE)
    {
        return Rect(mX - 25, mY, mWidth + 25, mHeight);
    }
    else
    {
        return Rect(mX, mY, mWidth, mHeight);
    }
}

void Projectile::ConvertToFireball(int theGridX)
{
    if (mHitTorchwoodGridX == theGridX)
        return;

    mProjectileType = PROJECTILE_FIREBALL;
    mHitTorchwoodGridX = theGridX;
    mApp->PlayFoley(FOLEY_FIREPEA);

    float aOffsetX = -25.0f;
    float aOffsetY = -25.0f;
    Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, REANIM_FIRE_PEA);
    if (mMotionType == MOTION_BACKWARDS)
    {
        aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
        aOffsetX += 80.0f;
    }

    aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
    aFirePeaReanim->mLoopType = REANIM_LOOP;
    aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
    AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToPea(int theGridX)
{
    if (mHitTorchwoodGridX == theGridX)
        return;

    AttachmentDie(mAttachmentID);
    mProjectileType = PROJECTILE_PEA;
    mHitTorchwoodGridX = theGridX;
    mApp->PlayFoley(FOLEY_THROW);
}

ProjectileDefinition& Projectile::GetProjectileDef()
{
    ProjectileDefinition& aProjectileDef = gProjectileDefinition[mProjectileType];
    TOD_ASSERT(aProjectileDef.mProjectileType == mProjectileType);

    return aProjectileDef;
}
