/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __LAWNMOWER_H__
#define __LAWNMOWER_H__

#include "../ConstEnums.h"
#include "../engine/Rect.h"

namespace Sexy { class LawnApp; }
class Board;
class Zombie;
namespace Sexy
{
    class Graphics;
};

class LawnMower
{
public:
    Sexy::LawnApp*      mApp;
    Board*              mBoard;
    float               mPosX;
    float               mPosY;
    int32_t             mRenderOrder;
    int32_t             mRow;
    int32_t             mAnimTicksPerFrame;
    ReanimationID       mReanimID;
    int32_t             mChompCounter;
    int32_t             mRollingInCounter;
    int32_t             mSquishedCounter;
    LawnMowerState      mMowerState;
    bool                mDead;
    bool                mVisible;
    LawnMowerType       mMowerType;
    float               mAltitude;
    MowerHeight         mMowerHeight;
    int32_t             mLastPortalX;

public:
    bool                BeginDraw(Sexy::Graphics* g) { (void)g; return false; }
    void                LawnMowerInitialize(int theRow);
    void                StartMower();
    void                Update();
    void                Draw(Sexy::Graphics* g);
    void                Die();
    Rect                GetLawnMowerAttackRect();
    void                UpdatePool();
    void                MowZombie(Zombie* theZombie);
    void                SquishMower();
    /*inline*/ void     EnableSuperMower(bool theEnable);
};

#endif
