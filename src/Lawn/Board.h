/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "../ConstEnums.h"
#include "../Sexy.TodLib/DataArray.h"
#include "../engine/Widget.h"
#include "../engine/ButtonListener.h"
#include "../engine/ButtonWidget.h"

#include "Plant.h"
#include "Zombie.h"
#include "Projectile.h"
#include "Coin.h"
#include "LawnMower.h"
#include "GridItem.h"

#include <e32std.h> // for sorting

#include "Widget/GameButton.h"
using namespace Sexy;

#define MAX_GRID_SIZE_X 9
#define MAX_GRID_SIZE_Y 6
#define MAX_ZOMBIES_IN_WAVE 50
#define MAX_ZOMBIE_WAVES 100
#define MAX_GRAVE_STONES MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y
#define MAX_POOL_GRID_SIZE 10
#define MAX_RENDER_ITEMS 2048
#define PROGRESS_METER_COUNTER 150

namespace Sexy { class LawnApp; }
class CursorObject;
class CursorPreview;
class MessageWidget;
class SeedBank;
class ToolTipWidget;
class CutScene;
class Challenge;
class Reanimation;
class DataSync;
class StoreScreen;
class TodParticleSystem;
namespace Sexy
{
	class Graphics;
	class ButtonWidget;
	class WidgetManager;
	class Image;
	class MTRand;
}

class HitResult
{
public:
	void*							mObject;
	GameObjectType					mObjectType;
};

class RenderItem
{
public:
	RenderObjectType				mRenderObjectType;
	int								mZPos;
	union
	{
		GameObject*					mGameObject;
		Plant*						mPlant;
		Zombie*						mZombie;
		Coin*						mCoin;
		Projectile*					mProjectile;
		CursorPreview*				mCursorPreview;
		TodParticleSystem*			mParticleSytem;
		Reanimation*				mReanimation;
		GridItem*					mGridItem;
		LawnMower*					mMower;
		BossPart					mBossPart;
		int							mBoardGridY;
	};
};
bool RenderItemSortFunc(const RenderItem& theItem1, const RenderItem& theItem2);

struct ZombiePicker
{
	int								mZombieCount;
	int								mZombiePoints;
	int								mZombieTypeCount[NUM_ZOMBIE_TYPES];
	int								mAllWavesZombieTypeCount[NUM_ZOMBIE_TYPES];
};

/*inline*/ void						ZombiePickerInitForWave(ZombiePicker* theZombiePicker);
/*inline*/ void						ZombiePickerInit(ZombiePicker* theZombiePicker);

struct PlantsOnLawn
{
	Plant*							mUnderPlant;
	Plant*							mPumpkinPlant;
	Plant*							mFlyingPlant;
	Plant*							mNormalPlant;
};

struct BungeeDropGrid
{
	TodWeightedGridArray			mGridArray[MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y];
	int								mGridArrayCount;
};

class Board : public Sexy::Widget, public Sexy::ButtonListener
{
public:
	LawnApp*						mApp;
	DataArray<Zombie>				mZombies;
	DataArray<Plant>				mPlants;
	DataArray<Projectile>			mProjectiles;
	DataArray<Coin>					mCoins;
	DataArray<LawnMower>			mLawnMowers;
	DataArray<GridItem>				mGridItems;
	CursorObject*					mCursorObject;
	CursorPreview*					mCursorPreview;
	MessageWidget*					mAdvice;
	SeedBank*						mSeedBank;
	GameButton*						mMenuButton;
	GameButton*						mStoreButton;
	bool							mIgnoreMouseUp;
	ToolTipWidget*					mToolTip;
	//_Font*							mDebugFont;
	CutScene*						mCutScene;
	Challenge*						mChallenge;
	bool							mPaused;
	GridSquareType					mGridSquareType[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y];
	int32_t							mGridCelLook[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y];
	int32_t							mGridCelOffset[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y][2];
	int32_t							mGridCelFog[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y + 1];
	bool							mEnableGraveStones;
	int32_t							mSpecialGraveStoneX;
	int32_t							mSpecialGraveStoneY;
	float							mFogOffset;
	int32_t							mFogBlownCountDown;
	PlantRowType					mPlantRow[MAX_GRID_SIZE_Y];
	int32_t							mWaveRowGotLawnMowered[MAX_GRID_SIZE_Y];
	int32_t							mBonusLawnMowersRemaining;
	int32_t							mIceMinX[MAX_GRID_SIZE_Y];
	int32_t							mIceTimer[MAX_GRID_SIZE_Y];
	ParticleSystemID				mIceParticleID[MAX_GRID_SIZE_Y];
	TodSmoothArray					mRowPickingArray[MAX_GRID_SIZE_Y];
	ZombieType						mZombiesInWave[MAX_ZOMBIE_WAVES][MAX_ZOMBIES_IN_WAVE];
	bool							mZombieAllowed[100];
	int32_t							mSunCountDown;
	int32_t							mNumSunsFallen;
	int32_t							mShakeCounter;
	int32_t							mShakeAmountX;
	int32_t							mShakeAmountY;
	BackgroundType					mBackground;
	int32_t							mLevel;
	int32_t							mSodPosition;
	int32_t							mPrevMouseX;
	int32_t							mPrevMouseY;
	int32_t							mSunMoney;
	int32_t							mNumWaves;
	uint32_t						mMainCounter;
	uint32_t						mEffectCounter;
	uint32_t						mDrawCount;
	int32_t							mRiseFromGraveCounter;
	int32_t							mOutOfMoneyCounter;
	int32_t							mCurrentWave;
	int32_t							mTotalSpawnedWaves;
	TutorialState					mTutorialState;
	ParticleSystemID				mTutorialParticleID;
	int32_t							mTutorialTimer;
	int32_t							mLastBungeeWave;
	int32_t							mZombieHealthToNextWave;
	int32_t							mZombieHealthWaveStart;
	int32_t							mZombieCountDown;
	int32_t							mZombieCountDownStart;
	int32_t							mHugeWaveCountDown;
	bool							mHelpDisplayed[NUM_ADVICE_TYPES];
	AdviceType						mHelpIndex;
	bool							mFinalBossKilled;
	bool							mShowShovel;
	int32_t							mCoinBankFadeCount;
	DebugTextMode					mDebugTextMode;
	bool							mLevelComplete;
	int32_t							mBoardFadeOutCounter;
	int32_t							mNextSurvivalStageCounter;
	int32_t							mScoreNextMowerCounter;
	bool							mLevelAwardSpawned;
	int32_t							mProgressMeterWidth;
	int32_t							mFlagRaiseCounter;
	int32_t							mIceTrapCounter;
	int32_t							mBoardRandSeed;
	ParticleSystemID				mPoolSparklyParticleID;
	ReanimationID					mFwooshID[MAX_GRID_SIZE_Y][12];
	int32_t							mFwooshCountDown;
	int32_t							mTimeStopCounter;
	bool							mDroppedFirstCoin;
	int32_t							mFinalWaveSoundCounter;
	int32_t							mCobCannonCursorDelayCounter;
	int32_t							mCobCannonMouseX;
	int32_t							mCobCannonMouseY;
	bool							mKilledYeti;
	bool							mMustacheMode;
	bool							mSuperMowerMode;
	bool							mFutureMode;
	bool							mPinataMode;
	bool							mDanceMode;
	bool							mDaisyMode;
	bool							mSukhbirMode;
	BoardResult						mPrevBoardResult;
	int32_t							mTriggeredLawnMowers;
	uint32_t						mPlayTimeActiveLevel;
	uint32_t						mPlayTimeInactiveLevel;
	int32_t							mMaxSunPlants;
	int64_t							mStartDrawTime;
	int64_t							mIntervalDrawTime;
	uint32_t						mIntervalDrawCountStart;
	float							mMinFPS;
	int32_t							mPreloadTime;
	intptr_t						mGameID;
	uint32_t						mGravesCleared;
	uint32_t						mPlantsEaten;
	uint32_t						mPlantsShoveled;
	bool							mPeaShooterUsed;
	bool							mCatapultPlantsUsed;
	bool							mMushroomAndCoffeeBeansOnly;
	bool							mMushroomsUsed;
	uint32_t						mLevelCoinsCollected;
	uint32_t						mGargantuarsKillsByCornCob;
	uint32_t						mCoinsCollected;
	uint32_t						mDiamondsCollected;
	uint32_t						mPottedPlantsCollected;
	uint32_t						mChocolateCollected;

public:
	Board(class LawnApp* theApp);
	virtual ~Board();

	void							DisposeBoard();
	int								CountSunBeingCollected();
	void							DrawGameObjects(Sexy::Graphics* g);
	void							ClearCursor();
	/*inline*/ bool					AreEnemyZombiesOnScreen();
	LawnMower*						FindLawnMowerInRow(int theRow);
	/*inline*/ void					SaveGame(const std::string& theFileName);
	bool							LoadGame(const std::string& theFileName);
	void							InitLevel();
	void							DisplayAdvice(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex);
	void							StartLevel();
	Plant*							AddPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SEED_NONE);
	Projectile*						AddProjectile(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType);
	Coin*							AddCoin(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion);
	void							RefreshSeedPacketFromCursor();
	ZombieType						PickGraveRisingZombieType();
	ZombieType						PickZombieType(int theZombiePoints, int theWaveIndex, ZombiePicker* theZombiePicker);
	int								PickRowForNewZombie(ZombieType theZombieType);
	/*inline*/ Zombie*				AddZombie(ZombieType theZombieType, int theFromWave);
	void							SpawnZombieWave();
	void							RemoveAllZombies();
	void							RemoveCutsceneZombies();
	void							SpawnZombiesFromGraves();
	bool					CanPlantAt(int theGridX, int theGridY, SeedType theSeedType);
	inline virtual void				MouseMove(int x, int y) { (void)x; (void)y; }
	inline virtual void				MouseDrag(int x, int y) { (void)x; (void)y; }
	virtual void					MouseDown(int x, int y, int theClickCount);
	virtual void					MouseUp(int x, int y);
	virtual void KeyChar(char theChar);
	virtual bool KeyUp(KeyCode) { return EFalse; }
	virtual void KeyDown(int theKey);
	virtual void					Update();
	void							UpdateLayers();
	virtual void					Draw(Sexy::Graphics* g);
	void							DrawBackdrop(Sexy::Graphics* g);
	virtual void					ButtonPress  	(int){}
	virtual void					ButtonDepress	(int){}
	virtual void					ButtonDownTick	(int){}
	virtual void					ButtonMouseEnter(int){}
	virtual void					ButtonMouseLeave(int){}
	virtual void					ButtonMouseMove(int, int, int){}
	/*inline*/ void					AddSunMoney(int theAmount);
	void							TakeSunMoney(int theAmount);
	/*inline*/ bool					CanTakeSunMoney(int theAmount);
	/*inline*/ void					Pause(bool thePause);
	inline bool						MakeEasyZombieType() { return false; }
	void							TryToSaveGame();
	/*inline*/ bool					NeedSaveGame();
	/*inline*/ bool					RowCanHaveZombies(int theRow);
	void							ProcessDeleteQueue();
	bool							ChooseSeedsOnCurrentLevel();
	int								GetNumSeedsInBank();
	/*inline*/ bool					StageIsNight();
	/*inline*/ bool					StageHasPool();
	/*inline*/ bool					StageHas6Rows();
	/*inline*/ bool					StageHasFog();
	/*inline*/ bool					StageIsDayWithoutPool();
	/*inline*/ bool					StageIsDayWithPool();
	bool							StageHasGraveStones();
	int								PixelToGridX(int theX, int theY);
	int								PixelToGridY(int theX, int theY);
	/*inline*/ int					GridToPixelX(int theGridX, int theGridY);
	int								GridToPixelY(int theGridX, int theGridY);
	bool							GridInRange(int theGridX, int theGridY, int theTargetX, int theTargetY, int theRangeX, int theRangeY);
	/*inline*/ int					PixelToGridXKeepOnBoard(int theX, int theY);
	/*inline*/ int					PixelToGridYKeepOnBoard(int theX, int theY);
	void							UpdateGameObjects();
	void							MouseHitTest(int x, int y, HitResult* theHitResult);
	PlantingReason					MouseDownWithPlant(int x, int y, SeedType theSeedType, SeedType theImitaterType);
	void							MouseDownWithTool(int x, int y, GameObjectType theToolType);
	void							CanInteractWithBoardButtons();
	void							DrawProgressMeter(Sexy::Graphics* g);
	void							UpdateToolTip();
	Plant*							GetTopPlantAt(int theGridX, int theGridY, PlantPriority thePriority);
	void							GetPlantsOnLawn(int theGridX, int theGridY, PlantsOnLawn* thePlantOnLawn);
	/*inline*/ int					CountSunFlowers();
	int								GetSeedPacketPositionX(int theIndex);
	void							AddGraveStones(int theGridX, int theCount, Sexy::MTRand& theLevelRNG);
	int								GetGraveStoneCount();
	void							ZombiesWon();
	void							DrawLevel(Sexy::Graphics* g);
	void							DrawShovel(Sexy::Graphics* g);
	void							UpdateZombieSpawning();
	void							UpdateSunSpawning();
	/*inline*/ void					ClearAdvice(AdviceType theHelpIndex);
	bool							RowCanHaveZombieType(int theRow, ZombieType theZombieType);
	/*inline*/ int					NumberZombiesInWave(int theWaveIndex);
	int								TotalZombiesHealthInWave(int theWaveIndex);
	void							DrawDebugText(Sexy::Graphics* g);
	void							DrawUICoinBank(Sexy::Graphics* g);
	/*inline*/ void					ShowCoinBank();
	void							FadeOutLevel();
	void							DrawFadeOut(Sexy::Graphics* g);
	void							DrawIce(Sexy::Graphics* g, int theGridY);
	bool							IsIceAt(int theGridX, int theGridY);
	/*inline*/ unsigned int			ZombieGetID(Zombie* theZombie);
	/*inline*/ Zombie*				ZombieGet(unsigned int theZombieID);
	/*inline*/ Zombie*				ZombieTryToGet(unsigned int theZombieID);
	void							DrawDebugObjectRects(Sexy::Graphics* g);
	void							UpdateIce();
	/*inline*/ int					GetIceZPos(int theRow);
	/*inline*/ bool					CanAddBobSled();
	/*inline*/ void					ShakeBoard(int theShakeCounter, int theShakeAmount);
	int								CountUntriggerLawnMowers();
	bool							IterateZombies(Zombie*& theZombie);
	bool							IteratePlants(Plant*& thePlant);
	bool							IterateProjectiles(Projectile*& theProjectile);
	bool							IterateCoins(Coin*& theCoin);
	bool							IterateLawnMowers(LawnMower*& theLawnMower);
	bool							IterateParticles(TodParticleSystem*& theParticle);
	bool							IterateReanimations(Reanimation*& theReanimation);
	bool							IterateGridItems(GridItem*& theGridItem);
	/*inline*/ Zombie*				AddZombieInRow(ZombieType theZombieType, int theRow, int theFromWave);
	/*inline*/ bool					IsPoolSquare(int theGridX, int theGridY);
	void							PickZombieWaves();
	void							StopAllZombieSounds();
	/*inline*/ bool					HasLevelAwardDropped();
	void							UpdateProgressMeter();
	void							DrawUIBottom(Sexy::Graphics* g);
	void							DrawUITop(Sexy::Graphics* g);
	Zombie*							ZombieHitTest(int theMouseX, int theMouseY);
	void							KillAllPlantsInRadius(int theX, int theY, int theRadius);
	Plant*							GetPumpkinAt(int theGridX, int theGridY);
	Plant*							GetFlowerPotAt(int theGridX, int theGridY);
	static bool						CanZombieSpawnOnLevel(ZombieType theZombieType, int theLevel);
	bool							IsZombieWaveDistributionOk();
	void							PickBackground();
	void							InitZombieWaves();
	void							InitSurvivalStage();
	static /*inline*/ int			MakeRenderOrder(RenderLayer theRenderLayer, int theRow, int theLayerOffset);
	void							UpdateGame();
	void							InitZombieWavesForLevel(int theForLevel);
	SeedNotRecommend				SeedNotRecommendedForLevel(SeedType theSeedType);
	void							DrawTopRightUI(Sexy::Graphics* g);
	void							DrawFog(Sexy::Graphics* g);
	void							UpdateFog();
	/*inline*/ int					LeftFogColumn();
	static /*inline*/ bool			IsZombieTypePoolOnly(ZombieType theZombieType);
	void							DropLootPiece(float thePosX, float thePosY, CoinType aCoinType);
	void							UpdateLevelEndSequence();
	LawnMower*						GetBottomLawnMower();
	bool							CanDropLoot();
	ZombieType						GetIntroducedZombieType();
	void							PickSpecialGraveStone();
	int								GetPosYBasedOnRow(int theRow);
	float							GetPosYBasedOnRow(float thePosX, int theRow);
	void							NextWaveComing();
	bool							BungeeIsTargetingCell(int theGridX, int theGridY);
	/*inline*/ int					PlantingPixelToGridX(int theX, int theY);
	/*inline*/ int					PlantingPixelToGridY(int theX, int theY);
	Plant*							FindUmbrellaPlant(int theGridX, int theGridY);
	void							SetTutorialState(TutorialState theTutorialState);
	void							DoFwoosh(int theRow);
	void							UpdateFwoosh();
	GameObjectType					SpecialPlantHitTest(int x, int y);
	void							UpdateMousePosition();
	/*inline*/ bool					ToolHitTestHelper(int x, int y, GameObjectType theObjectType);
	/*inline*/ GameObjectType		ToolHitTest(int theX, int theY);
	bool							CanAddGraveStoneAt(int theGridX, int theGridY);
	void							UpdateGridItems();
	/*inline*/ GridItem*			AddAGraveStone(int theGridX, int theGridY);
	int								GetSurvivalFlagsCompleted();
	bool							HasProgressMeter();
	inline void						UpdateCursor() {}
	void							UpdateZenCursor();
	void							UpdateTutorial();
	SeedType						GetSeedTypeInCursor();
	/*inline*/ int					CountPlantByType(SeedType theSeedType);
	bool							PlantingRequirementsMet(SeedType theSeedType, int theGridX, int theGridY);
	bool							HasValidCobCannonSpot();
	bool							IsValidCobCannonSpot(int theGridX, int theGridY);
	bool							IsValidCobCannonSpotHelper(int theGridX, int theGridY);
	void							MouseDownCobcannonFire(int x, int y, int theClickCount);
	int								KillAllZombiesInRadius(int theGridX, int theGridY, int theRadius, int theRow, int theDamageRange, bool theBurn, int theMinZombiesKilled);
	/*inline*/ int					GetSeedBankExtraWidth();
	bool							IsFlagWave(int theWaveNumber);
	void							DrawHouseDoorTop(Sexy::Graphics* g);
	void							DrawHouseDoorBottom(Sexy::Graphics* g);
	Zombie*							GetBossZombie();
	bool							HasConveyorBeltSeedBank();
	/*inline*/ bool					StageHasRoof();
	void							SpawnZombiesFromPool();
	void							SpawnZombiesFromSky();
	void							PickUpTool(int x, int y, GameObjectType theObjectType);
	void							TutorialArrowShow(float theX, float theY, float theRotation);
	void							TutorialArrowRemove();
	int								CountCoinsBeingCollected();
	void							BungeeDropZombie(ZombieType theZombieType);
	void							SetupBungeeDrop(int x, int theGridY, ZombieType theZombieType);
	/*inline*/ void					PutZombieInWave(ZombieType theZombieType, int theWaveNumber, ZombiePicker* theZombiePicker);
	/*inline*/ void					PutInMissingZombies(int theWaveNumber, ZombiePicker* theZombiePicker);
	Rect							GetShovelButtonRect();
	void							GetZenButtonRect(GameObjectType theObjectType, Rect& theRect);
	Plant*							NewPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SEED_NONE);
	void							DoPlantingEffects(int theGridX, int theGridY, Plant* thePlant);
	bool							IsFinalSurvivalStage();
	void							SurvivalSaveScore();
	int								CountZombiesOnScreen();
	int								GetLiveGargantuarCount();
	/*inline*/ int					GetNumWavesPerSurvivalStage();
	int								GetLevelRandSeed();
	void							AddBossRenderItem(Zombie* theZombie, RenderItem* theRenderItems, int& theRenderItemCount);
	/*inline*/ GridItem*			GetCraterAt(int theGridX, int theGridY);
	/*inline*/ GridItem*			GetGraveStoneAt(int theGridX, int theGridY);
	/*inline*/ GridItem*			GetLadderAt(int theGridX, int theGridY);
	/*inline*/ GridItem*			AddALadder(int theGridX, int theGridY);
	/*inline*/ GridItem*			AddACrater(int theGridX, int theGridY);
	void							InitLawnMowers();
	/*inline*/ bool					IsPlantInCursor();
	void							HighlightPlantsForMouse(int theMouseX, int theMouseY);
	void							ClearFogAroundPlant(Plant* thePlant, int theSize);
	/*inline*/ void					RemoveParticleByType(ParticleEffect theEffectType);
	/*inline*/ GridItem*			GetScaryPotAt(int theGridX, int theGridY);
	void							PuzzleSaveStreak();
	/*inline*/ void					ClearAdviceImmediately();
	/*inline*/ bool					IsFinalScaryPotterStage();
	/*inline*/ void					DisplayAdviceAgain(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex);
	GridItem*						GetSquirrelAt(int theGridX, int theGridY);
	GridItem*						GetZenToolAt(int theGridX, int theGridY);
	bool							IsPlantInGoldWateringCanRange(int theMouseX, int theMouseY, Plant* thePlant);
	bool							StageHasZombieWalkInFromRight();
	void							PlaceRake();
	GridItem*						GetRake();
	/*inline*/ bool					IsScaryPotterDaveTalking();
	/*inline*/ Zombie*				GetWinningZombie();
	/*inline*/ void					ResetFPSStats();
	int								CountEmptyPotsOrLilies(SeedType theSeedType);
	GridItem*						GetGridItemAt(GridItemType theGridItemType, int theGridX, int theGridY);
	bool							ProgressMeterHasFlags();
	/*inline*/ bool					IsLastStandFinalStage();
	/*inline*/ int					GetNumWavesPerFlag();
	int								GetCurrentPlantCost(SeedType theSeedType, int theSeedIndex);
	/*inline*/ bool					PlantUsesAcceleratedPricing(SeedType theSeedType);
	void							FreezeEffectsForCutscene(bool theFreeze);
	void							LoadBackgroundImages();
	bool							CanUseGameObject(GameObjectType theGameObject);
	void							SetMustacheMode(bool theEnableMustache);
	int								CountCoinByType(CoinType theCoinType);
	void							SetSuperMowerMode(bool theEnableSuperMower);
	void							DrawZenWheelBarrowButton(Sexy::Graphics* g);
	void							DrawZenButtons(Sexy::Graphics* g);
	/*inline*/ int					OffsetYForPlanting(float theGridY);
	void							SetDanceMode(bool theEnableDance);
	void							SetFutureMode(bool theEnableFuture);
	void							SetPinataMode(bool theEnablePinata);
	void							SetDaisyMode(bool theEnableDaisy);
	void							SetSukhbirMode(bool theEnableSukhbir);
	void							MouseHitTestPlant(int x, int y, HitResult* theHitResult);

	/*inline*/ Reanimation*			CreateRakeReanim(float theRakeX, float theRakeY, int theRenderOrder);
	void							CompleteEndLevelSequenceForSaving();
	void							RemoveZombiesForRepick();
	int								GetGraveStonesCount();
	/*inline*/ bool					IsSurvivalStageWithRepick();
	/*inline*/ bool					IsLastStandStageWithRepick();
	void							DoTypingCheck();
	int								CountZombieByType(ZombieType theZombieType);
	static /*inline*/ bool			IsZombieTypeSpawnedOnly(ZombieType theZombieType);
};
extern bool gShownMoreSunTutorial;

int									GetRectOverlap(const Rect& rect1, const Rect& rect2);
bool								GetCircleRectOverlap(int theCircleX, int theCircleY, int theRadius, const Rect& theRect);
/*inline*/ void						BoardInitForPlayer();

#endif // __BOARD_H__

