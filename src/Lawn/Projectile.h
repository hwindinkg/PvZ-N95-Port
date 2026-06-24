/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "ConstEnums.h"
#include "GameObject.h"

class HitResult;
class Plant;
class Zombie;
namespace Sexy
{
    class Graphics;
};
using namespace Sexy;

class ProjectileDefinition
{
public:
    ProjectileType          mProjectileType;
    int32_t                 mImageRow;
    int32_t                 mDamage;
};
extern ProjectileDefinition gProjectileDefinition[NUM_PROJECTILES];

class Projectile : public GameObject
{
public:
    int32_t                 mFrame;
    int32_t                 mNumFrames;
    int32_t                 mAnimCounter;
    float                   mPosX;
    float                   mPosY;
    float                   mPosZ;
    float                   mVelX;
    float                   mVelY;
    float                   mVelZ;
    float                   mAccZ;
    float                   mShadowY;
    bool                    mDead;
    int32_t                 mAnimTicksPerFrame;
    ProjectileMotion        mMotionType;
    ProjectileType          mProjectileType;
    int32_t                 mProjectileAge;
    int32_t                 mClickBackoffCounter;
    float                   mRotation;
    float                   mRotationSpeed;
    bool                    mOnHighGround;
    int32_t                 mDamageRangeFlags;
    int32_t                 mHitTorchwoodGridX;
    AttachmentID            mAttachmentID;
    float                   mCobTargetX;
    int32_t                 mCobTargetRow;
    ZombieID                mTargetZombieID;
    int32_t                 mLastPortalX;

public:
    Projectile();
    ~Projectile();

    void                    ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType);
    void                    Update();
    void                    Draw(Graphics* g);
    void                    DrawShadow(Graphics* g);
    void                    Die();
    void                    DoImpact(Zombie* theZombie);
    void                    UpdateMotion();
    void                    CheckForCollision();
    Zombie*                 FindCollisionTarget();
    void                    UpdateLobMotion();
    void                    CheckForHighGround();
    bool                    CantHitHighGround();
    void                    DoSplashDamage(Zombie* theZombie);
    ProjectileDefinition&   GetProjectileDef();
    unsigned int            GetDamageFlags(Zombie* theZombie = NULL);
    Rect                    GetProjectileRect();
    void                    UpdateNormalMotion();
    Plant*                  FindCollisionTargetPlant();
    void                    ConvertToFireball(int theGridX);
    void                    ConvertToPea(int theGridX);
    bool                    IsSplashDamage(Zombie* theZombie = NULL);
    void                    PlayImpactSound(Zombie* theZombie);
    bool                    IsZombieHitBySplash(Zombie* theZombie);
    bool                    PeaAboutToHitTorchwood();
    bool                    MouseHitTest(int x, int y, HitResult* theHitResult) { (void)x; (void)y; (void)theHitResult; return false; }

};

#endif
