/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#ifndef __COIN_H__
#define __COIN_H__

#include "GameObject.h"
#include "ZenGarden.h"
struct PottedPlant { int mGrowDays; int mAge; int mAgeSeq; int mFlowerType; int mFertilizer; int mPacked; int mLastWateredTime; int mLastFertilizedTime; int mPlantId; int mSeedType; int mFacing; int mPlantAge; int mNeed; float mPosX; float mPosY; void InitializePottedPlant(int theSeedType) { mSeedType = theSeedType; mGrowDays = 0; mAge = 0; mAgeSeq = 0; mFlowerType = 0; mFertilizer = 0; mPacked = 0; mLastWateredTime = 0; mLastFertilizedTime = 0; mPlantId = 0; mFacing = 0; mPlantAge = 0; mNeed = 0; mPosX = 0.0f; mPosY = 0.0f; } };
#include "System/PlayerInfo.h"

class HitResult;
namespace Sexy
{
    class Graphics;
}
using namespace Sexy;

class Coin : public GameObject
{
public:
    float                   mPosX;
    float                   mPosY;
    float                   mVelX;
    float                   mVelY;
    float                   mScale;
    bool                    mDead;
    int32_t                 mFadeCount;
    float                   mCollectX;
    float                   mCollectY;
    int32_t                 mGroundY;
    int32_t                 mCoinAge;
    bool                    mIsBeingCollected;
    int32_t                 mDisappearCounter;
    CoinType                mType;
    CoinMotion              mCoinMotion;
    AttachmentID            mAttachmentID;
    float                   mCollectionDistance;
    SeedType                mUsableSeedType;
    PottedPlant             mPottedPlantSpec;
    bool                    mNeedsBouncyArrow;
    bool                    mHasBouncyArrow;
    bool                    mHitGround;
    int32_t                 mTimesDropped;

public:
    Coin();
    ~Coin();

    void                    CoinInitialize(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion);
    void                    MouseDown(int x, int y, int theClickCount);
    bool                    MouseHitTest(int theX, int theY, HitResult* theHitResult);
    void                    Die();
    void                    StartFade();
    void                    Update();
    void                    Draw(Graphics* g);
    void                    Collect();
    /*inline*/ int          GetSunValue();
    static /*inline*/ int   GetCoinValue(CoinType theCoinType);
    void                    UpdateFade();
    void                    UpdateFall();
    void                    ScoreCoin();
    void                    UpdateCollected();
    Color                   GetColor();
    /*inline*/ bool         IsMoney();
    /*inline*/ bool         IsSun();
    float                   GetSunScale();
    inline bool             IsOnGround() { return false; }
    SeedType                GetFinalSeedPacketType();
    bool                    IsLevelAward();
    bool                    CoinGetsBouncyArrow();
    void                    FanOutCoins(CoinType theCoinType, int theNumCoins);
    int                     GetDisappearTime();
    void                    DroppedUsableSeed();
    void                    PlayCollectSound();
    void                    TryAutoCollectAfterLevelAward();
    bool                    IsPresentWithAdvice();
    void                    PlayLaunchSound();
    void                    PlayGroundSound();

    static /*inline*/ bool  IsMoney(CoinType theType);
};

#endif
