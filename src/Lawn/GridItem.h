/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __GRIDITEM_H__
#define __GRIDITEM_H__

#include "../ConstEnums.h"

#define NUM_MOTION_TRAIL_FRAMES 12

namespace Sexy { class LawnApp; }
class Board;
class Zombie;
namespace Sexy
{
	class Graphics;
};

class MotionTrailFrame
{
public:
	float					mPosX;
	float					mPosY;
	float					mAnimTime;
};

class GridItem
{
public:
	Sexy::LawnApp*			mApp;
	Board*					mBoard;
	GridItemType			mGridItemType;
	GridItemState			mGridItemState;
	int32_t					mGridX;
	int32_t					mGridY;
	int32_t					mGridItemCounter;
	int32_t					mRenderOrder;
	bool					mDead;
	float					mPosX;
	float					mPosY;
	float					mGoalX;
	float					mGoalY;
	ReanimationID			mGridItemReanimID;
	ParticleSystemID		mGridItemParticleID;
	ZombieType				mZombieType;
	SeedType				mSeedType;
	ScaryPotType			mScaryPotType;
	bool					mHighlighted;
	int32_t					mTransparentCounter;
	int32_t					mSunCount;
	MotionTrailFrame		mMotionTrailFrames[NUM_MOTION_TRAIL_FRAMES];
	int32_t					mMotionTrailCount;

public:
	GridItem();

	bool					BeginDraw(Sexy::Graphics* g) { (void)g; return false; }
	bool					MouseHitTest(int x, int y, HitResult* theHitResult) { (void)x; (void)y; (void)theHitResult; return false; }
	void					DrawLadder(Sexy::Graphics* g);
	void					DrawCrater(Sexy::Graphics* g);
	void					DrawGraveStone(Sexy::Graphics* g);
	void					GridItemDie();
	void					AddGraveStoneParticles();
	void					DrawGridItem(Sexy::Graphics* g);
	void					DrawGridItemOverlay(Sexy::Graphics* g);
	void					OpenPortal();
	void					Update();
	void					ClosePortal();
	void					DrawScaryPot(Sexy::Graphics* g);
	void					UpdateScaryPot();
	void					UpdatePortal();
	void					DrawSquirrel(Sexy::Graphics* g);
	void					UpdateRake();
	Zombie*					RakeFindZombie();
	void					DrawIZombieBrain(Sexy::Graphics* g);
	void					UpdateBrain();
	void					DrawStinky(Sexy::Graphics* g);
	/*inline*/ bool			IsOpenPortal();
};

#endif
