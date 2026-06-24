/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#include <time.h>
#include "../Sexy.TodLib/Trail.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/Attachment.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/EffectSystem.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "../engine/Dialog.h"
#include "../engine/MTRand.h"
#include "../engine/ImageFont.h"
#include "../engine/SoundManager.h"
#include "../engine/ButtonWidget.h"
#include "../engine/WidgetManager.h"
#include "../engine/SoundInstance.h"
#include "../engine/PerfTimer.h"
#include "../Resources.h"

#include "Board.h"
#include "../LawnApp.h"

// Global variables
bool mChooseShapeSwapChallenged = false;
#include "AllStubs.h"
#include "LawnCommon.h"
#include "ZenGarden.h"
#include "BoardInclude.h"
#include "System/Music.h"
#include "System/SaveGame.h"
#include "Widget/LawnDialog.h"
#include "System/PlayerInfo.h"
#include "System/ReportAchievement.h"
#include "System/PoolEffect.h"
#include "System/TypingCheck.h"
#include "Widget/StoreScreen.h"
#include "Widget/AwardScreen.h"
#include "Widget/ChallengeScreen.h"
#include "Widget/SeedChooserScreen.h"
#include "Widget/AchievementsScreen.h"

//#include "graphics/SysFont.h"

//#define SEXY_PERF_ENABLED

//#define SEXY_MEMTRACE
//#include "../SexyAppFramework/memmgr.h"

bool gShownMoreSunTutorial = false;

// GOTY @Patoke: 0x40A3C0
Board::Board(Sexy::LawnApp* theApp)
{
	mApp = theApp;
	mApp->mBoard = this;
	TodHesitationTrace("preboard");

	mZombies.DataArrayInitialize(1024U, "zombies");
	mPlants.DataArrayInitialize(1024U, "plants");
	mProjectiles.DataArrayInitialize(1024U, "projectiles");
	mCoins.DataArrayInitialize(1024U, "coins");
	mLawnMowers.DataArrayInitialize(32U, "lawnmowers");
	mGridItems.DataArrayInitialize(128U, "griditems");
	TodHesitationTrace("board dataarrays");

	mApp->mEffectSystem->EffectSystemFreeAll();
	mBoardRandSeed = mApp->mAppRandSeed;
	if (mApp->IsSurvivalMode())
	{
		mBoardRandSeed = Rand();
	}
	mCoinBankFadeCount = 0;
	mLevel = 0;
	mCursorObject = new CursorObject();
	mCursorPreview = new CursorPreview();
	mSeedBank = new SeedBank();
	mCutScene = new CutScene();
	mSpecialGraveStoneX = -1;
	mSpecialGraveStoneY = -1;
	for (int i = 0; i < MAX_GRID_SIZE_X; i++)
	{
		for (int j = 0; j < MAX_GRID_SIZE_Y; j++)
		{
			mGridSquareType[i][j] = GRIDSQUARE_GRASS;
			mGridCelLook[i][j] = Rand(20);
			mGridCelOffset[i][j][0] = Rand(10) - 5;
			mGridCelOffset[i][j][1] = Rand(10) - 5;
		}

		for (int k = 0; k < MAX_GRID_SIZE_Y + 1; k++)
		{
			mGridCelFog[i][k] = 0;
		}
	}
	mFogOffset = 0.0f;
	mSunCountDown = 0;
	mShakeCounter = 0;
	mShakeAmountX = 0;
	mShakeAmountY = 0;
	mPaused = false;
	mLevelAwardSpawned = false;
	mFlagRaiseCounter = 0;
	mIceTrapCounter = 0;
	mLevelComplete = false;
	mBoardFadeOutCounter = -1;
	mNextSurvivalStageCounter = 0;
	mScoreNextMowerCounter = 0;
	mProgressMeterWidth = 0;
	mPoolSparklyParticleID = PARTICLESYSTEMID_NULL;
	mFogBlownCountDown = 0;
	mFwooshCountDown = 0;
	mTimeStopCounter = 0;
	mCobCannonCursorDelayCounter = 0;
	mCobCannonMouseX = 0;
	mCobCannonMouseY = 0;
	mDroppedFirstCoin = false;
	mBonusLawnMowersRemaining = 0;
	mEnableGraveStones = false;
	mHelpIndex = ADVICE_NONE;
	mEffectCounter = 0;
	mDrawCount = 0;
	mRiseFromGraveCounter = 0;
	mFinalWaveSoundCounter = 0;
	mKilledYeti = false;
	mTriggeredLawnMowers = 0;
	mPlayTimeActiveLevel = 0;
	mPlayTimeInactiveLevel = 0;
	mMaxSunPlants = 0;
	mStartDrawTime = 0;
	mIntervalDrawTime = 0;
	mIntervalDrawCountStart = 0;
	mPreloadTime = 0;
	mGameID = time(0);
	mMinFPS = 1000.0f;
	mGravesCleared = 0;
	mPlantsEaten = 0;
	mPlantsShoveled = 0;
	mPeaShooterUsed = false;
	mCatapultPlantsUsed = false;
	mMushroomAndCoffeeBeansOnly = true;
	mMushroomsUsed = false;
	mLevelCoinsCollected = 0;
	mGargantuarsKillsByCornCob = 0;
	mCoinsCollected = 0;
	mDiamondsCollected = 0;
	mPottedPlantsCollected = 0;
	mChocolateCollected = 0;
	for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < 12; x++)
		{
			mFwooshID[y][x] = REANIMATIONID_NULL;
		}
	}
	mPrevMouseX = -1;
	mPrevMouseY = -1;
	mFinalBossKilled = false;
	mMustacheMode = mApp->mMustacheMode;
	mSuperMowerMode = mApp->mSuperMowerMode;
	mFutureMode = mApp->mFutureMode;
	mPinataMode = mApp->mPinataMode;
	mDanceMode = mApp->mDanceMode;
	mDaisyMode = mApp->mDaisyMode;
	mSukhbirMode = mApp->mSukhbirMode;
	mShowShovel = false;
	mToolTip = new ToolTipWidget();
	//mDebugFont = new SysFont("Arial Unicode MS", 10, true, false, false);
	mAdvice = new MessageWidget(mApp);
	mBackground = BACKGROUND_1_DAY;
	mMainCounter = 0;
	mTutorialState = TUTORIAL_OFF;
	mTutorialTimer = -1;
	mTutorialParticleID = PARTICLESYSTEMID_NULL;
	mChallenge = new Challenge();
	mClip = false;
	mDebugTextMode = DEBUG_TEXT_NONE;
	mMenuButton = new GameButton(0);
	mMenuButton->mDrawStoneButton = true;
	mStoreButton = NULL;
	mIgnoreMouseUp = false;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		mMenuButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->Resize(628, -10, 163, 46);

		mStoreButton = new GameButton(1);
		mStoreButton->mButtonImage = IMAGE_ZENSHOPBUTTON;
		mStoreButton->mOverImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
		mStoreButton->mDownImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
		mStoreButton->mParentWidget = this;
		mStoreButton->Resize(678, 33, IMAGE_ZENSHOPBUTTON->mWidth, 40);
	}
	else
	{
		mMenuButton->SetLabel("[MENU_BUTTON]");
		mMenuButton->Resize(681, -10, 117, 46);
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
	{
		mStoreButton = new GameButton(1);
		mStoreButton->mDrawStoneButton = true;
		mStoreButton->mBtnNoDraw = true;
		mStoreButton->mDisabled = true;
	}

	if (mApp->mGameMode == GAMEMODE_UPSELL)
	{
		mMenuButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->Resize(628, -10, 163, 46);

		mStoreButton = new GameButton(1);
		mStoreButton->mDrawStoneButton = true;
		mStoreButton->mBtnNoDraw = true;
		mStoreButton->SetLabel("[GET_FULL_VERSION_BUTTON]");
	}
}

Board::~Board()
{
	delete mAdvice;
	delete mCursorObject;
	delete mCursorPreview;
	delete mSeedBank;
	if (mMenuButton)
	{
		delete mMenuButton;
	}
	if (mStoreButton)
	{
		delete mStoreButton;
	}
	mZombies.DataArrayDispose();
	mPlants.DataArrayDispose();
	mProjectiles.DataArrayDispose();
	mCoins.DataArrayDispose();
	mLawnMowers.DataArrayDispose();
	mGridItems.DataArrayDispose();
	if (mToolTip)
	{
		delete mToolTip;
	}
	/*
	if (mDebugFont)
	{
		delete mDebugFont;
	}
	*/
	delete mCutScene;
	delete mChallenge;
}

void BoardInitForPlayer()
{
	gShownMoreSunTutorial = false;
}

// GOTY @Patoke: 0x40B320
void Board::DisposeBoard()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
		mApp->mZenGarden->LeaveGarden();
	if (mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
		mChallenge->TreeOfWisdomLeave();

	mApp->mSoundSystem->StopFoley(FOLEY_RAIN);
	mApp->mZenGarden->mBoard = NULL;
	mApp->CrazyDaveDie();
	mApp->mEffectSystem->EffectSystemFreeAll();
}

bool Board::AreEnemyZombiesOnScreen()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled)
		{
			return true;
		}
	}
	return false;
}

// GOTY @Patoke: 0x40B4A0
int Board::CountZombiesOnScreen()
{
	int aCount = 0;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled && aZombie->IsOnBoard())
		{
			aCount++;
		}
	}
	return aCount;
}

// GOTY @Patoke: 0x40B3B0
int Board::GetLiveGargantuarCount() {
	int aCount = 0;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && aZombie->IsOnBoard() && (aZombie->mZombieType == ZOMBIE_GARGANTUAR || aZombie->mZombieType == ZOMBIE_REDEYE_GARGANTUAR))
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountUntriggerLawnMowers()
{
	int aCount = 0;
	LawnMower* aLawnMower = NULL;
	while (IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mMowerState != MOWER_TRIGGERED && aLawnMower->mMowerState != MOWER_SQUISHED)
		{
			aCount++;
		}
	}
	return aCount;
}

void Board::TryToSaveGame()
{
	std::string aFileName = GetSavedGameName(mApp->mGameMode, mApp->mPlayerInfo->mId);

	if (NeedSaveGame())
	{
		if (mBoardFadeOutCounter > 0)
		{
			CompleteEndLevelSequenceForSaving();
			return;
		}

		MkDir(GetAppDataPath("userdata"));
		mApp->mMusic->GameMusicPause(true);
		LawnSaveGame(this, aFileName);
		mApp->ClearUpdateBacklog();
		SurvivalSaveScore();
	}
}

bool Board::NeedSaveGame()
{
	return
		mApp->mGameMode != GAMEMODE_CHALLENGE_ICE &&
		mApp->mGameMode != GAMEMODE_UPSELL &&
		mApp->mGameMode != GAMEMODE_INTRO &&
		mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN &&
		mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM &&
		mApp->mGameScene == SCENE_PLAYING;
}

void Board::SaveGame(const std::string& theFileName)
{
	LawnSaveGame(this, theFileName);
}

// GOTY @Patoke: 0x40B739
void Board::ResetFPSStats()
{
	int64 aTickCount = User::TickCount();
	mStartDrawTime = aTickCount;
	mIntervalDrawTime = aTickCount;
	mDrawCount = 1;
	mIntervalDrawCountStart = 1;
}

// GOTY @Patoke: 0x40B710
bool Board::LoadGame(const std::string& theFileName)
{
	if (!LawnLoadGame(this, theFileName))
		return false;

	LoadBackgroundImages();
	mApp->ClearUpdateBacklog();
	ResetFPSStats();
	UpdateLayers();
	return true;
}

GridItem* Board::GetGridItemAt(GridItemType theGridItemType, int theGridX, int theGridY)
{
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY && aGridItem->mGridItemType == theGridItemType)
		{
			return aGridItem;
		}
	}
	return NULL;
}

GridItem* Board::GetRake()
{
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_RAKE)
		{
			return aGridItem;
		}
	}
	return NULL;
}

GridItem* Board::GetCraterAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_CRATER, theGridX, theGridY);
}

GridItem* Board::GetGraveStoneAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_GRAVESTONE, theGridX, theGridY);
}

GridItem* Board::GetLadderAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_LADDER, theGridX, theGridY);
}

GridItem* Board::GetScaryPotAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_SCARY_POT, theGridX, theGridY);
}

/*
GridItem* Board::GetSquirrelAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_SQUIRREL, theGridX, theGridY);
}
*/

GridItem* Board::GetZenToolAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GRIDITEM_ZEN_TOOL, theGridX, theGridY);
}

bool Board::CanAddGraveStoneAt(int theGridX, int theGridY)
{
	if (mGridSquareType[theGridX][theGridY] != GRIDSQUARE_GRASS && mGridSquareType[theGridX][theGridY] != GRIDSQUARE_HIGH_GROUND)
	{
		return false;
	}

	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY)
		{
			if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE ||
				aGridItem->mGridItemType == GRIDITEM_CRATER ||
				aGridItem->mGridItemType == GRIDITEM_LADDER)
				return false;
		}
	}
	return true;
}

int Board::MakeRenderOrder(RenderLayer theRenderLayer, int theRow, int theLayerOffset)
{
	return theRow * static_cast<int>(RENDER_LAYER_ROW_OFFSET) + theRenderLayer + theLayerOffset;
}

GridItem* Board::AddALadder(int theGridX, int theGridY)
{
	GridItem* aLadder = mGridItems.DataArrayAlloc();
	aLadder->mGridItemType = GRIDITEM_LADDER;
	aLadder->mRenderOrder = MakeRenderOrder(RENDER_LAYER_PLANT, theGridY, 800);
	aLadder->mGridX = theGridX;
	aLadder->mGridY = theGridY;
	return aLadder;
}

GridItem* Board::AddACrater(int theGridX, int theGridY)
{
	GridItem* aCrater = mGridItems.DataArrayAlloc();
	aCrater->mGridItemType = GRIDITEM_CRATER;
	aCrater->mRenderOrder = MakeRenderOrder(RENDER_LAYER_GROUND, theGridY, 1);
	aCrater->mGridX = theGridX;
	aCrater->mGridY = theGridY;
	return aCrater;
}

GridItem* Board::AddAGraveStone(int theGridX, int theGridY)
{
	GridItem* aGraveStone = mGridItems.DataArrayAlloc();
	aGraveStone->mGridItemType = GRIDITEM_GRAVESTONE;
	aGraveStone->mGridItemCounter = -Rand(50);
	aGraveStone->mRenderOrder = MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, theGridY, 3);
	aGraveStone->mGridX = theGridX;
	aGraveStone->mGridY = theGridY;
	return aGraveStone;
}

void Board::AddGraveStones(int theGridX, int theCount, MTRand& theLevelRNG)
{
	TOD_ASSERT(theCount <= MAX_GRID_SIZE_Y);

	int aGridAllowGraveStonesCount = 0;
	for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
	{
		if (CanAddGraveStoneAt(theGridX, y))
		{
			aGridAllowGraveStonesCount++;
		}
	}
	theCount = MIN(theCount, aGridAllowGraveStonesCount);

	int i = 0;
	while (i < theCount)
	{
		int aGridY = theLevelRNG.Next((unsigned long)MAX_GRID_SIZE_Y);
		if (CanAddGraveStoneAt(theGridX, aGridY))
		{
			GridItem* aGraveStone = AddAGraveStone(theGridX, aGridY);
			(void)aGraveStone;
			++i;
		}
	}
}

int Board::GetNumWavesPerFlag()
{
	return (mApp->IsFirstTimeAdventureMode() && mNumWaves < 10) ? mNumWaves : 10;
}

bool Board::IsFlagWave(int theWaveNumber)
{
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
		return false;

	int aWavesPerFlag = GetNumWavesPerFlag();
	return theWaveNumber % aWavesPerFlag == aWavesPerFlag - 1;
}

void ZombiePickerInitForWave(ZombiePicker* theZombiePicker)
{
	theZombiePicker->mZombieCount = 0;
	theZombiePicker->mZombiePoints = 0;
	memset(theZombiePicker->mZombieTypeCount, 0, sizeof(theZombiePicker->mZombieTypeCount));
}

void ZombiePickerInit(ZombiePicker* theZombiePicker)
{
	ZombiePickerInitForWave(theZombiePicker);
	memset(theZombiePicker->mAllWavesZombieTypeCount, 0, sizeof(theZombiePicker->mAllWavesZombieTypeCount));
}

void Board::PutZombieInWave(ZombieType theZombieType, int theWaveNumber, ZombiePicker* theZombiePicker)
{
	TOD_ASSERT(theWaveNumber < MAX_ZOMBIE_WAVES && theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE);
	mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount++] = theZombieType;
	if (theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE)
	{
		mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount] = ZOMBIE_INVALID;
	}
	theZombiePicker->mZombiePoints -= GetZombieDefinition(theZombieType).mZombieValue;
	theZombiePicker->mZombieTypeCount[theZombieType]++;
	theZombiePicker->mAllWavesZombieTypeCount[theZombieType]++;
}

void Board::PutInMissingZombies(int theWaveNumber, ZombiePicker* theZombiePicker)
{
	for (ZombieType aZombieType = ZOMBIE_NORMAL; aZombieType < NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (theZombiePicker->mZombieTypeCount[aZombieType] <= 0 && aZombieType != ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mLevel))
		{
			PutZombieInWave(aZombieType, theWaveNumber, theZombiePicker);
		}
	}
}

void Board::PickZombieWaves()
{
	if (mApp->IsAdventureMode())
	{
		if (mApp->IsWhackAZombieLevel())
		{
			mNumWaves = 8;
		}
		else
		{
			mNumWaves = gZombieWaves[ClampInt(mLevel - 1, 0, 49)];
			if (!mApp->IsFirstTimeAdventureMode() && !mApp->IsMiniBossLevel())
			{
				mNumWaves = mNumWaves < 10 ? 20 : mNumWaves + 10;
			}
		}
	}
	else
	{
		GameMode aGameMode = mApp->mGameMode;
		if (mApp->IsSurvivalMode() || aGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
			mNumWaves = GetNumWavesPerSurvivalStage();
		else if (aGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GAMEMODE_TREE_OF_WISDOM || mApp->IsSquirrelLevel())
			mNumWaves = 0;
		else if (aGameMode == GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE)
			mNumWaves = 12;
		else if (aGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING || aGameMode == GAMEMODE_CHALLENGE_AIR_RAID ||
				 aGameMode == GAMEMODE_CHALLENGE_GRAVE_DANGER || aGameMode == GAMEMODE_CHALLENGE_HIGH_GRAVITY ||
				 aGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT || aGameMode == GAMEMODE_CHALLENGE_WAR_AND_PEAS ||
				 aGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
			mNumWaves = 20;
		else if (mApp->IsStormyNightLevel() || mApp->IsLittleTroubleLevel() || mApp->IsBungeeBlitzLevel() ||
				 aGameMode == GAMEMODE_CHALLENGE_COLUMN || mApp->IsShovelLevel() || aGameMode == GAMEMODE_CHALLENGE_WAR_AND_PEAS_2 ||
				 aGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2 || aGameMode == GAMEMODE_CHALLENGE_POGO_PARTY)
			mNumWaves = 30;
		else
			mNumWaves = 40;
	}

	ZombiePicker aZombiePicker;
	ZombiePickerInit(&aZombiePicker);
	ZombieType aIntroZombieType = GetIntroducedZombieType();
	TOD_ASSERT(mNumWaves <= MAX_ZOMBIE_WAVES);

	for (int aWave = 0; aWave < mNumWaves; aWave++)
	{
		ZombiePickerInitForWave(&aZombiePicker);
		mZombiesInWave[aWave][0] = ZOMBIE_INVALID;

		bool aIsFlagWave = IsFlagWave(aWave);
		bool aIsFinalWave = aWave == mNumWaves - 1;

		if (mApp->IsBungeeBlitzLevel() && aIsFlagWave)
		{
			for (int _i = 0; _i < 5; _i++)
				PutZombieInWave(ZOMBIE_BUNGEE, aWave, &aZombiePicker);

			if (!aIsFinalWave)
				continue;
		}

		int& aZombiePoints = aZombiePicker.mZombiePoints;
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
		{
			aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave + 10) * 2 / 5 + 1;
		}
		else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
		{
			aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave) * 2 / 5 + 1;
		}
		else if (mApp->IsAdventureMode() && mApp->HasFinishedAdventure() && mLevel != 5)
		{
			aZombiePoints = aWave * 2 / 5 + 1;
		}
		else
		{
			aZombiePoints = aWave / 3 + 1;
		}

		if (aIsFlagWave)
		{
			int aPlainZombiesNum = MIN(aZombiePoints, 8);
			aZombiePoints *= 2.5f;

			if (mApp->mGameMode != GAMEMODE_CHALLENGE_WAR_AND_PEAS && mApp->mGameMode != GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
			{
				for (int _i = 0; _i < aPlainZombiesNum; _i++)
				{
					PutZombieInWave(ZOMBIE_NORMAL, aWave, &aZombiePicker);
				}
				PutZombieInWave(ZOMBIE_FLAG, aWave, &aZombiePicker);
			}
		}

		if (mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN)
		{
			aZombiePoints *= 6;
		}
		else if (mApp->IsLittleTroubleLevel() || mApp->IsWallnutBowlingLevel())
		{
			aZombiePoints *= 4;
		}
		else if (mApp->IsMiniBossLevel())
		{
			aZombiePoints *= 3;
		}
		else if (mApp->IsStormyNightLevel() && mApp->IsAdventureMode())
		{
			aZombiePoints *= 3;
		}
		else if (mApp->IsShovelLevel() || mApp->IsBungeeBlitzLevel() || mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT || mApp->mGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
		{
			aZombiePoints *= 2;
		}

		if (aIntroZombieType != ZOMBIE_INVALID && aIntroZombieType != ZOMBIE_DUCKY_TUBE)
		{
			bool aSpawnIntro = false;
			if ((aIntroZombieType == ZOMBIE_DIGGER || aIntroZombieType == ZOMBIE_BALLOON))
			{
				if (aWave + 1== 7 || aIsFinalWave)
				{
					aSpawnIntro = true;
				}
			}
			else if (aIntroZombieType == ZOMBIE_YETI)
			{
				if (aWave == mNumWaves / 2 && !mApp->mSawYeti)
				{
					aSpawnIntro = true;
				}
			}
			else if (aWave == mNumWaves / 2 || aIsFinalWave)
			{
				aSpawnIntro = true;
			}

			if (aSpawnIntro)
			{
				PutZombieInWave(aIntroZombieType, aWave, &aZombiePicker);
			}
		}

		if (mLevel == 50 && aIsFinalWave)
		{
			PutZombieInWave(ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
		}
		if (mApp->IsAdventureMode() && aIsFinalWave)
		{
			PutInMissingZombies(aWave, &aZombiePicker);
		}
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN)
		{
			if (aWave % 10 == 5)
			{
				for (int _i = 0; _i < 10; _i++)
				{
					PutZombieInWave(ZOMBIE_LADDER, aWave, &aZombiePicker);
				}
			}

			if (aWave % 10 == 8)
			{
				for (int _i = 0; _i < 10; _i++)
				{
					PutZombieInWave(ZOMBIE_JACK_IN_THE_BOX, aWave, &aZombiePicker);
				}
			}

			if (aWave == 19)
			{
				for (int _i = 0; _i < 3; _i++)
				{
					PutZombieInWave(ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
				}
			}
			if (aWave == 29)
			{
				for (int _i = 0; _i < 5; _i++)
				{
					PutZombieInWave(ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
				}
			}
		}

		while (aZombiePoints > 0 && aZombiePicker.mZombieCount < MAX_ZOMBIES_IN_WAVE)
		{
			ZombieType aZombieType = PickZombieType(aZombiePoints, aWave, &aZombiePicker);
			PutZombieInWave(aZombieType, aWave, &aZombiePicker);
		}
	}
}

int Board::GetLevelRandSeed()
{
	int aRndSeed = mApp->mPlayerInfo->mId + mBoardRandSeed;
	if (mApp->IsAdventureMode())
	{
		aRndSeed += mApp->mPlayerInfo->mFinishedAdventure * 101 + mLevel;
	}
	else
	{
		aRndSeed += mChallenge->mSurvivalStage * 101 + mApp->mGameMode;
	}
	return aRndSeed;
}

// GOTY @Patoke: 0x40C9F0
void Board::LoadBackgroundImages()
{
	switch (mBackground)
	{
	case BACKGROUND_1_DAY:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background1";
		if ((mApp->IsAdventureMode() && mLevel <= 4) || mApp->mGameMode == GAMEMODE_CHALLENGE_RESODDED)
		{
			if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_BackgroundUnsodded";
		}
		break;

	case BACKGROUND_2_NIGHT:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background2";
		break;

	case BACKGROUND_3_POOL:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background3";
		break;

	case BACKGROUND_4_FOG:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background4";
		break;

	case BACKGROUND_5_ROOF:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background5";
		break;

	case BACKGROUND_6_BOSS:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Background6";
		break;

	case BACKGROUND_GREENHOUSE:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_GreenHouseGarden";
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_GreenHouseOverlay";
		break;

	case BACKGROUND_TREEOFWISDOM:
		ReanimatorEnsureDefinitionLoaded(REANIM_TREEOFWISDOM, true);
		break;

	case BACKGROUND_ZOMBIQUARIUM:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_Zombiquarium";
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_GreenHouseOverlay";
		break;

	case BACKGROUND_MUSHROOM_GARDEN:
		if (mLoadedResourceCount < 64) mLoadedResourceNames[mLoadedResourceCount++] = (char*)"DelayLoad_MushroomGarden";
		break;

	default:
		TOD_ASSERT(false);
		break;
	}

	for (int i = 0; i < mLoadedResourceCount; i++)
	{
		TodLoadResources(mLoadedResourceNames[i]);
	}
}

void Board::PickBackground()
{
	switch (mApp->mGameMode)
	{
	case GAMEMODE_ADVENTURE:
		if (mLevel <= 1 * LEVELS_PER_AREA)
		{
			mBackground = BACKGROUND_1_DAY;
		}
		else if (mLevel <= 2 * LEVELS_PER_AREA)
		{
			mBackground = BACKGROUND_2_NIGHT;
		}
		else if (mLevel <= 3 * LEVELS_PER_AREA)
		{
			mBackground = BACKGROUND_3_POOL;
		}
		else if (mApp->IsScaryPotterLevel())
		{
			mBackground = BACKGROUND_2_NIGHT;
		}
		else if (mLevel <= 4 * LEVELS_PER_AREA)
		{
			mBackground = BACKGROUND_4_FOG;
		}
		else if (mLevel < FINAL_LEVEL)
		{
			mBackground = BACKGROUND_5_ROOF;
		}
		else if (mLevel == FINAL_LEVEL)
		{
			mBackground = BACKGROUND_6_BOSS;
		}
		else
		{
			mBackground = BACKGROUND_1_DAY;
		}
		break;

	case GAMEMODE_SURVIVAL_NORMAL_STAGE_1:
	case GAMEMODE_SURVIVAL_HARD_STAGE_1:
	case GAMEMODE_SURVIVAL_ENDLESS_STAGE_1:
	case GAMEMODE_CHALLENGE_WAR_AND_PEAS:
	case GAMEMODE_CHALLENGE_WALLNUT_BOWLING:
	case GAMEMODE_CHALLENGE_SLOT_MACHINE:
	case GAMEMODE_CHALLENGE_SEEING_STARS:
	case GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2:
	case GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT:
	case GAMEMODE_CHALLENGE_SUNNY_DAY:
	case GAMEMODE_CHALLENGE_RESODDED:
	case GAMEMODE_CHALLENGE_BIG_TIME:
	case GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER:
	case GAMEMODE_CHALLENGE_ICE:
	case GAMEMODE_CHALLENGE_SHOVEL:
	case GAMEMODE_CHALLENGE_SQUIRREL:
		mBackground = BACKGROUND_1_DAY;
		break;

	case GAMEMODE_SURVIVAL_NORMAL_STAGE_2:
	case GAMEMODE_SURVIVAL_HARD_STAGE_2:
	case GAMEMODE_SURVIVAL_ENDLESS_STAGE_2:
	case GAMEMODE_CHALLENGE_BEGHOULED:
	case GAMEMODE_CHALLENGE_BEGHOULED_TWIST:
	case GAMEMODE_CHALLENGE_PORTAL_COMBAT:
	case GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE:
	case GAMEMODE_CHALLENGE_GRAVE_DANGER:
	case GAMEMODE_SCARY_POTTER_1:
	case GAMEMODE_SCARY_POTTER_2:
	case GAMEMODE_SCARY_POTTER_3:
	case GAMEMODE_SCARY_POTTER_4:
	case GAMEMODE_SCARY_POTTER_5:
	case GAMEMODE_SCARY_POTTER_6:
	case GAMEMODE_SCARY_POTTER_7:
	case GAMEMODE_SCARY_POTTER_8:
	case GAMEMODE_SCARY_POTTER_9:
	case GAMEMODE_SCARY_POTTER_ENDLESS:
	case GAMEMODE_PUZZLE_I_ZOMBIE_1:
	case GAMEMODE_PUZZLE_I_ZOMBIE_2:
	case GAMEMODE_PUZZLE_I_ZOMBIE_3:
	case GAMEMODE_PUZZLE_I_ZOMBIE_4:
	case GAMEMODE_PUZZLE_I_ZOMBIE_5:
	case GAMEMODE_PUZZLE_I_ZOMBIE_6:
	case GAMEMODE_PUZZLE_I_ZOMBIE_7:
	case GAMEMODE_PUZZLE_I_ZOMBIE_8:
	case GAMEMODE_PUZZLE_I_ZOMBIE_9:
	case GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS:
		mBackground = BACKGROUND_2_NIGHT;
		break;

	case GAMEMODE_SURVIVAL_NORMAL_STAGE_3:
	case GAMEMODE_SURVIVAL_HARD_STAGE_3:
	case GAMEMODE_SURVIVAL_ENDLESS_STAGE_3:
	case GAMEMODE_CHALLENGE_LITTLE_TROUBLE:
	case GAMEMODE_CHALLENGE_BOBSLED_BONANZA:
	case GAMEMODE_CHALLENGE_SPEED:
	case GAMEMODE_CHALLENGE_LAST_STAND:
	case GAMEMODE_CHALLENGE_WAR_AND_PEAS_2:
	case GAMEMODE_UPSELL:
	case GAMEMODE_INTRO:
		mBackground = BACKGROUND_3_POOL;
		break;

	case GAMEMODE_SURVIVAL_NORMAL_STAGE_4:
	case GAMEMODE_SURVIVAL_HARD_STAGE_4:
	case GAMEMODE_SURVIVAL_ENDLESS_STAGE_4:
	case GAMEMODE_CHALLENGE_RAINING_SEEDS:
	case GAMEMODE_CHALLENGE_INVISIGHOUL:
	case GAMEMODE_CHALLENGE_AIR_RAID:
	case GAMEMODE_CHALLENGE_STORMY_NIGHT:
		mBackground = BACKGROUND_4_FOG;
		break;

	case GAMEMODE_SURVIVAL_NORMAL_STAGE_5:
	case GAMEMODE_SURVIVAL_HARD_STAGE_5:
	case GAMEMODE_SURVIVAL_ENDLESS_STAGE_5:
	case GAMEMODE_CHALLENGE_COLUMN:
	case GAMEMODE_CHALLENGE_POGO_PARTY:
	case GAMEMODE_CHALLENGE_HIGH_GRAVITY:
	case GAMEMODE_CHALLENGE_BUNGEE_BLITZ:
		mBackground = BACKGROUND_5_ROOF;
		break;

	case GAMEMODE_CHALLENGE_FINAL_BOSS:
		mBackground = BACKGROUND_6_BOSS;
		break;

	case GAMEMODE_CHALLENGE_ZOMBIQUARIUM:
		mBackground = BACKGROUND_ZOMBIQUARIUM;
		break;

	case GAMEMODE_CHALLENGE_ZEN_GARDEN:
		mBackground = BACKGROUND_GREENHOUSE;
		break;

	case GAMEMODE_TREE_OF_WISDOM:
		mBackground = BACKGROUND_TREEOFWISDOM;
		break;

	default:
		TOD_ASSERT(false);
		break;
	}
	LoadBackgroundImages();

	if (mBackground == BACKGROUND_1_DAY || mBackground == BACKGROUND_GREENHOUSE || mBackground == BACKGROUND_TREEOFWISDOM)
	{
		mPlantRow[0] = PLANTROW_NORMAL;
		mPlantRow[1] = PLANTROW_NORMAL;
		mPlantRow[2] = PLANTROW_NORMAL;
		mPlantRow[3] = PLANTROW_NORMAL;
		mPlantRow[4] = PLANTROW_NORMAL;
		mPlantRow[5] = PLANTROW_DIRT;

		if (mApp->IsAdventureMode() && mApp->IsFirstTimeAdventureMode())
		{
			if (mLevel == 1)
			{
				mPlantRow[0] = PLANTROW_DIRT;
				mPlantRow[1] = PLANTROW_DIRT;
				mPlantRow[3] = PLANTROW_DIRT;
				mPlantRow[4] = PLANTROW_DIRT;
			}
			else if (mLevel == 2 || mLevel == 3)
			{
				mPlantRow[0] = PLANTROW_DIRT;
				mPlantRow[4] = PLANTROW_DIRT;
			}
		}
		else if (mApp->mGameMode == GAMEMODE_CHALLENGE_RESODDED)
		{
			mPlantRow[0] = PLANTROW_DIRT;
			mPlantRow[4] = PLANTROW_DIRT;
		}
	}
	else if (mBackground == BACKGROUND_2_NIGHT)
	{
		mPlantRow[0] = PLANTROW_NORMAL;
		mPlantRow[1] = PLANTROW_NORMAL;
		mPlantRow[2] = PLANTROW_NORMAL;
		mPlantRow[3] = PLANTROW_NORMAL;
		mPlantRow[4] = PLANTROW_NORMAL;
		mPlantRow[5] = PLANTROW_DIRT;
	}
	else if (mBackground == BACKGROUND_3_POOL || mBackground == BACKGROUND_ZOMBIQUARIUM || mBackground == BACKGROUND_4_FOG)
	{
		mPlantRow[0] = PLANTROW_NORMAL;
		mPlantRow[1] = PLANTROW_NORMAL;
		mPlantRow[2] = PLANTROW_POOL;
		mPlantRow[3] = PLANTROW_POOL;
		mPlantRow[4] = PLANTROW_NORMAL;
		mPlantRow[5] = PLANTROW_NORMAL;
	}
	else if (mBackground == BACKGROUND_5_ROOF || mBackground == BACKGROUND_6_BOSS)
	{
		mPlantRow[0] = PLANTROW_NORMAL;
		mPlantRow[1] = PLANTROW_NORMAL;
		mPlantRow[2] = PLANTROW_NORMAL;
		mPlantRow[3] = PLANTROW_NORMAL;
		mPlantRow[4] = PLANTROW_NORMAL;
		mPlantRow[5] = PLANTROW_DIRT;
	}
	else
	{
		TOD_ASSERT(false);
	}

	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			if (mPlantRow[y] == PLANTROW_DIRT)
			{
				mGridSquareType[x][y] = GRIDSQUARE_DIRT;
			}
			else if (mPlantRow[y] == PLANTROW_POOL && x >= 0 && x <= 8)
			{
				mGridSquareType[x][y] = GRIDSQUARE_POOL;
			}
			else if (mPlantRow[y] == PLANTROW_HIGH_GROUND && x >= 4 && x <= 8)
			{
				mGridSquareType[x][y] = GRIDSQUARE_HIGH_GROUND;
			}
		}
	}

	MTRand aLevelRNG(GetLevelRandSeed());
	if (StageHasGraveStones())
	{
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_GRAVE_DANGER)
		{
			AddGraveStones(6, RandRangeInt(1, 2), aLevelRNG);
			AddGraveStones(7, RandRangeInt(1, 3), aLevelRNG);
			AddGraveStones(8, RandRangeInt(2, 3), aLevelRNG);
		}
		else if (mApp->IsWhackAZombieLevel())
		{
			mChallenge->WhackAZombiePlaceGraves(9);
		}
		else if (mBackground == BACKGROUND_2_NIGHT)
		{
			if (mApp->IsSurvivalNormal(mApp->mGameMode))
			{
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 1, aLevelRNG);
				AddGraveStones(8, 2, aLevelRNG);
			}
			else if (!mApp->IsAdventureMode())
			{
				AddGraveStones(4, 1, aLevelRNG);
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 2, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel == 11 || mLevel == 12 || mLevel == 13)
			{
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 1, aLevelRNG);
				AddGraveStones(8, 2, aLevelRNG);
			}
			else if (mLevel == 14 || mLevel == 16)
			{
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 2, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel == 17 || mLevel == 18 || mLevel == 19)
			{
				AddGraveStones(4, 1, aLevelRNG);
				AddGraveStones(5, 2, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 3, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel >= 20)
			{
				AddGraveStones(3, 1, aLevelRNG);
				AddGraveStones(4, 2, aLevelRNG);
				AddGraveStones(5, 2, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 3, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else
			{
				TOD_ASSERT(false);
			}
		}
	}
	PickSpecialGraveStone();
}

void Board::InitZombieWavesForLevel(int theForLevel)
{
	if (mApp->IsWhackAZombieLevel() || (mApp->IsWallnutBowlingLevel() && !mApp->IsFirstTimeAdventureMode()))
	{
		mChallenge->InitZombieWaves();
		return;
	}

	for (int aZombieType = ZOMBIE_NORMAL; aZombieType < NUM_ZOMBIE_TYPES; aZombieType++)
	{
		mZombieAllowed[aZombieType] = CanZombieSpawnOnLevel(static_cast<ZombieType>(aZombieType), theForLevel);
	}
}

bool Board::IsZombieWaveDistributionOk()
{
	if (!mApp->IsAdventureMode())
		return true;

	int aZombieTypeCount[NUM_ZOMBIE_TYPES];
	memset(aZombieTypeCount, 0, sizeof(aZombieTypeCount));
	for (int aWave = 0; aWave < mNumWaves; aWave++)
	{
		for (int aIndex = 0; aIndex < MAX_ZOMBIES_IN_WAVE; aIndex++)
		{
			ZombieType aZombieType = mZombiesInWave[aWave][aIndex];
			if (aZombieType == ZOMBIE_INVALID)
			{
				break;
			}

			TOD_ASSERT(aZombieType >= 0 && aZombieType < NUM_ZOMBIE_TYPES);
			aZombieTypeCount[aZombieType]++;
		}
	}

	for (ZombieType aZombieType = ZOMBIE_NORMAL; aZombieType < NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (aZombieType != ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mLevel) && aZombieTypeCount[aZombieType] == 0)
		{
			TodTraceAndLog("Didn't spawn required zombie %s, level %d", GetZombieDefinition(aZombieType).mZombieName, mLevel);
			return false;
		}
	}
	return true;
}

void Board::InitZombieWaves()
{
	memset(mZombieAllowed, false, sizeof(mZombieAllowed));
	if (mApp->IsAdventureMode())
	{
		InitZombieWavesForLevel(mLevel);
	}
	else
	{
		mChallenge->InitZombieWaves();
	}
	PickZombieWaves();
	TOD_ASSERT(IsZombieWaveDistributionOk());

	mCurrentWave = 0;
	mTotalSpawnedWaves = 0;
	mApp->mSawYeti = false;
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 2)
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN * 2;
	}
	else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN_RANGE;
	}
	else
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN_FIRST_WAVE;
	}

	mZombieHealthWaveStart = 0;
	mLastBungeeWave = 0;
	mProgressMeterWidth = 0;
	mHugeWaveCountDown = 0;
	mLevelAwardSpawned = false;
	mZombieCountDownStart = mZombieCountDown;
	mZombieHealthToNextWave = -1;
}

void Board::FreezeEffectsForCutscene(bool theFreeze)
{
	TodParticleSystem* aParticle = NULL;
	while (IterateParticles(aParticle))
	{
		if (aParticle->mEffectType == PARTICLE_GRAVE_BUSTER)
		{
			aParticle->mDontUpdate = theFreeze;
		}
		else if (aParticle->mEffectType == PARTICLE_POOL_SPARKLY && mIceTrapCounter == 0)
		{
			aParticle->mDontUpdate = theFreeze;
		}
	}

	Reanimation* aReanim = NULL;
	while (IterateReanimations(aReanim))
	{
		if (aReanim->mReanimationType == REANIM_SLEEPING)
		{
			aReanim->mAnimRate = theFreeze ? 0.0f : RandRangeFloat(6, 8);
		}
	}
}

void Board::InitSurvivalStage()
{
	RefreshSeedPacketFromCursor();
	mApp->mSoundSystem->GamePause(true);
	FreezeEffectsForCutscene(true);
	mLevelComplete = false;
	InitZombieWaves();
	mApp->mGameScene = SCENE_LEVEL_INTRO;
	mApp->ShowSeedChooserScreen();
	mCutScene->StartLevelIntro();
	mSeedBank->UpdateWidth();

	for (int i = 0; i < SEEDBANK_MAX; i++)
	{
		SeedPacket* aPacket = &mSeedBank->mSeedPackets[i];
		aPacket->mX = GetSeedPacketPositionX(i);
		aPacket->mPacketType = SEED_NONE;
	}

	if (StageHasFog())
	{
		mFogBlownCountDown = FOG_BLOW_RETURN_TIME;
	}
	for (int j = 0; j < MAX_GRID_SIZE_Y; j++)
	{
		mWaveRowGotLawnMowered[j] = -100;
	}
}

Rect Board::GetShovelButtonRect()
{
	Rect aRect(GetSeedBankExtraWidth() + 456, 0, Sexy::IMAGE_SHOVELBANK->GetWidth(), Sexy::IMAGE_SHOVELBANK->GetHeight());
	if (mApp->IsSlotMachineLevel() || mApp->IsSquirrelLevel())
	{
		aRect.mX = 600;
	}
	return aRect;
}

void Board::GetZenButtonRect(GameObjectType theObjectType, Rect& theRect)
{
	theRect.mX = 30;
	if (mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		return;
	}

	bool usable = true;
	for (int anObject = OBJECT_TYPE_WATERING_CAN; anObject <= OBJECT_TYPE_WHEELBARROW; anObject++)
	{
		if (!CanUseGameObject((GameObjectType)anObject))
		{
			usable = false;
			break;
		}
	}
	if (usable)
	{
		theRect.mX = 0;
	}

	int aShovelWidth = Sexy::IMAGE_SHOVELBANK->GetWidth();
	for (int anObject = OBJECT_TYPE_WATERING_CAN; anObject < theObjectType; anObject++)
	{
		if (CanUseGameObject((GameObjectType)anObject))
		{
			theRect.mX += aShovelWidth;
		}
	}
}

// GOTY @Patoke: 0x40D840
void Board::InitLevel()
{
	mMainCounter = 0;
	mEnableGraveStones = false;
	mSodPosition = 0;
	mPrevBoardResult = mApp->mBoardResult;

	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode != GAMEMODE_TREE_OF_WISDOM && aGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mMusic->StopAllMusic();
	}
	mLevel = mApp->IsAdventureMode() ? mApp->mPlayerInfo->mLevel : 0;
	PickBackground();
	InitZombieWaves();
	if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->IsScaryPotterLevel() || mApp->IsWhackAZombieLevel())
	{
		mSunMoney = 0;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
	{
		mSunMoney = 5000;
	}
	else if (mApp->IsIZombieLevel())
	{
		mSunMoney = 150;
	}
	else if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
	{
		mSunMoney = 150;
	}
	else
	{
		mSunMoney = 50;
	}

	memset(mRowPickingArray, 0, sizeof(mRowPickingArray));
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		mWaveRowGotLawnMowered[aRow] = -100;
		mIceMinX[aRow] = BOARD_ICE_START;
		mIceTimer[aRow] = 0;
		mIceParticleID[aRow] = PARTICLESYSTEMID_NULL;
		mRowPickingArray[aRow].mItem = aRow;
	}
	mNumSunsFallen = 0;
	if (!StageIsNight())
	{
		mSunCountDown = RandRangeInt(425, 700);
	}
	memset(mHelpDisplayed, 0, sizeof(mHelpDisplayed));
	mSeedBank->mNumPackets = GetNumSeedsInBank();
	mSeedBank->UpdateWidth();
	for (int i = 0; i < SEEDBANK_MAX; i++)
	{
		SeedPacket* aPacket = &mSeedBank->mSeedPackets[i];
		aPacket->mIndex = i;
		aPacket->mX = GetSeedPacketPositionX(i);
		aPacket->mY = 8;
		aPacket->mPacketType = SEED_NONE;
	}
	if (mApp->IsSlotMachineLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_SUNFLOWER);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_PEASHOOTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_SNOWPEA);
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_ICE)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 6);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_PEASHOOTER);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_CHERRYBOMB);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_WALLNUT);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_REPEATER);
		mSeedBank->mSeedPackets[4].SetPacketType(SEED_SNOWPEA);
		mSeedBank->mSeedPackets[5].SetPacketType(SEED_CHOMPER);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_1)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_FOOTBALL);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_2)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_SCREEN_DOOR);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_PAIL);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_3)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_DIGGER);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_4)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_LADDER);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_5)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_BALLOON);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_6)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_GARGANTUAR);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_7)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_DANCER);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_8)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 6);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[4].SetPacketType(SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[5].SetPacketType(SEED_ZOMBIE_LADDER);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_9)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 8);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[4].SetPacketType(SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[5].SetPacketType(SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[6].SetPacketType(SEED_ZOMBIE_LADDER);
		mSeedBank->mSeedPackets[7].SetPacketType(SEED_ZOMBIE_FOOTBALL);
	}
	else if (aGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 9);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[3].SetPacketType(SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[4].SetPacketType(SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[5].SetPacketType(SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[6].SetPacketType(SEED_ZOMBIE_LADDER);
		mSeedBank->mSeedPackets[7].SetPacketType(SEED_ZOMBIE_FOOTBALL);
		mSeedBank->mSeedPackets[8].SetPacketType(SEED_ZOMBIE_DANCER);
	}
	else if (mApp->IsScaryPotterLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 1);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_CHERRYBOMB);
	}
	else if (mApp->IsWhackAZombieLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_POTATOMINE);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_GRAVEBUSTER);
		mSeedBank->mSeedPackets[2].SetPacketType(mApp->IsAdventureMode() ? SEED_CHERRYBOMB : SEED_ICESHROOM);
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
		mSeedBank->mSeedPackets[0].SetPacketType(SEED_ZOMBIQUARIUM_SNORKLE);
		mSeedBank->mSeedPackets[1].SetPacketType(SEED_ZOMBIQUARIUM_TROPHY);
	}
	else if (!ChooseSeedsOnCurrentLevel() && !HasConveyorBeltSeedBank())
	{
		mSeedBank->mNumPackets = GetNumSeedsInBank();
		for (int i = 0; i < mSeedBank->mNumPackets; i++)
		{
			mSeedBank->mSeedPackets[i].SetPacketType((SeedType)i);
		}
	}
	MarkAllDirty();

	mPaused = false;
	mOutOfMoneyCounter = 0;
	if (StageHasFog())
	{
		mFogBlownCountDown = 200;
		mFogOffset = 1065 - LeftFogColumn() * 80;
	}
	mChallenge->InitLevel();
}

Reanimation* Board::CreateRakeReanim(float theRakeX, float theRakeY, int theRenderOrder)
{
	Reanimation* aReanim = mApp->AddReanimation(theRakeX + 20, theRakeY, theRenderOrder, REANIM_RAKE);
	aReanim->mAnimRate = 0;
	aReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
	aReanim->mIsAttachment = true;
	return aReanim;
}

void Board::PlaceRake()
{
	if (!mApp->mPlayerInfo->mPurchases[STORE_ITEM_RAKE])
		return;

	int aGridX = 7;
	if (mApp->IsScaryPotterLevel())
	{
		GridItem* aGridItem = NULL;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT && aGridItem->mGridX <= aGridX && aGridItem->mGridX > 0)
			{
				aGridX = aGridItem->mGridX - 1;
			}
		}
	}
	else
	{
		if (!StageHasZombieWalkInFromRight() || mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED ||
			mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST || mApp->mGameMode == GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
			return;
	}

	int aPickCount = 0;
	TodWeightedArray aPickArray[MAX_GRID_SIZE_Y];
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (aRow != 5 && mPlantRow[aRow] == PLANTROW_NORMAL)
		{
			aPickArray[aPickCount].mWeight = 1;
			aPickArray[aPickCount].mItem = aRow;
			aPickCount++;
		}
	}
	if (aPickCount == 0)
		return;

	int aGridY = TodPickFromWeightedArray(aPickArray, aPickCount);
	mApp->mPlayerInfo->mPurchases[STORE_ITEM_RAKE]--;
	GridItem* aRake = mGridItems.DataArrayAlloc();
	aRake->mGridItemType = GRIDITEM_RAKE;
	aRake->mGridX = aGridX;
	aRake->mGridY = aGridY;
	aRake->mPosX = GridToPixelX(aGridX, aGridY);
	aRake->mPosY = GridToPixelY(aGridX, aGridY);
	aRake->mRenderOrder = MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, aGridY, 9);
	aRake->mGridItemReanimID = mApp->ReanimationGetID(CreateRakeReanim(aRake->mPosX, aRake->mPosY, 0));
	aRake->mGridItemState = GRIDITEM_STATE_RAKE_ATTRACTING;
}

void Board::InitLawnMowers()
{
	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		aGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GAMEMODE_TREE_OF_WISDOM ||
		aGameMode == GAMEMODE_CHALLENGE_LAST_STAND || aGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
		mApp->IsSquirrelLevel() || mApp->IsIZombieLevel() || (StageHasRoof() && !mApp->mPlayerInfo->mPurchases[STORE_ITEM_ROOF_CLEANER]))
		return;

	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if ((aGameMode == GAMEMODE_CHALLENGE_RESODDED && aRow <= 4) ||
			(mApp->IsAdventureMode() && mLevel == 35) ||
			(!mApp->IsScaryPotterLevel() && mPlantRow[aRow] != PLANTROW_DIRT))
		{
			LawnMower* aLawnMower = mLawnMowers.DataArrayAlloc();
			aLawnMower->LawnMowerInitialize(aRow);
			aLawnMower->mVisible = false;
		}
	}
}

bool Board::ChooseSeedsOnCurrentLevel()
{
	if (mApp->IsChallengeWithoutSeedBank() || HasConveyorBeltSeedBank())
		return false;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
		return false;

	if (mApp->IsIZombieLevel() || mApp->IsSlotMachineLevel())
		return false;

	return (!mApp->IsFirstTimeAdventureMode() || mLevel > 7);
}

// GOTY @Patoke: 0x40E6A0
void Board::StartLevel()
{
	mCoinBankFadeCount = 0;
	mApp->mLastLevelStats->Reset();
	mChallenge->StartLevel();

	unsigned int aSurvivalStage = mApp->mGameMode - GAMEMODE_SURVIVAL_ENDLESS_STAGE_1;
	if (aSurvivalStage <= 4) {
		if (GetSurvivalFlagsCompleted() >= 20) {
			ReportAchievement::GiveAchievement(mApp, Immortal, true);
		}
	}

	if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
	{
		FreezeEffectsForCutscene(false);
		mApp->mSoundSystem->GamePause(false);
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM ||
		mApp->mGameMode == GAMEMODE_UPSELL ||
		mApp->mGameMode == GAMEMODE_INTRO ||
		mApp->IsFinalBossLevel())
		return;

	mApp->mMusic->StartGameMusic();
}

LawnMower* Board::GetBottomLawnMower()
{
	LawnMower* aLawnMower = NULL;
	LawnMower* aBottomMower = NULL;
	while (IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mMowerState == MOWER_TRIGGERED || aLawnMower->mMowerState == MOWER_SQUISHED)
			continue;

		if (aBottomMower == NULL || aBottomMower->mRow < aLawnMower->mRow)
		{
			aBottomMower = aLawnMower;
		}
	}
	return aBottomMower;
}

// GOTY @Patoke: 0x40E860
void Board::UpdateLevelEndSequence()
{
	if (mNextSurvivalStageCounter > 0)
	{
		if (!IsScaryPotterDaveTalking())
		{
			mNextSurvivalStageCounter--;
			if (mApp->IsAdventureMode() && mApp->IsScaryPotterLevel() && mNextSurvivalStageCounter == 300)
			{
				mApp->CrazyDaveEnter();
				mApp->CrazyDaveTalkIndex(mChallenge->mSurvivalStage == 0 ? 2700 : 2800);
				mChallenge->PuzzleNextStageClear();
				mNextSurvivalStageCounter = 100;
			}
		}

		if (mNextSurvivalStageCounter == 1 && mApp->IsSurvivalMode())
		{
			TryToSaveGame();
		}

		if (!mNextSurvivalStageCounter)
		{
			if (mApp->IsScaryPotterLevel())
			{
				if (mApp->IsAdventureMode())
					return;

				if (!IsFinalScaryPotterStage())
				{
					mChallenge->PuzzleNextStageClear();
					mChallenge->ScaryPotterPopulate();
				}
			}
			else if (LawnApp::IsEndlessIZombie(mApp->mGameMode))
			{
				mChallenge->PuzzleNextStageClear();
				mChallenge->IZombieInitLevel();
			}
			else if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
			{
				ClearAdvice(ADVICE_NONE);
			}
			else
			{
				mLevelComplete = true;
				RemoveZombiesForRepick();
			}
			return;
		}
	}

	if (mBoardFadeOutCounter < 0)
		return;

	mBoardFadeOutCounter--;
	if (mBoardFadeOutCounter == 0)
	{
		mLevelComplete = true;
		return;
	}
	if (mBoardFadeOutCounter == 300)
	{
		if (!IsSurvivalStageWithRepick() && !(mLevel == 9 || mLevel == 19 || mLevel == 29 || mLevel == 39 || mLevel == 49))
		{
			mApp->PlaySample(SOUND_LIGHTFILL);
		}
	}

	if (mScoreNextMowerCounter > 0)
	{
		mScoreNextMowerCounter--;
		if (mScoreNextMowerCounter)
		{
			return;
		}
	}

	if (CanDropLoot() && !IsSurvivalStageWithRepick())
	{
		mScoreNextMowerCounter = 40;
		LawnMower* aLawnMower = GetBottomLawnMower();
		if (aLawnMower)
		{
			AddCoin(aLawnMower->mPosX + 40, aLawnMower->mPosY + 40, COIN_GOLD, COIN_MOTION_LAWNMOWER_COIN);
			SoundInstance* aSoundInstance = mApp->mSoundManager->GetSoundInstance(SOUND_POINTS);
			if (aSoundInstance)
			{
				aSoundInstance->Play(false, true);
				float aPitch = ClampFloat(6 - CountUntriggerLawnMowers(), 0.0f, 6.0f);
				aSoundInstance->AdjustPitch(aPitch);
			}
			aLawnMower->Die();
		}
	}
}

void Board::CompleteEndLevelSequenceForSaving()
{
	if (CanDropLoot())
	{
		LawnMower* aLawnMower = NULL;
		while (IterateLawnMowers(aLawnMower))
		{
			if (aLawnMower->mMowerState != MOWER_TRIGGERED && aLawnMower->mMowerState != MOWER_SQUISHED)
			{
				int aCoinValue = Coin::GetCoinValue(COIN_GOLD);
				mApp->mPlayerInfo->AddCoins(aCoinValue);
				mCoinsCollected += aCoinValue;
			}
		}
	}

	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mIsBeingCollected)
		{
			aCoin->ScoreCoin();
		}
		else
		{
			aCoin->Die();
		}
	}

	mApp->UpdatePlayerProfileForFinishingLevel();
}

void Board::FadeOutLevel()
{
	if (mApp->mGameScene != SCENE_PLAYING)
	{
		RefreshSeedPacketFromCursor();
		mApp->mLastLevelStats->Reset();
		mLevelComplete = true;
	}

	bool aNeedSoundEffect = true;
	if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage())
	{
		aNeedSoundEffect = false;
	}
	else if (IsSurvivalStageWithRepick() || IsLastStandStageWithRepick() || mApp->IsEndlessIZombie(mApp->mGameMode))
	{
		aNeedSoundEffect = false;
	}
	if (aNeedSoundEffect)
	{
		mApp->mMusic->StopAllMusic();
		if (mApp->IsAdventureMode() && mLevel == 50)
		{
			mApp->PlayFoley(FOLEY_FINAL_FANFARE);
		}
		else if (mApp->TrophiesNeedForGoldSunflower() == 1)
		{
			mApp->PlayFoley(FOLEY_FINAL_FANFARE);
		}
		else
		{
			mApp->PlayFoley(FOLEY_WINMUSIC);
		}
	}

	if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage())
	{
		mNextSurvivalStageCounter = 500;
		if (mApp->IsAdventureMode())
		{
			ClearAdvice(ADVICE_NONE);
		}
		else
		{
			mLevelAwardSpawned = true;
			std::string aStreakStr = mApp->IsEndlessScaryPotter(mApp->mGameMode) ? "[ADVICE_MORE_SCARY_POTS]" : "[ADVICE_3_IN_A_ROW]";
			std::string aMessage = TodReplaceNumberString(aStreakStr, "{STREAK}", mChallenge->mSurvivalStage + 1);
			PuzzleSaveStreak();
			ClearAdvice(ADVICE_NONE);
			DisplayAdvice(aMessage, MESSAGE_STYLE_BIG_MIDDLE, ADVICE_NONE);
		}
		return;
	}

	if (mApp->IsEndlessIZombie(mApp->mGameMode))
	{
		mNextSurvivalStageCounter = 500;
		std::string aMessage = TodReplaceNumberString("[ADVICE_MORE_IZOMBIE]", "{STREAK}", mChallenge->mSurvivalStage + 1);
		PuzzleSaveStreak();
		ClearAdvice(ADVICE_NONE);
		DisplayAdvice(aMessage, MESSAGE_STYLE_BIG_MIDDLE, ADVICE_NONE);
		return;
	}

	if (IsLastStandStageWithRepick())
	{
		mNextSurvivalStageCounter = 500;
		mChallenge->LastStandCompletedStage();
		return;
	}

	if (!IsSurvivalStageWithRepick())
	{
		RefreshSeedPacketFromCursor();
		mApp->mLastLevelStats->mUnusedLawnMowers = CountUntriggerLawnMowers();

		mBoardFadeOutCounter = 600;
		if (mLevel == 9 || mLevel == 19 || mLevel == 29 || mLevel == 39 || mLevel == 49)
		{
			mBoardFadeOutCounter = 500;
		}

		if (CanDropLoot())
		{
			mScoreNextMowerCounter = 200;
		}

		Coin* aCoin = NULL;
		while (IterateCoins(aCoin))
		{
			aCoin->TryAutoCollectAfterLevelAward();
		}
	}
	else
	{
		TOD_ASSERT(mApp->IsSurvivalMode());
		mNextSurvivalStageCounter = 500;
		DisplayAdvice("[ADVICE_MORE_ZOMBIES]", MESSAGE_STYLE_BIG_MIDDLE, ADVICE_NONE);
		mApp->mMusic->FadeOut(500);
		mApp->PlaySample(SOUND_HUGE_WAVE);
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
		{
			mIceTimer[aRow] = mNextSurvivalStageCounter;
		}
	}

	mApp->SetCursor(CURSOR_POINTER);
}

void Board::DisplayAdvice(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex)
{
	if (theHelpIndex != ADVICE_NONE)
	{
		if (mHelpDisplayed[theHelpIndex])
			return;

		mHelpDisplayed[theHelpIndex] = true;
	}

	mAdvice->SetLabel(theAdvice, theMessageStyle);
	mHelpIndex = theHelpIndex;
}

void Board::DisplayAdviceAgain(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex)
{
	if (theHelpIndex != ADVICE_NONE)
	{
		mHelpDisplayed[theHelpIndex] = false;
	}
	DisplayAdvice(theAdvice, theMessageStyle, theHelpIndex);
}

void Board::ClearAdviceImmediately()
{
	ClearAdvice(ADVICE_NONE);
	mAdvice->mDuration = 0;
}

void Board::ClearAdvice(AdviceType theHelpIndex)
{
	if (theHelpIndex == ADVICE_NONE || theHelpIndex == mHelpIndex)
	{
		mAdvice->ClearLabel();
		mHelpIndex = ADVICE_NONE;
	}
}

Coin* Board::AddCoin(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion)
{
	Coin* aCoin = mCoins.DataArrayAlloc();
	aCoin->CoinInitialize(theX, theY, theCoinType, theCoinMotion);
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
	{
		DisplayAdvice("[ADVICE_CLICK_ON_SUN]", MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, ADVICE_CLICK_ON_SUN);
	}
	return aCoin;
}

bool Board::IsPlantInCursor()
{
	return
		mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK ||
		mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_USABLE_COIN ||
		mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_GLOVE ||
		mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_DUPLICATOR ||
		mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW;
}

// GOTY @Patoke: 0x40F600
SeedType Board::GetSeedTypeInCursor()
{
	if (mCursorObject->mCursorType == CURSOR_TYPE_WHEEELBARROW)
	{
		PottedPlant* aPottedPlant = reinterpret_cast<PottedPlant*>(mApp->mZenGarden->GetPottedPlantInWheelbarrow());
		if (aPottedPlant)
		{
			return static_cast<SeedType>(aPottedPlant->mSeedType);
		}
	}

	if (!IsPlantInCursor())
	{
		return SEED_NONE;
	}
	return static_cast<SeedType>(mCursorObject->mType == SEED_IMITATER ? mCursorObject->mImitaterType : mCursorObject->mType);
}

void Board::RefreshSeedPacketFromCursor()
{
	if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_USABLE_COIN)
	{
		mCoins.DataArrayTryToGet(mCursorObject->mCoinID)->DroppedUsableSeed();
	}
	else if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
	{
		TOD_ASSERT(mCursorObject->mSeedBankIndex >= 0 && mCursorObject->mSeedBankIndex < mSeedBank->mNumPackets);
		mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
	}
	ClearCursor();
}

bool Board::IsPoolSquare(int theGridX, int theGridY)
{
	if (theGridX >= 0 && theGridY >= 0)
	{
		TOD_ASSERT(theGridX < MAX_GRID_SIZE_X && theGridY < MAX_GRID_SIZE_Y);
		return mGridSquareType[theGridX][theGridY] == GRIDSQUARE_POOL;
	}
	return false;
}

Plant* Board::NewPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
	Plant* aPlant = mPlants.DataArrayAlloc();
	aPlant->mIsOnBoard = true;
	aPlant->PlantInitialize(theGridX, theGridY, theSeedType, theImitaterType);
	return aPlant;
}

void Board::DoPlantingEffects(int theGridX, int theGridY, Plant* thePlant)
{
	int aXPos = GridToPixelX(theGridX, theGridY) + 41;
	int aYPos = GridToPixelY(theGridX, theGridY) + 74;
	if (thePlant)
	{
		if (thePlant->mSeedType == SEED_LILYPAD)
		{
			aYPos += 15;
		}
		else if (thePlant->mSeedType == SEED_FLOWERPOT)
		{
			aYPos += 30;
		}
	}

	if (mBackground == BACKGROUND_GREENHOUSE)
	{
		mApp->PlayFoley(FOLEY_CERAMIC);
		return;
	}
	if (mBackground == BACKGROUND_ZOMBIQUARIUM)
	{
		mApp->PlayFoley(FOLEY_PLANT_WATER);
		return;
	}
	if (Plant::IsFlying(thePlant->mSeedType))
	{
		mApp->PlayFoley(FOLEY_PLANT);
		return;
	}

	if (IsPoolSquare(theGridX, theGridY))
	{
		mApp->PlayFoley(FOLEY_PLANT_WATER);
		mApp->AddTodParticle(aXPos, aYPos, RENDER_LAYER_TOP, PARTICLE_PLANTING_POOL);
	}
	else
	{
		mApp->PlayFoley(FOLEY_PLANT);
		mApp->AddTodParticle(aXPos, aYPos, RENDER_LAYER_TOP, PARTICLE_PLANTING);
	}
}

// GOTY @Patoke: 0x40FA10
Plant* Board::AddPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
	Plant* aPlant = NewPlant(theGridX, theGridY, theSeedType, theImitaterType);
	DoPlantingEffects(theGridX, theGridY, aPlant);
	mChallenge->PlantAdded(aPlant);

	int aSunPlantsCount = CountPlantByType(SEED_SUNSHROOM) + CountPlantByType(SEED_SUNFLOWER);
	if (aSunPlantsCount > mMaxSunPlants)
	{
		mMaxSunPlants = aSunPlantsCount;
	}

	if (theSeedType == SEED_PEASHOOTER ||
		theSeedType == SEED_SNOWPEA ||
		theSeedType == SEED_REPEATER ||
		theSeedType == SEED_THREEPEATER ||
		theSeedType == SEED_SPLITPEA ||
		theSeedType == SEED_GATLINGPEA)
	{
		mPeaShooterUsed = true;
	}
	if (theSeedType == SEED_CABBAGEPULT ||
		theSeedType == SEED_KERNELPULT ||
		theSeedType == SEED_MELONPULT ||
		theSeedType == SEED_WINTERMELON)
	{
		mCatapultPlantsUsed = true;
	}

	bool aIsFungi = Plant::IsFungus(theSeedType);
	if (!Plant::IsFlying(theSeedType) && !aIsFungi) {
		mMushroomAndCoffeeBeansOnly = false;
	}
	if (aIsFungi) {
		mMushroomsUsed = true;
	}

	return aPlant;
}

// GOTY @Patoke: 0x40FBA0
Plant* Board::GetPumpkinAt(int theGridX, int theGridY)
{
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() && aPlant->mSeedType == SEED_PUMPKINSHELL)
		{
			return aPlant;
		}
	}
	return NULL;
}

Plant* Board::GetFlowerPotAt(int theGridX, int theGridY)
{
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() && aPlant->mSeedType == SEED_FLOWERPOT)
		{
			return aPlant;
		}
	}
	return NULL;
}

void Board::GetPlantsOnLawn(int theGridX, int theGridY, PlantsOnLawn* thePlantOnLawn)
{
	thePlantOnLawn->mUnderPlant = NULL;
	thePlantOnLawn->mPumpkinPlant = NULL;
	thePlantOnLawn->mFlyingPlant = NULL;
	thePlantOnLawn->mNormalPlant = NULL;

	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
		return;

	if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		return;

	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		SeedType aSeedType = aPlant->mSeedType;
		if (aSeedType == SEED_IMITATER && aPlant->mImitaterType != SEED_NONE)
		{
			aSeedType = aPlant->mImitaterType;
		}

		if (aPlant->mRow != theGridY)
		{
			continue;
		}
		if (aSeedType == SEED_COBCANNON)
		{
			if (aPlant->mPlantCol < theGridX - 1 || aPlant->mPlantCol > theGridX)
			{
				continue;
			}
		}
		else
		{
			if (aPlant->mPlantCol != theGridX)
			{
				continue;
			}
		}
		if (aPlant->NotOnGround())
		{
			continue;
		}

		if (Plant::IsFlying(aSeedType))
		{
			TOD_ASSERT(!thePlantOnLawn->mFlyingPlant);
			thePlantOnLawn->mFlyingPlant = aPlant;
		}
		else if (aSeedType == SEED_FLOWERPOT || (aSeedType == SEED_LILYPAD && mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN))
		{
			TOD_ASSERT(!thePlantOnLawn->mUnderPlant);
			thePlantOnLawn->mUnderPlant = aPlant;
		}
		else if (aSeedType == SEED_PUMPKINSHELL)
		{
			TOD_ASSERT(!thePlantOnLawn->mPumpkinPlant);
			thePlantOnLawn->mPumpkinPlant = aPlant;
		}
		else
		{
			TOD_ASSERT(!thePlantOnLawn->mNormalPlant);
			thePlantOnLawn->mNormalPlant = aPlant;
		}
	}
}

Plant* Board::GetTopPlantAt(int theGridX, int theGridY, PlantPriority thePriority)
{
	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
		return NULL;

	if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		return NULL;

	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);

	switch (thePriority)
	{
	case TOPPLANT_EATING_ORDER:
		if (aPlantOnLawn.mPumpkinPlant)							return aPlantOnLawn.mPumpkinPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case TOPPLANT_DIGGING_ORDER:
		if (aPlantOnLawn.mNormalPlant)							return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case TOPPLANT_BUNGEE_ORDER:
	case TOPPLANT_CATAPULT_ORDER:
	case TOPPLANT_ANY:
		if (aPlantOnLawn.mFlyingPlant)							return aPlantOnLawn.mFlyingPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else if (aPlantOnLawn.mPumpkinPlant)					return aPlantOnLawn.mPumpkinPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case TOPPLANT_ZEN_TOOL_ORDER:
		if (aPlantOnLawn.mFlyingPlant)							return aPlantOnLawn.mFlyingPlant;
		else if (aPlantOnLawn.mPumpkinPlant)					return aPlantOnLawn.mPumpkinPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case TOPPLANT_ONLY_NORMAL_POSITION:			return aPlantOnLawn.mNormalPlant;
	case TOPPLANT_ONLY_FLYING:					return aPlantOnLawn.mFlyingPlant;
	case TOPPLANT_ONLY_PUMPKIN:					return aPlantOnLawn.mPumpkinPlant;
	case TOPPLANT_ONLY_UNDER_PLANT:				return aPlantOnLawn.mUnderPlant;
	default:									TOD_ASSERT(false);
	}
	return NULL;
}

int Board::CountSunFlowers()
{
	int aCount = 0;
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->MakesSun())
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountPlantByType(SeedType theSeedType)
{
	int aCount = 0;
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == theSeedType)
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountEmptyPotsOrLilies(SeedType theSeedType)
{
	int aCount = 0;
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == theSeedType && !GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, TOPPLANT_ONLY_NORMAL_POSITION))
		{
			aCount++;
		}
	}
	return aCount;
}

bool Board::IsValidCobCannonSpotHelper(int theGridX, int theGridY)
{
	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);
	if (aPlantOnLawn.mPumpkinPlant)
		return false;

	if (aPlantOnLawn.mNormalPlant && aPlantOnLawn.mNormalPlant->mSeedType == SEED_KERNELPULT)
		return true;

	return mApp->mEasyPlantingCheat && CanPlantAt(theGridX, theGridY, SEED_KERNELPULT) == PLANTING_OK;
}

bool Board::IsValidCobCannonSpot(int theGridX, int theGridY)
{
	if (!IsValidCobCannonSpotHelper(theGridX, theGridY) || !IsValidCobCannonSpotHelper(theGridX + 1, theGridY))
		return false;

	return !GetFlowerPotAt(theGridX, theGridY) == !GetFlowerPotAt(theGridX + 1, theGridY);
}

bool Board::HasValidCobCannonSpot()
{
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SEED_KERNELPULT && IsValidCobCannonSpot(aPlant->mPlantCol, aPlant->mRow))
		{
			return true;
		}
	}
	return false;
}

Projectile* Board::AddProjectile(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType)
{
	Projectile* aProjectile = mProjectiles.DataArrayAlloc();
	aProjectile->ProjectileInitialize(theX, theY, theRenderOrder, theRow, theProjectileType);
	return aProjectile;
}

bool Board::CanZombieSpawnOnLevel(ZombieType theZombieType, int theLevel)
{
	const ZombieDefinition& aZombieDef = GetZombieDefinition(theZombieType);
	if (theZombieType == ZOMBIE_YETI)
	{
		return gLawnApp->CanSpawnYetis();
	}

	if (theLevel < aZombieDef.mStartingLevel || aZombieDef.mPickWeight == 0)
	{
		return false;
	}

	TOD_ASSERT(gZombieAllowedLevels[theZombieType].mZombieType == theZombieType);
	return gZombieAllowedLevels[theZombieType].mAllowedOnLevel[ClampInt(theLevel - 1, 0, 49)];
}

ZombieType Board::GetIntroducedZombieType()
{
	if (!mApp->IsAdventureMode() || mLevel == 1)
	{
		return ZOMBIE_INVALID;
	}

	for (ZombieType aZombieType = ZOMBIE_NORMAL; aZombieType < NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		const ZombieDefinition& aZombieDef = GetZombieDefinition(aZombieType);
		if ((aZombieType != ZOMBIE_YETI || mApp->CanSpawnYetis()) && aZombieDef.mStartingLevel == mLevel)
		{
			return aZombieType;
		}
	}
	return ZOMBIE_INVALID;
}

ZombieType Board::PickGraveRisingZombieType()
{
	TodWeightedArray aZombieWeightArray[NUM_ZOMBIE_TYPES];
	int aCount = 2;
	aZombieWeightArray[0].mItem = ZOMBIE_NORMAL;
	aZombieWeightArray[0].mWeight = GetZombieDefinition(ZOMBIE_NORMAL).mPickWeight;
	aZombieWeightArray[1].mItem = ZOMBIE_TRAFFIC_CONE;
	aZombieWeightArray[1].mWeight = GetZombieDefinition(ZOMBIE_TRAFFIC_CONE).mPickWeight;
	if (!StageHasGraveStones())
	{
		aZombieWeightArray[2].mItem = ZOMBIE_PAIL;
		aZombieWeightArray[2].mWeight = GetZombieDefinition(ZOMBIE_PAIL).mPickWeight;
		aCount++;
	}

	for (int i = 0; i < aCount; i++)
	{
		ZombieType aZombieType = static_cast<ZombieType>(aZombieWeightArray[i].mItem);
		const ZombieDefinition& aZombieDef = GetZombieDefinition(aZombieType);
		if ((mApp->IsFirstTimeAdventureMode() && mLevel < aZombieDef.mStartingLevel) || (!mZombieAllowed[aZombieType] && aZombieType != ZOMBIE_NORMAL))
		{
			aZombieWeightArray[i].mWeight = 0;
		}
	}

	return (ZombieType)TodPickFromWeightedArray(aZombieWeightArray, aCount);
}

ZombieType Board::PickZombieType(int theZombiePoints, int theWaveIndex, ZombiePicker* theZombiePicker)
{
	int aPickCount = 0;
	TodWeightedArray aZombieWeightArray[NUM_ZOMBIE_TYPES];
	for (int aZombieType = ZOMBIE_NORMAL; aZombieType < NUM_ZOMBIE_TYPES; aZombieType++)
	{
		if (!mZombieAllowed[aZombieType])
			continue;

		const ZombieDefinition& aZombieDef = GetZombieDefinition((ZombieType)aZombieType);

		GameMode aGameMode = mApp->mGameMode;
		if (aZombieType == ZOMBIE_BUNGEE && mApp->IsSurvivalEndless(aGameMode))
		{
			if (!IsFlagWave(theWaveIndex))
			{
				continue;
			}
		}
		else if (aGameMode != GAMEMODE_CHALLENGE_POGO_PARTY && aGameMode != GAMEMODE_CHALLENGE_BOBSLED_BONANZA && aGameMode != GAMEMODE_CHALLENGE_AIR_RAID)
		{
			int aFirstAllowedWave = aZombieDef.mFirstAllowedWave;
			if (mApp->IsSurvivalEndless(aGameMode))
			{
				int aFlags = GetSurvivalFlagsCompleted();
				int aAllowedWave = aFirstAllowedWave - TodAnimateCurve(18, 50, aFlags, 0, 15, CURVE_LINEAR);
				aFirstAllowedWave = MAX(aAllowedWave, 1);
			}
			if (theWaveIndex + 1 < aFirstAllowedWave || theZombiePoints < aZombieDef.mZombieValue)
			{
				continue;
			}
		}

		int aPickWeight = aZombieDef.mPickWeight;
		if (mApp->IsSurvivalMode())
		{
			int aFlags = GetSurvivalFlagsCompleted();
			if (aZombieType == ZOMBIE_GARGANTUAR || aZombieType == ZOMBIE_ZAMBONI)
			{
				if (theZombiePicker->mZombieTypeCount[aZombieType] >= TodAnimateCurve(10, 50, aFlags, 2, 50, CURVE_LINEAR))
				{
					continue;
				}
			}
			else if (aZombieType == ZOMBIE_REDEYE_GARGANTUAR)
			{
				if (IsFlagWave(theWaveIndex))
				{
					if (theZombiePicker->mZombieTypeCount[aZombieType] >= TodAnimateCurve(14, 100, aFlags, 1, 50, CURVE_LINEAR))
					{
						continue;
					}
				}
				else
				{
					if (theZombiePicker->mAllWavesZombieTypeCount[aZombieType] >= TodAnimateCurve(10, 110, aFlags, 1, 50, CURVE_LINEAR))
					{
						continue;
					}
					aPickWeight = 1000;
				}
			}
			else if (aZombieType == ZOMBIE_NORMAL)
			{
				aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 10, CURVE_LINEAR);
			}
			else if (aZombieType == ZOMBIE_TRAFFIC_CONE)
			{
				aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 4, CURVE_LINEAR);
			}
		}
		aZombieWeightArray[aPickCount].mItem = aZombieType;
		aZombieWeightArray[aPickCount].mWeight = aPickWeight;
		aPickCount++;
	}

	return (ZombieType)TodPickFromWeightedArray(aZombieWeightArray, aPickCount);
}

bool Board::IsZombieTypePoolOnly(ZombieType theZombieType)
{
	return (theZombieType == ZOMBIE_SNORKEL || theZombieType == ZOMBIE_DOLPHIN_RIDER);
}

bool Board::RowCanHaveZombieType(int theRow, ZombieType theZombieType)
{
	if (!RowCanHaveZombies(theRow))
	{
		return false;
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_RESODDED && mPlantRow[theRow] == PLANTROW_DIRT && mCurrentWave < 5)
	{
		return false;
	}
	if (mPlantRow[theRow] == PLANTROW_POOL && !Zombie::ZombieTypeCanGoInPool(theZombieType) && theZombieType != ZOMBIE_BALLOON)
	{
		return false;
	}
	if (mPlantRow[theRow] == PLANTROW_HIGH_GROUND && !Zombie::ZombieTypeCanGoOnHighGround(theZombieType))
	{
		return false;
	}

	int aCurrentWave = mCurrentWave;
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
	{
		aCurrentWave += mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage();
	}
	if (mPlantRow[theRow] == PLANTROW_POOL)
	{
		if (aCurrentWave < 5 && !IsZombieTypePoolOnly(theZombieType))
		{
			return false;
		}
	}
	else if (IsZombieTypePoolOnly(theZombieType))
	{
		return false;
	}
	if (theZombieType == ZOMBIE_BOBSLED && !mIceTimer[theRow])
	{
		return false;
	}
	if (theRow == 0 && !mApp->IsSurvivalMode())
	{
		if (theZombieType == ZOMBIE_GARGANTUAR || theZombieType == ZOMBIE_REDEYE_GARGANTUAR)
		{
			return false;
		}
	}
	if (theZombieType != ZOMBIE_DANCER || StageHasPool())
	{
		return true;
	}
	return RowCanHaveZombies(theRow - 1) && RowCanHaveZombies(theRow + 1);
}

int Board::PickRowForNewZombie(ZombieType theZombieType)
{
	GridItem* aRake = GetRake();
	if (aRake && aRake->mGridItemState == GRIDITEM_STATE_RAKE_ATTRACTING && RowCanHaveZombieType(aRake->mGridY, theZombieType))
	{
		aRake->mGridItemState = GRIDITEM_STATE_RAKE_WAITING;
		TodUpdateSmoothArrayPick(mRowPickingArray, MAX_GRID_SIZE_Y, aRake->mGridY);
		return aRake->mGridY;
	}

	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (!RowCanHaveZombieType(aRow, theZombieType))
		{
			mRowPickingArray[aRow].mWeight = 0;
		}
		else if (mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
		{
			mRowPickingArray[aRow].mWeight = mChallenge->PortalCombatRowSpawnWeight(aRow);
		}
		else if (mApp->mGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL && mCurrentWave <= 3 && aRow == 5)
		{
			mRowPickingArray[aRow].mWeight = 0;
		}
		else
		{
			int aWavesMowered = mCurrentWave - mWaveRowGotLawnMowered[aRow];
			if (mApp->IsContinuousChallenge() && mCurrentWave == mNumWaves - 1)
			{
				aWavesMowered = 100;
			}

			if (aWavesMowered <= 1)
			{
				mRowPickingArray[aRow].mWeight = 0.01f;
			}
			else if (aWavesMowered <= 2)
			{
				mRowPickingArray[aRow].mWeight = 0.5f;
			}
			else
			{
				mRowPickingArray[aRow].mWeight = 1.0f;
			}
		}
	}
	return TodPickFromSmoothArray(mRowPickingArray, MAX_GRID_SIZE_Y);
}

bool Board::CanAddBobSled()
{
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (mIceTimer[aRow] > 0 && mIceMinX[aRow] < 700)
		{
			return true;
		}
	}
	return false;
}

// GOTY @Patoke: 0x410700
Zombie* Board::AddZombieInRow(ZombieType theZombieType, int theRow, int theFromWave)
{
	if (mZombies.mSize >= mZombies.mMaxSize - 1)
	{
		TodTrace("Too many zombies!!");
		return NULL;
	}

	if (theZombieType == ZOMBIE_YETI) {
		if (mApp->IsAdventureMode() && mLevel == 40 && theFromWave >= 0)
			ReportAchievement::GiveAchievement(mApp, Zombologist, true);
	}

	bool aVariant = !Rand(5);
	Zombie* aZombie = mZombies.DataArrayAlloc();
	aZombie->ZombieInitialize(theRow, theZombieType, aVariant, NULL, theFromWave);
	if (theZombieType == ZOMBIE_BOBSLED && aZombie->IsOnBoard())
	{
		for (int _i = 0; _i < 3; _i++)
		{
			mZombies.DataArrayAlloc()->ZombieInitialize(theRow, ZOMBIE_BOBSLED, false, aZombie, theFromWave);
		}
	}
	return aZombie;
}

Zombie* Board::AddZombie(ZombieType theZombieType, int theFromWave)
{
	return AddZombieInRow(theZombieType, PickRowForNewZombie(theZombieType), theFromWave);
}

void Board::RemoveAllZombies()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		aZombie->mDead = true;
	}
}

void Board::RemoveZombiesForRepick()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying())
		{
			float aX = aZombie->PosX();
			float aY = aZombie->PosY();
			mApp->AddTodParticle(aX, aY, RENDER_LAYER_TOP, PARTICLE_ZOMBIE_SURPRISE);
			aZombie->Die();
		}
	}
}

void Board::RemoveCutsceneZombies()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mZombieType == ZOMBIE_CUTSCENE)
		{
			aZombie->Die();
		}
	}
}

bool Board::IsIceAt(int theGridX, int theRow)
{
	if (theRow >= 0 && theRow < MAX_GRID_SIZE_Y && theGridX >= 0 && theGridX < MAX_GRID_SIZE_X && mIceTimer[theRow] > 0)
	{
		return true;
	}
	return false;
}

bool Board::CanPlantAt(int theGridX, int theGridY, SeedType theSeedType)
{
	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X)
		return PLANTING_NOT_ON_LAWN;

	if (theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
		return PLANTING_NOT_ON_LAWN;

	int aGridSquare = mGridSquareType[theGridX][theGridY];

	if (aGridSquare == GRIDSQUARE_POOL && !Plant::IsAquatic(theSeedType) && theSeedType != SEED_LILYPAD)
		return PLANTING_NOT_ON_LAWN;

	if (aGridSquare == GRIDSQUARE_DIRT)
		return PLANTING_NOT_ON_LAWN;

	if (aGridSquare == GRIDSQUARE_HIGH_GROUND && !Plant::CanPlaceOnHighGround(theSeedType))
		return PLANTING_NOT_ON_LAWN;

	if (mChallenge->IsZombieSeed(theSeedType))
	{
		if (mGridSquareType[theGridX][theGridY] == GRIDSQUARE_POOL && !Plant::IsAquatic(theSeedType) && theSeedType != SEED_LILYPAD)
			return PLANTING_NOT_ON_LAWN;

		if (GetGraveStoneAt(theGridX, theGridY) || GetCraterAt(theGridX, theGridY) || GetLadderAt(theGridX, theGridY))
			return PLANTING_NOT_ON_LAWN;

		GridItem* aGridItem = NULL;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT && aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY)
				return PLANTING_NOT_ON_LAWN;
		}
	}

	if (GetGraveStoneAt(theGridX, theGridY) && theSeedType != SEED_GRAVEBUSTER && theSeedType != SEED_TANGLEKELP)
		return PLANTING_NOT_ON_GRAVE;

	if (GetCraterAt(theGridX, theGridY))
		return PLANTING_NOT_ON_CRATER;

	if (theSeedType == SEED_LILYPAD && IsPoolSquare(theGridX, theGridY) && GetFlowerPotAt(theGridX, theGridY))
		return PLANTING_NOT_ON_POT;

	if (GetTopPlantAt(theGridX, theGridY, TOPPLANT_DIGGING_ORDER))
	{
		if (Plant::IsFlying(theSeedType))
			return PLANTING_OK;
		else
			return PLANTING_NOT_PASSED;
	}

	if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		return PLANTING_OK;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND && (mNextSurvivalStageCounter > 0 || mBoardFadeOutCounter >= 0))
		return PLANTING_OK;

	if (theSeedType == SEED_FLOWERPOT && GetFlowerPotAt(theGridX, theGridY))
		return PLANTING_OK;

	if (theSeedType == SEED_LILYPAD && GetTopPlantAt(theGridX, theGridY, TOPPLANT_ONLY_NORMAL_POSITION))
		return PLANTING_OK;

	if (aGridSquare == GRIDSQUARE_POOL && Plant::IsAquatic(theSeedType) && !GetFlowerPotAt(theGridX, theGridY))
		return PLANTING_OK;

	if (aGridSquare == GRIDSQUARE_POOL && theSeedType == SEED_LILYPAD && !GetTopPlantAt(theGridX, theGridY, TOPPLANT_ONLY_UNDER_PLANT))
		return PLANTING_OK;

	if (aGridSquare == GRIDSQUARE_HIGH_GROUND && Plant::CanPlaceOnHighGround(theSeedType) && !GetFlowerPotAt(theGridX, theGridY))
		return PLANTING_OK;

	if (mApp->mEasyPlantingCheat)
		return PLANTING_OK;

	return PLANTING_NOT_PASSED;
}

void Board::UpdateToolTip()
{
	if (mApp->GetDialogCount())
	{
		mToolTip->mVisible = false;
		return;
	}

	if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
	{
		SeedType aSeedType = static_cast<SeedType>(mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].mPacketType);
		int aPlantCost = GetCurrentPlantCost(aSeedType, mCursorObject->mSeedBankIndex);
		int aCostTextX = 0;

		if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
			return;

		if (mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].mPacketType == SEED_NONE)
			return;

		mToolTip->mCenter = false;
		mToolTip->mX = mApp->mWidgetManager->mLastMouseX - mToolTip->mWidth / 2;
		mToolTip->mY = 455;
		mToolTip->mVisible = true;
	}
	else if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_USABLE_COIN)
	{
		Coin* aCoin = mCoins.DataArrayTryToGet(mCursorObject->mCoinID);
		if (aCoin && aCoin->mUsableSeedType != SEED_NONE)
		{
			mToolTip->mVisible = true;
		}
		else
		{
			mToolTip->mVisible = false;
		}
	}
	else
	{
		mToolTip->mVisible = false;
	}
}

void Board::MouseDownCobcannonFire(int x, int y, int theClickCount)
{
	int aGridX = PlantingPixelToGridX(x, y);
	int aGridY = PlantingPixelToGridY(x, y);

	Plant* aPlant = GetTopPlantAt(aGridX, aGridY, TOPPLANT_ONLY_NORMAL_POSITION);
	if (aPlant && aPlant->mSeedType == SEED_COBCANNON)
	{
		aPlant->BeginCobCannonFire(x, y);
	}
	else if (IsValidCobCannonSpot(aGridX, aGridY) && mApp->mEasyPlantingCheat)
	{
		AddPlant(aGridX, aGridY, SEED_COBCANNON, SEED_NONE);
		AddPlant(aGridX + 1, aGridY, SEED_COBCANNON, SEED_NONE);
	}
}

PlantingReason Board::MouseDownWithPlant(int x, int y, SeedType theSeedType, SeedType theImitaterType)
{
	int aGridX = PlantingPixelToGridX(x, y);
	int aGridY = PlantingPixelToGridY(x, y);
	PlantingReason aReason = static_cast<PlantingReason>(CanPlantAt(aGridX, aGridY, theSeedType));

	if (aReason == PLANTING_OK || aReason == PLANTING_OK_PASS)
	{
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE)
		{
			if (mApp->IsIceTrap(theSeedType))
			{
				mIceTrapCounter = 150;
			}
		}

		if (theSeedType == SEED_LILYPAD && GetTopPlantAt(aGridX, aGridY, TOPPLANT_DIGGING_ORDER))
		{
			return PLANTING_OK;
		}
		if (theSeedType == SEED_FLOWERPOT && GetFlowerPotAt(aGridX, aGridY))
		{
			return PLANTING_OK;
		}

		if (Plant::IsFlying(theSeedType))
		{
			PlantsOnLawn aPlantOnLawn;
			GetPlantsOnLawn(aGridX, aGridY, &aPlantOnLawn);
			if (aPlantOnLawn.mUnderPlant)
			{
				return PLANTING_OK;
			}
			if (aPlantOnLawn.mNormalPlant)
			{
				return PLANTING_OK;
			}
		}

		if (!mChallenge->IsZombieSeed(theSeedType) && theSeedType != SEED_GRAVEBUSTER && theSeedType != SEED_TANGLEKELP &&
			theSeedType != SEED_COBCANNON && theSeedType != SEED_FLOWERPOT && theSeedType != SEED_LILYPAD && !Plant::IsFlying(theSeedType) &&
			!mApp->mEasyPlantingCheat)
		{
			int aCost = GetCurrentPlantCost(theSeedType, mCursorObject->mSeedBankIndex);
			if (mChallenge->IsZombieSeed(theSeedType))
				return PLANTING_OK;
			if (mSunMoney >= aCost)
			{
				TakeSunMoney(aCost);
			}
			else
			{
				return PLANTING_NOT_ENOUGH_SUN;
			}
		}

		if (theSeedType == SEED_CHERRYBOMB || theSeedType == SEED_POTATOMINE || theSeedType == SEED_SQUASH)
		{
			Plant* aCurrentPlant = GetTopPlantAt(aGridX, aGridY, TOPPLANT_DIGGING_ORDER);
			if (aCurrentPlant)
			{
				TOD_ASSERT(!aCurrentPlant->NotOnGround());
				aCurrentPlant->Die();
			}
		}

		if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		{
			if (theSeedType == SEED_WALLNUT)
			{
				Plant* aPlant = GetTopPlantAt(aGridX, aGridY, TOPPLANT_ONLY_NORMAL_POSITION);
				if (!aPlant)
				{
					AddPlant(aGridX, aGridY, theSeedType, theImitaterType);
				}
				mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
				ClearCursor();
				return PLANTING_OK;
			}
		}

		if (theSeedType != SEED_COBCANNON)
		{
			AddPlant(aGridX, aGridY, theSeedType, theImitaterType);
		}
		else
		{
			AddPlant(aGridX, aGridY, theSeedType, theImitaterType);
			AddPlant(aGridX + 1, aGridY, theSeedType, theImitaterType);
		}

		if (theSeedType == SEED_LILYPAD || theSeedType == SEED_FLOWERPOT)
		{
			if (!mChallenge->IsZombieSeed(theSeedType) && mApp->IsAdventureMode())
			{
				UpdateSeedBankMouseOver();
			}
		}
	}

	return aReason;
}

void Board::TutorialArrowShow(float theX, float theY, float theRotation)
{
	mApp->AddTodParticle(theX, theY, RENDER_LAYER_TOP, PARTICLE_TUTORIAL_ARROW);
	TodParticleSystem* aTutorialParticle = mApp->ParticleTryToGet(mTutorialParticleID);
	mTutorialParticleID = mApp->ParticleGetID(aTutorialParticle);

	TodParticleSystem* aParticle = mApp->ParticleTryToGet(mTutorialParticleID);
	if (aParticle)
	{
		aParticle->mParticleRotation = theRotation;
	}
}

void Board::TutorialArrowRemove()
{
	TodParticleSystem* aParticle = mApp->ParticleTryToGet(mTutorialParticleID);
	if (aParticle)
	{
		aParticle->ParticleSystemDie();
	}
	mTutorialParticleID = PARTICLESYSTEMID_NULL;
}

void Board::MouseDownWithTool(int x, int y, GameObjectType theToolType)
{
	if (theToolType == OBJECT_TYPE_SHOVEL)
	{
		mCursorObject->mCursorType = CURSOR_TYPE_SHOVEL;
		mApp->PlayFoley(FOLEY_SHOVEL);
		mApp->PlaySample(SOUND_SHOVEL);
		return;
	}

	int aGridX = PlantingPixelToGridX(x, y);
	int aGridY = PlantingPixelToGridY(x, y);
	Plant* aPlant = GetTopPlantAt(aGridX, aGridY, TOPPLANT_ZEN_TOOL_ORDER);
	if (!aPlant)
		return;

	switch (theToolType)
	{
	case OBJECT_TYPE_WATERING_CAN:
		mApp->mZenGarden->WaterPlant(aPlant);
		break;
	case OBJECT_TYPE_FERTILIZER:
		mApp->mZenGarden->FertilizePlant(aPlant);
		break;
	case OBJECT_TYPE_BUG_SPRAY:
		mApp->mZenGarden->ApplyBugSpray(aPlant);
		break;
	case OBJECT_TYPE_PHONOGRAPH:
		mApp->mZenGarden->ApplyPhonograph(aPlant);
		break;
	case OBJECT_TYPE_CHOCOLATE:
		mApp->mZenGarden->ApplyChocolate(aPlant);
		break;
	case OBJECT_TYPE_GLOVE:
		mApp->mZenGarden->PlantInWheelbarrow(aPlant);
		mCursorObject->mCursorType = CURSOR_TYPE_WHEEELBARROW;
		break;
	case OBJECT_TYPE_MONEY_SIGN:
		mApp->mZenGarden->SellPlant(aPlant);
		break;
	case OBJECT_TYPE_WHEELBARROW:
		mApp->mZenGarden->DropPlantFromWheelbarrow(aPlant);
		break;
	case OBJECT_TYPE_TREE_FOOD:
		mChallenge->TreeOfWisdomFeed(aPlant);
		break;
	case OBJECT_TYPE_TREE_OF_WISDOM:
		mChallenge->TreeOfWisdomBoost();
		break;
	default:
		TOD_ASSERT(false);
		break;
	}
}

GameObjectType Board::SpecialPlantHitTest(int x, int y)
{
	for (int anObject = OBJECT_TYPE_WATERING_CAN; anObject <= OBJECT_TYPE_TREE_OF_WISDOM; anObject++)
	{
		if (ToolHitTestHelper(x, y, (GameObjectType)anObject))
		{
			return (GameObjectType)anObject;
		}
	}
	return OBJECT_TYPE_NONE;
}

void Board::MouseHitTestPlant(int x, int y, HitResult* theHitResult)
{
	theHitResult->mObjectType = OBJECT_TYPE_NONE;
	theHitResult->mObject = NULL;

	if (mApp->mGameScene != SCENE_PLAYING)
		return;

	Plant* aPlant = GetTopPlantAt(PlantingPixelToGridX(x, y), PlantingPixelToGridY(x, y), TOPPLANT_BUNGEE_ORDER);
	if (aPlant && aPlant->BeginDraw(NULL) && aPlant->MouseHitTest(x, y, theHitResult))
	{
		theHitResult->mObjectType = OBJECT_TYPE_PLANT;
		theHitResult->mObject = aPlant;
	}
}

void Board::MouseHitTest(int x, int y, HitResult* theHitResult)
{
	theHitResult->mObjectType = OBJECT_TYPE_NONE;
	theHitResult->mObject = NULL;

	if (mApp->GetDialogCount() || !mApp->mWidgetManager->mWidgetsEnabled)
		return;

	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->BeginDraw(NULL) && aZombie->MouseHitTest(x, y, theHitResult))
		{
			return;
		}
	}
	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->BeginDraw(NULL) && aCoin->MouseHitTest(x, y, theHitResult))
		{
			return;
		}
	}
	Projectile* aProjectile = NULL;
	while (IterateProjectiles(aProjectile))
	{
		if (aProjectile->BeginDraw(NULL) && aProjectile->MouseHitTest(x, y, theHitResult))
		{
			return;
		}
	}
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_ZEN_TOOL && aGridItem->BeginDraw(NULL))
		{
			if (aGridItem->MouseHitTest(x, y, theHitResult))
			{
				return;
			}
		}
	}
	MouseHitTestPlant(x, y, theHitResult);
	if (theHitResult->mObjectType != OBJECT_TYPE_NONE)
		return;

	LawnMower* aMower = NULL;
	while (IterateLawnMowers(aMower))
	{
		if (aMower->mMowerState != MOWER_TRIGGERED && aMower->mMowerState != MOWER_SQUISHED && aMower->BeginDraw(NULL))
		{
			Rect aMowerRect = aMower->GetLawnMowerAttackRect();
			Rect aMouseRect(x, y, 1, 1);
			if (aMouseRect.Intersects(aMowerRect))
			{
				theHitResult->mObjectType = OBJECT_TYPE_LAWNMOWER;
				theHitResult->mObject = aMower;
				return;
			}
		}
	}
}

void Board::PickUpTool(int x, int y, GameObjectType theObjectType)
{
	if (theObjectType == OBJECT_TYPE_SHOVEL)
	{
		MouseDownWithTool(x, y, OBJECT_TYPE_SHOVEL);
	}
	else
	{
		MouseDownWithTool(x, y, theObjectType);
	}
}

// GOTY @Patoke: 0x414B40
void Board::MouseDown(int x, int y, int theClickCount)
{
	if (mApp->GetDialogCount() && mApp->mWidgetManager->mWidgetsEnabled && mApp->mDialogList.front()->mImage != NULL)
	{
		mApp->KillDialog(mApp->mDialogList.front()->mId);
		MarkAllDirty();
		return;
	}

	if (mMenuButton->mDisabled && (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM))
	{
		mChooseShapeSwapChallenged = 0;
	}

	if (mMenuButton->MouseHitTest(x, y))
	{
		mApp->PlaySample(SOUND_TAP);
		mApp->BoardButtonPress();
		return;
	}

	if (mStoreButton && mStoreButton->MouseHitTest(x, y))
	{
		mApp->PlaySample(SOUND_TAP);
		mChallenge->StoreButtonPressed();
		return;
	}

	if (mApp->mGameMode != GAMEMODE_CHALLENGE_ICE && mApp->mGameMode != GAMEMODE_CHALLENGE_LAST_STAND && mApp->mGameMode != GAMEMODE_UPSELL && !mApp->IsScaryPotterLevel())
	{
		if (x >= 681 && x <= 798 && y >= 546 && y <= 598)
		{
			if (!(mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM))
			{
				mApp->PlaySample(SOUND_TAP);
			}
		}
		else if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND || mApp->mGameMode == GAMEMODE_UPSELL)
		{
			mApp->PlaySample(SOUND_GRAVEBUTTON);
		}
	}

	if (mApp->mGameScene == SCENE_LEVEL_INTRO && mApp->mSeedChooserScreen)
	{
		mApp->mSeedChooserScreen->CancelLawnView();
	}
	if (mApp->mGameScene == SCENE_ZOMBIES_WON)
	{
		mCutScene->ZombieWonClick();
		return;
	}
	if (mApp->mGameScene == SCENE_LEVEL_INTRO)
	{
		mCutScene->MouseDown(x, y);
	}

	if (mApp->mTodCheatKeys && !mApp->IsScaryPotterLevel() && mNextSurvivalStageCounter > 0)
	{
		mNextSurvivalStageCounter = 2;
		for (int i = 0; i < MAX_GRID_SIZE_Y; i++)
		{
			if (mIceTimer[i] > 2)
			{
				mIceTimer[i] = 2;
			}
		}
	}

	CursorType aCursor = static_cast<CursorType>(mCursorObject->mCursorType);
	HitResult aHitResult;
	MouseHitTest(x, y, &aHitResult);

	if (aHitResult.mObjectType == OBJECT_TYPE_NONE)
	{
		if (aCursor == CURSOR_TYPE_COBCANNON_TARGET)
		{
			MouseDownCobcannonFire(x, y, theClickCount);
			UpdateCursor();
			return;
		}
	}
	else if (aHitResult.mObjectType == OBJECT_TYPE_COIN && theClickCount >= 0)
	{
		Coin* aCoin = (Coin*)aHitResult.mObject;
		if (aCoin->mBoard)
		{
			aCoin->MouseDown(x, y, theClickCount);
		}
		UpdateCursor();
		return;
	}

	if (aCursor == CURSOR_TYPE_SHOVEL ||
		aCursor == CURSOR_TYPE_WATERING_CAN ||
		aCursor == CURSOR_TYPE_FERTILIZER ||
		aCursor == CURSOR_TYPE_BUG_SPRAY ||
		aCursor == CURSOR_TYPE_PHONOGRAPH ||
		aCursor == CURSOR_TYPE_CHOCOLATE ||
		aCursor == CURSOR_TYPE_GLOVE ||
		aCursor == CURSOR_TYPE_MONEY_SIGN ||
		aCursor == CURSOR_TYPE_WHEEELBARROW)
	{
		PickUpTool(x, y, SpecialPlantHitTest(x, y));
		UpdateCursor();
		return;
	}

	if (aCursor == CURSOR_TYPE_PLANT_FROM_BANK || aCursor == CURSOR_TYPE_PLANT_FROM_USABLE_COIN || aCursor == CURSOR_TYPE_PLANT_FROM_GLOVE)
	{
		SeedType aSeedType = GetSeedTypeInCursor();
		SeedType aImitaterType = SEED_NONE;
		if (aSeedType == SEED_IMITATER && mCursorObject->mImitaterType != SEED_NONE)
		{
			aImitaterType = static_cast<SeedType>(mCursorObject->mImitaterType);
		}
		else if (aSeedType == SEED_ZOMBIE_IMP)
		{
			aSeedType = SEED_ZOMBIE_NORMAL;
		}

		PlantingReason aPlantResult = MouseDownWithPlant(x, y, aSeedType, aImitaterType);
		if (aPlantResult == PLANTING_OK)
		{
			mApp->PlayFoley(FOLEY_PLANT);
			if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
			{
				mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
			}
			else if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_USABLE_COIN)
			{
				mCoins.DataArrayTryToGet(mCursorObject->mCoinID)->DroppedUsableSeed();
			}

			ClearCursor();
			if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
			{
				DisplayAdvice("[ADVICE_CLICK_ON_SUN]", MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, ADVICE_CLICK_ON_SUN);
			}
		}
		else if (aPlantResult == PLANTING_NOT_ENOUGH_SUN)
		{
			mApp->PlayFoley(FOLEY_OUT_OF_SUN);
		}
		else if (aPlantResult == PLANTING_NOT_ON_GRAVE)
		{
			mApp->PlayFoley(FOLEY_GRAVE);
		}
		return;
	}

	if (aCursor == CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW || aCursor == CURSOR_TYPE_PLANT_FROM_DUPLICATOR)
	{
		SeedType aSeedType = static_cast<SeedType>(mCursorObject->mType);
		SeedType aImitaterType = SEED_NONE;
		AddPlant(PlantingPixelToGridX(x, y), PlantingPixelToGridY(x, y), aSeedType, aImitaterType);
		if (aCursor == CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW)
		{
			mApp->mZenGarden->DropPlantFromWheelbarrow(NULL);
		}
		mApp->PlayFoley(FOLEY_PLANT);
		ClearCursor();
		return;
	}

	if (aHitResult.mObjectType == OBJECT_TYPE_LAWNMOWER && theClickCount >= 0)
	{
		LawnMower* aMower = (LawnMower*)aHitResult.mObject;
		aMower->StartMower();
		return;
	}

	if (aCursor != CURSOR_TYPE_NEXT_GARDEN && aCursor != CURSOR_TYPE_POINTER)
		return;

	GameObjectType aToolType = ToolHitTest(x, y);
	if (aToolType != OBJECT_TYPE_NONE)
	{
		PickUpTool(x, y, aToolType);
		UpdateCursor();
		return;
	}

	if (aCursor == CURSOR_TYPE_NEXT_GARDEN)
	{
		mApp->mZenGarden->GardenNext();
		return;
	}
}

void Board::ClearCursor()
{
	mCursorObject->mCursorType = CURSOR_TYPE_POINTER;
	mCursorObject->mSeedBankIndex = -1;
	mCursorObject->mCoinID = COINID_NULL;
	mCursorPreview->mVisible = false;
}

void Board::CanInteractWithBoardButtons()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE && mIceTrapCounter > 0 &&
		mApp->mGameScene == SCENE_PLAYING && mApp->mWidgetManager && mApp->mWidgetManager->mWidgetsEnabled)
	{
		return;
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mMenuButton->mBtnNoDraw = mApp->mZenGarden->mGardenChoice != GARDEN_MAIN;
	}
	return;
}

// GOTY @Patoke: 0x415F80
void Board::MouseUp(int x, int y)
{
	if (mApp->mGameScene != SCENE_PLAYING || mIgnoreMouseUp)
	{
		mIgnoreMouseUp = false;
		return;
	}

	if (!mApp->GetDialogCount() && mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
	{
		SeedType aSeedType = static_cast<SeedType>(mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].mPacketType);

		if (MowingInvokedByMouseUp(aSeedType) && mChallenge->IsZombieSeed(aSeedType))
		{
			AddZombie(static_cast<ZombieType>(aSeedType - SEED_ZOMBIE_NORMAL + ZOMBIE_NORMAL), ZOMBIE_WAVE_DEBUG);
			ClearCursor();
		}
		else
		{
			int aGridX = PlantingPixelToGridX(x, y);
			int aGridY = PlantingPixelToGridY(x, y);
			SeedType aImitaterType = SEED_NONE;

			if (aSeedType == SEED_IMITATER && mCursorObject->mImitaterType != SEED_NONE)
			{
				aImitaterType = static_cast<SeedType>(mCursorObject->mImitaterType);
			}

			PlantingReason aPlantResult = MouseDownWithPlant(x, y, aSeedType, aImitaterType);
			if (aPlantResult == PLANTING_OK)
			{
				mApp->PlayFoley(FOLEY_PLANT);
				mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
				ClearCursor();
			}
			else if (aPlantResult == PLANTING_NOT_ENOUGH_SUN)
			{
				mApp->PlayFoley(FOLEY_OUT_OF_SUN);
			}
			else if (aPlantResult == PLANTING_NOT_ON_GRAVE)
			{
				mApp->PlayFoley(FOLEY_GRAVE);
			}
		}
	}
}

void Board::ShowCoinBank()
{
	mCoinBankFadeCount = 100;
}

void Board::Pause(bool thePause)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		return;
	}
	mPaused = thePause;
}

int Board::GetGraveStonesCount()
{
	int aCount = 0;
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE)
		{
			aCount++;
		}
	}
	return aCount;
}

void Board::PickSpecialGraveStone()
{
	if (!StageHasGraveStones())
	{
		mSpecialGraveStoneX = -1;
		mSpecialGraveStoneY = -1;
		return;
	}

	if (mApp->IsFirstTimeAdventureMode())
	{
		int aGridCount = 0;
		int aGrid[MAX_GRID_SIZE_Y];
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			if (GetGraveStoneAt(8, y))
			{
				aGrid[aGridCount++] = y;
			}
		}
		if (aGridCount > 0)
		{
			int aRand = Rand(aGridCount);
			mSpecialGraveStoneX = 8;
			mSpecialGraveStoneY = aGrid[aRand];
		}
	}
	else
	{
		int aGridCount = 0;
		int aGridX[MAX_GRAVE_STONES];
		int aGridY[MAX_GRAVE_STONES];
		GridItem* aGridItem = NULL;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE)
			{
				aGridX[aGridCount] = aGridItem->mGridX;
				aGridY[aGridCount] = aGridItem->mGridY;
				aGridCount++;
			}
		}
		if (aGridCount > 0)
		{
			int aRand = Rand(aGridCount);
			mSpecialGraveStoneX = aGridX[aRand];
			mSpecialGraveStoneY = aGridY[aRand];
		}
	}
}

void Board::SpawnZombiesFromPool()
{
	int aRow = Rand(2) + 2;
	ZombieType aZombieType = ZOMBIE_NORMAL;

	Zombie* aZombie = mZombies.DataArrayAlloc();
	aZombie->ZombieInitialize(aRow, aZombieType, false, NULL, ZOMBIE_WAVE_DEBUG);
	aZombie->mPosX -= 75;
	aZombie->mPosY += 195;
}

void Board::SetupBungeeDrop(int x, int theGridY, ZombieType theZombieType)
{
	Zombie* aBungee = AddZombieInRow(theZombieType, theGridY, ZOMBIE_WAVE_DEBUG);
	if (aBungee)
	{
		aBungee->mPosX = x;
		aBungee->mTargetCol = PixelToGridX(x, theGridY);
	}
}

void Board::BungeeDropZombie(ZombieType theZombieType)
{
	int aAttempt = 0;
	while (aAttempt < 20)
	{
		int aGridY = RandRangeInt(0, MAX_GRID_SIZE_Y - 1);
		if (RowCanHaveZombies(aGridY))
		{
			int aPosX = RandRangeInt(400, 700);
			SetupBungeeDrop(aPosX, aGridY, theZombieType);
			return;
		}
		aAttempt++;
	}
}

void Board::SpawnZombiesFromSky()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BUNGEE_BLITZ)
	{
		int aCount = 0;
		if (IsFlagWave(mCurrentWave))
		{
			aCount = mCurrentWave / 10 + 1;
		}
		for (int i = 0; i < aCount; i++)
		{
			BungeeDropZombie(ZOMBIE_BUNGEE);
		}
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_AIR_RAID)
	{
		if (IsFlagWave(mCurrentWave))
		{
			for (int i = 0; i < 5; i++)
			{
				BungeeDropZombie(ZOMBIE_BALLOON);
			}
		}
		else
		{
			BungeeDropZombie(ZOMBIE_BALLOON);
		}
	}
}

void Board::SpawnZombiesFromGraves()
{
	int aRow = Rand(MAX_GRID_SIZE_Y);
	if (!RowCanHaveZombies(aRow))
		return;

	if (GetGraveStonesCount() == 0)
		return;

	int aGridX = -1;
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE && aGridItem->mGridY == aRow)
		{
			aGridX = aGridItem->mGridX;
			break;
		}
	}

	if (aGridX == -1)
		return;

	if (GetGraveStoneAt(aGridX, aRow))
	{
		ZombieType aZombieType = PickGraveRisingZombieType();
		Zombie* aZombie = AddZombieInRow(aZombieType, aRow, ZOMBIE_WAVE_DEBUG);
		if (aZombie)
		{
			aZombie->mPosX = GridToPixelX(aGridX, aRow) + 40;
			aZombie->mRiseFromGraveCounter = RandRangeInt(150, 250);
			aZombie->mRiseFromGraveGridX = aGridX;
		}
	}
}

int Board::TotalZombiesHealthInWave(int theWave)
{
	int aTotalHealth = 0;
	for (int aZombieIndex = 0; aZombieIndex < MAX_ZOMBIES_IN_WAVE; aZombieIndex++)
	{
		ZombieType aZombieType = mZombiesInWave[theWave][aZombieIndex];
		if (aZombieType == ZOMBIE_INVALID)
			break;

		aTotalHealth += GetZombieDefinition(aZombieType).mBodyHealth;
	}
	return aTotalHealth;
}

void Board::SpawnZombieWave()
{
	if (mCurrentWave < mNumWaves)
	{
		if (!mApp->IsStormyNightLevel())
		{
			// 显示下一大波文字，但风暴关卡除外（风暴本身会显示）
			if (IsFlagWave(mCurrentWave) && mCurrentWave > 0)
			{
				mHugeWaveCountDown = HugeWaveCountDown;
				mApp->PlayFoley(FOLEY_HUGE_WAVE);
			}
		}

		for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
		{
			ZombieType aZombieType = mZombiesInWave[mCurrentWave][i];
			if (aZombieType == ZOMBIE_INVALID)
				break;

			AddZombie(aZombieType, mCurrentWave);
		}

		SpawnZombiesFromSky();
		mTotalSpawnedWaves++;
		mCurrentWave++;
		if (mCurrentWave == mNumWaves)
		{
			mFinalWaveSoundCounter = 100;
		}
	}

	if (mApp->IsAdventureMode() && mCurrentWave < mNumWaves)
	{
		int aDisplayWaveNumber = mCurrentWave + 1;
		if (mApp->IsFirstTimeAdventureMode() && mLevel == 2)
		{
			aDisplayWaveNumber = 0;
		}
		if (mApp->IsFirstTimeAdventureMode() && mLevel <= 9)
		{
			DisplayAdviceAgain(TodReplaceNumberString("[ADVICE_WAVES]", "{WAVES}", aDisplayWaveNumber), MESSAGE_STYLE_HINT_STAY, ADVICE_NONE);
		}
	}
}

void Board::UpdateGameObjects()
{
	ProcessDeleteQueue();

	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mDead)
			continue;

		TOD_ASSERT(aPlant->mIsOnBoard);
		aPlant->Update();
	}

	Projectile* aProjectile = NULL;
	while (IterateProjectiles(aProjectile))
	{
		if (aProjectile->mDead)
			continue;

		aProjectile->Update();
	}

	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mDead)
			continue;

		aCoin->Update();
	}

	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mDead)
			continue;

		aZombie->Update();
	}

	UpdateGridItems();

	LawnMower* aMower = NULL;
	while (IterateLawnMowers(aMower))
	{
		aMower->Update();
	}

	mApp->mEffectSystem->Update();
	ProcessDeleteQueue();
}

void Board::StopAllZombieSounds()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		aZombie->StopZombieSounds();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		StopAllZombieSounds();
	}
}

int Board::GetSurvivalFlagsCompleted()
{
	return mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + mCurrentWave;
}

void Board::SurvivalSaveScore()
{
	if (!mApp->IsSurvivalMode())
		return;

	int aFlags = GetSurvivalFlagsCompleted();
	if (mApp->IsSurvivalNormal(mApp->mGameMode))
	{
		int aBestFlags = mApp->mPlayerInfo->mSurvivalNormalFlags;
		if (aFlags > aBestFlags)
		{
			mApp->mPlayerInfo->mSurvivalNormalFlags = aFlags;
		}
	}
	else if (mApp->IsSurvivalHard(mApp->mGameMode))
	{
		int aBestFlags = mApp->mPlayerInfo->mSurvivalHardFlags;
		if (aFlags > aBestFlags)
		{
			mApp->mPlayerInfo->mSurvivalHardFlags = aFlags;
		}
	}
	else if (mApp->IsSurvivalEndless(mApp->mGameMode))
	{
		int aBestFlags = mApp->mPlayerInfo->mSurvivalEndlessFlags;
		if (aFlags > aBestFlags)
		{
			mApp->mPlayerInfo->mSurvivalEndlessFlags = aFlags;
		}
	}
}

void Board::PuzzleSaveStreak()
{
	if (mApp->IsEndlessIZombie(mApp->mGameMode))
	{
		int aStreak = mChallenge->mSurvivalStage;
		if (aStreak > mApp->mPlayerInfo->mIZombieEndlessStreak)
		{
			mApp->mPlayerInfo->mIZombieEndlessStreak = aStreak;
		}
	}
	if (mApp->IsEndlessScaryPotter(mApp->mGameMode))
	{
		int aStreak = mChallenge->mSurvivalStage;
		if (aStreak > mApp->mPlayerInfo->mScaryPotterEndlessStreak)
		{
			mApp->mPlayerInfo->mScaryPotterEndlessStreak = aStreak;
		}
	}
}

void Board::ZombiesWon()
{
	if (mApp->mGameScene == SCENE_ZOMBIES_WON)
		return;

	mApp->mGameScene = SCENE_ZOMBIES_WON;
	mApp->mMusic->StopAllMusic();
	ClearAdviceImmediately();
	mApp->PlaySample(SOUND_ZOMBIESARE_COMING);
	mCutScene->ZombieWonStart();
}

bool Board::IsFinalScaryPotterStage()
{
	if (!mApp->IsScaryPotterLevel() || mApp->IsAdventureMode())
		return false;

	if (LawnApp::IsEndlessScaryPotter(mApp->mGameMode))
		return false;

	return mChallenge->mSurvivalStage >= 2;
}

bool Board::IsFinalSurvivalStage()
{
	if (mChallenge->mSurvivalStage < 4)
		return false;

	if ((mApp->mGameMode == GAMEMODE_SURVIVAL_NORMAL_STAGE_5 || mApp->mGameMode == GAMEMODE_SURVIVAL_HARD_STAGE_5 || mApp->mGameMode == GAMEMODE_SURVIVAL_ENDLESS_STAGE_5) && mChallenge->mSurvivalStage >= 4)
		return true;

	return false;
}

bool Board::IsLastStandFinalStage()
{
	if (mApp->mGameMode != GAMEMODE_CHALLENGE_LAST_STAND)
		return false;

	return (mChallenge->mSurvivalStage >= 4);
}

bool Board::IsSurvivalStageWithRepick()
{
	if (!mApp->IsSurvivalMode())
		return false;

	return (mChallenge->mSurvivalStage > 0 && mChallenge->mSurvivalStage % 5 != 0);
}

bool Board::IsLastStandStageWithRepick()
{
	if (mApp->mGameMode != GAMEMODE_CHALLENGE_LAST_STAND)
		return false;

	return (mChallenge->mSurvivalStage > 0 && mChallenge->mSurvivalStage % 5 != 0);
}

bool Board::HasLevelAwardDropped()
{
	return mLevelAwardSpawned;
}

void Board::UpdateSunSpawning()
{
	if (mApp->mGameScene != SCENE_PLAYING)
		return;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
		return;

	if (mApp->IsStormyNightLevel())
		return;

	if (mApp->IsScaryPotterLevel())
	{
		if (mSunCountDown > 0)
		{
			mSunCountDown--;
		}
		if (mSunCountDown == 0)
		{
			mSunCountDown = RandRangeInt(200, 600);
			int aSunPosX = RandRangeInt(70, 730);
			int aSunPosY = -70;
			AddCoin(aSunPosX, aSunPosY, COIN_SUN, COIN_MOTION_FROM_SKY);
		}
		return;
	}

	if (mApp->IsIZombieLevel())
	{
		if (mSunCountDown > 0)
			mSunCountDown--;

		if (mSunCountDown == 0)
		{
			mSunCountDown = 1000;
			AddCoin(RandRangeInt(40, 760), 0, COIN_SUN, COIN_MOTION_FROM_SKY);
		}
		return;
	}

	if (StageIsNight())
		return;

	if (mSunCountDown > 0)
	{
		mSunCountDown--;
	}

	if (mSunCountDown == 0)
	{
		mApp->PlayFoley(FOLEY_SPAWN_SUN);
		AddCoin(RandRangeInt(40, 760), 0, COIN_SUN, COIN_MOTION_FROM_SKY);
		mSunCountDown = RandRangeInt(425, 700);
		mNumSunsFallen++;
	}
}

void Board::NextWaveComing()
{
	mZombieHealthToNextWave = 0;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mHasHead && aZombie->mZombieType != ZOMBIE_BUNGEE && aZombie->mZombieType != ZOMBIE_BALLOON)
		{
			mZombieHealthToNextWave += aZombie->mBodyHealth;
			mZombieHealthToNextWave += aZombie->mHelmHealth;
		}
	}
	if (mZombieHealthToNextWave > 0)
	{
		mZombieHealthToNextWave = MAX(mZombieHealthToNextWave / 3, 100);
	}
}

void Board::UpdateZombieSpawning()
{
	if (mApp->mGameScene != SCENE_PLAYING || mPaused)
		return;

	if (mCurrentWave >= mNumWaves)
		return;

	if (mNumWaves == 0)
		return;

	if (mZombieHealthToNextWave > 0)
	{
		if (mZombieCountDown <= 0)
		{
			NextWaveComing();
		}
		else
		{
			int aMinHealth = TotalZombiesHealthInWave(mCurrentWave);
			if (aMinHealth < mZombieHealthToNextWave)
			{
				mZombieHealthToNextWave = aMinHealth;
			}
		}

		if (mZombieHealthToNextWave < 100)
		{
			mZombieCountDown = 2;
			mZombieHealthToNextWave = 0;
		}
		return;
	}

	if (mApp->IsStormyNightLevel() && mCurrentWave > 0 && mZombieCountDownStart > 0)
	{
		mZombieCountDownStart--;
		if (mZombieCountDownStart == 0)
		{
			SpawnZombieWave();
		}
		return;
	}

	if (mZombieCountDown <= 0)
	{
		if (mZombieCountDownStart > 0)
		{
			mZombieCountDown = 2;
			mZombieCountDownStart = 0;
		}
		return;
	}

	mZombieCountDown--;
	if (mZombieCountDown == 0)
	{
		SpawnZombieWave();
		if (mCurrentWave < mNumWaves)
		{
			Zombie* aZombie = NULL;
			int aZombieCount = 0;
			while (IterateZombies(aZombie))
			{
				if (!aZombie->IsDeadOrDying())
				{
					aZombieCount++;
				}
			}

			int aCountDown = RankRangeInt(ZOMBIE_COUNTDOWN_RANGE);
			bool aIsLotOfZombies = aZombieCount > mCurrentWave * 3 + 5;
			if (aIsLotOfZombies || mCurrentWave < 1 || mApp->IsFirstTimeAdventureMode())
			{
				mZombieCountDownStart = aCountDown;
			}
			else
			{
				int aZombieDefeated = mCurrentWave * 3 + 5;
				if (aZombieCount < aZombieDefeated)
				{
					mZombieCountDownStart = aCountDown;
				}
				else
				{
					NextWaveComing();
				}
			}
		}
		else
		{
			mZombieCountDownStart = 0;
		}
	}
}

void Board::UpdateIce()
{
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (mIceTimer[aRow])
		{
			mIceTimer[aRow]--;
			TodParticleSystem* aParticleIce = mApp->ParticleTryToGet(mIceParticleID[aRow]);
			if (mIceTimer[aRow] == 0)
			{
				mIceMinX[aRow] = BOARD_ICE_START;
				if (aParticleIce)
				{
					aParticleIce->ParticleSystemDie();
				}
			}
			else
			{
				float aPosX = mIceMinX[aRow];
				float aPosY = GridToPixelY(8, aRow);
				if (aParticleIce)
				{
					aParticleIce->SystemMove(aPosX, aPosY);
				}
				else
				{
					int aRenderPosition = MakeRenderOrder(RENDER_LAYER_GROUND, aRow, 3);
					aParticleIce = reinterpret_cast<TodParticleSystem*>(mApp->AddTodParticle(aPosX, aPosY, aRenderPosition, PARTICLE_ICE_SPARKLE));
					mIceParticleID[aRow] = mApp->ParticleGetID(aParticleIce);
				}
			}

			int anAlpha = ClampInt(mIceTimer[aRow] / 10, 0, 255);
			aParticleIce->OverrideColor(NULL, Color(255, 255, 255, anAlpha));
		}
	}
}

void Board::UpdateProgressMeter()
{
	if (mApp->IsFinalBossLevel())
	{
		Zombie* aBoss = GetBossZombie();
		if (aBoss && !aBoss->IsDeadOrDying())
		{
			mProgressMeterWidth = 150 * (aBoss->mBodyMaxHealth - aBoss->mBodyHealth) / aBoss->mBodyMaxHealth;
		}
		else
		{
			mProgressMeterWidth = 150;
		}
	}
	else if (mCurrentWave != 0)
	{
		if (mFlagRaiseCounter > 0)
			mFlagRaiseCounter--;

		int aTotalWidth = 150;
		int aNumWavesPerFlag = GetNumWavesPerFlag();
		bool aHasFlags = ProgressMeterHasFlags();
		if (aHasFlags)
		{
			aTotalWidth -= 12 * mNumWaves / aNumWavesPerFlag;
		}

		int aNumPercent = 0;
		if (mNumWaves > 0)
		{
			aNumPercent = (mProgressMeterWidth * (mCurrentWave)) / mNumWaves;
		}

		if (mCurrentWave == mNumWaves)
		{
			mProgressMeterWidth = aTotalWidth;
		}
		else
		{
			int aStartWidth = aTotalWidth * mCurrentWave / mNumWaves;
			int aEndWidth = aTotalWidth * (mCurrentWave + 1) / mNumWaves;
			if (mZombieCountDownStart > 0)
			{
				mProgressMeterWidth = aEndWidth;
			}
			else
			{
				float aPercent = 1.0f;
				if (mZombieHealthToNextWave > 0)
				{
					Zombie* aZombie = NULL;
					int aTotalHealth = 0;
					while (IterateZombies(aZombie))
					{
						if (!aZombie->IsDeadOrDying())
						{
							aTotalHealth += aZombie->mBodyHealth;
							aTotalHealth += aZombie->mHelmHealth;
						}
					}
					aPercent = (float)aTotalHealth / (float)mZombieHealthToNextWave;
					if (aPercent > 1.0f)
						aPercent = 1.0f;
				}
				mProgressMeterWidth = aStartWidth + (int)((aEndWidth - aStartWidth) * (1.0f - aPercent));
			}
		}
	}
}

void Board::UpdateTutorial()
{
	if (mTutorialState == TUTORIAL_OFF)
		return;

	if (mTutorialTimer > 0)
	{
		mTutorialTimer--;
		if (mTutorialTimer <= 0 && mTutorialState == TUTORIAL_LEVEL_1_PICK_SEEDS)
		{
			mTutorialTimer = 600;
		}
	}
}

void Board::SetTutorialState(TutorialState theTutorialState)
{
	TutorialArrowRemove();
	mTutorialState = theTutorialState;
	mTutorialTimer = 0;
}

void Board::UpdateGame()
{
	if (mApp->GetDialogCount())
		return;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		mApp->mZenGarden->Update();
	}

	if (mPaused)
		return;

	if (mApp->mGameScene != SCENE_PLAYING && mApp->mGameScene != SCENE_LEVEL_INTRO)
		return;

	if (mApp->mGameScene == SCENE_PLAYING)
	{
		mEffectCounter++;
		if (mMainCounter == 0xFFFFFFFF)
		{
			mMainCounter = 0;
		}
		mMainCounter++;
		UpdateGameObjects();

		if (mShakeCounter > 0)
			mShakeCounter--;

		if (mFwooshCountDown > 0)
			mFwooshCountDown--;

		mChallenge->Update();

		if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM)
		{
			UpdateZombieSpawning();
			mSeedBank->Update();
			UpdateSunSpawning();
			UpdateIce();
			UpdateProgressMeter();
			UpdateTutorial();
			UpdateLevelEndSequence();

			if (mCoinBankFadeCount > 0)
				mCoinBankFadeCount--;
		}

		mApp->mEffectSystem->Update();
		if (mApp->mGameScene != SCENE_ZOMBIES_WON)
		{
			mApp->mSoundSystem->UpdateFoley();
			mApp->mMusic->UpdateMusic();
		}
	}
}

void Board::Update()
{
	Widget::Update();
	MarkDirty();
	mMenuButton->Update();
	mMenuButton->mBtnNoDraw = false;
	mApp->mMusic->UpdateMusic();
	mChallenge->Update();

	if (mStoreButton)
	{
		mStoreButton->Update();
		mStoreButton->mBtnNoDraw = false;
		mStoreButton->mParentWidget = this;
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
		{
			mStoreButton->mBtnNoDraw = false;
		}
	}

	UpdateGame();
	UpdateCursor();
	UpdateMousePosition();
	ProcessDeleteQueue();

	if (mApp->mGameScene == SCENE_PLAYING && mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mZenGarden->Update();
	}
}

void Board::UpdateLayers()
{
	MarkAllDirty();
	UpdateCursor();
	UpdateMousePosition();
	UpdateToolTip();
}

bool Board::RowCanHaveZombies(int theRow)
{
	if (theRow < 0 || theRow >= MAX_GRID_SIZE_Y)
		return false;

	return mPlantRow[theRow] != PLANTROW_DIRT;
}

int Board::GetIceZPos(int theRow)
{
	int aZPos = GridToPixelY(MAX_GRID_SIZE_X, theRow);
	if (mBackground == BACKGROUND_5_ROOF)
	{
		aZPos -= 30;
	}
	return aZPos;
}

void Board::DrawIce(Graphics* g, int theRow)
{
	if (!mIceTimer[theRow])
		return;

	int aZPos = GetIceZPos(theRow);
	g->SetColorizeImages(true);
	g->SetColor(Color(255, 255, 255, ClampInt(mIceTimer[theRow] / 10, 0, 255)));
	g->DrawImageCel(Sexy::IMAGE_ICE, mIceMinX[theRow], aZPos, 0, 0);
	g->SetColorizeImages(false);
}

void Board::DrawBackdrop(Graphics* g)
{
	Image* aBackdrop = NULL;
	switch (mBackground)
	{
	case BACKGROUND_1_DAY:
		aBackdrop = Sexy::IMAGE_BACKGROUND1;
		break;
	case BACKGROUND_2_NIGHT:
		aBackdrop = Sexy::IMAGE_BACKGROUND2;
		break;
	case BACKGROUND_3_POOL:
		aBackdrop = Sexy::IMAGE_BACKGROUND3;
		break;
	case BACKGROUND_4_FOG:
		aBackdrop = Sexy::IMAGE_BACKGROUND4;
		break;
	case BACKGROUND_5_ROOF:
		aBackdrop = Sexy::IMAGE_BACKGROUND5;
		break;
	case BACKGROUND_6_BOSS:
		aBackdrop = Sexy::IMAGE_BACKGROUND6;
		break;
	case BACKGROUND_GREENHOUSE:
		aBackdrop = Sexy::IMAGE_GREENHOUSE;
		break;
	case BACKGROUND_MUSHROOM_GARDEN:
		aBackdrop = Sexy::IMAGE_MUSHROOM_GARDEN;
		break;
	case BACKGROUND_ZOMBIQUARIUM:
		aBackdrop = Sexy::IMAGE_ZOMBIQUARIUM;
		break;
	case BACKGROUND_TREEOFWISDOM:
		aBackdrop = Sexy::IMAGE_TREEOFWISDOM;
		break;
	default:
		TOD_ASSERT(false);
		break;
	}
	if (aBackdrop)
	{
		g->DrawImage(aBackdrop, 0, 0);
	}
}

bool RenderItemSortFunc(const RenderItem& theItem1, const RenderItem& theItem2)
{
	return theItem1.mZPos < theItem2.mZPos;
}

void Board::AddBossRenderItem(Zombie* theZombie, RenderItem* theRenderItems, int& theRenderItemCount)
{
	if (!theZombie->BeginDraw(NULL))
		return;

	RenderItem aRenderItem;
	aRenderItem.mZombie = theZombie;

	if (theZombie->mZombiePhase == PHASE_BOSS_STOMP)
	{
		aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_PARTICLE, theZombie->mRow, 10000);
	}
	else
	{
		aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder);
	}
	aRenderItem.mBossPart = BOSS_PART_BACK_LEG;
	aRenderItem.mRenderObjectType = RENDER_ITEM_ZOMBIE;
	theRenderItems[theRenderItemCount++] = aRenderItem;

	aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder + 1);
	aRenderItem.mBossPart = BOSS_PART_FRONT_LEG;
	theRenderItems[theRenderItemCount++] = aRenderItem;

	aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder + 2);
	aRenderItem.mBossPart = BOSS_PART_MAIN;
	theRenderItems[theRenderItemCount++] = aRenderItem;

	aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder + 2);
	aRenderItem.mBossPart = BOSS_PART_BACK_ARM;
	theRenderItems[theRenderItemCount++] = aRenderItem;

	aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder + 100);
	aRenderItem.mBossPart = BOSS_PART_FIREBALL;
	aRenderItem.mRenderObjectType = RENDER_ITEM_ZOMBIE;
	theRenderItems[theRenderItemCount++] = aRenderItem;

	aRenderItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, theZombie->mRow, theZombie->mRenderOrder + 200);
	aRenderItem.mBossPart = BOSS_PART_FIREBALL;
	theRenderItems[theRenderItemCount++] = aRenderItem;
}

void Board::DrawGameObjects(Graphics* g)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		DrawBackdrop(g);
	}

	RenderItem aRenderItems[MAX_RENDER_ITEMS];
	int aRenderItemCount = 0;

	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		GridItem* aGridItem = NULL;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GRIDITEM_LADDER && aGridItem->mGridY == aRow)
			{
				RenderItem aItem;
				aItem.mGridItem = aGridItem;
				aItem.mRenderObjectType = RENDER_ITEM_GRID_ITEM;
				aItem.mZPos = aGridItem->mRenderOrder;
				aRenderItems[aRenderItemCount++] = aItem;
			}
		}

		RenderItem aIceItem;
		aIceItem.mBoardGridY = aRow;
		aIceItem.mRenderObjectType = RENDER_ITEM_ICE;
		aIceItem.mZPos = MakeRenderOrder(RENDER_LAYER_GROUND, aRow, 2);
		aRenderItems[aRenderItemCount++] = aIceItem;

		GridItem* aTrailGridItem = NULL;
		while (IterateGridItems(aTrailGridItem))
		{
			if (aTrailGridItem->mGridItemType == GRIDITEM_TRAIL && aTrailGridItem->mGridY == aRow)
			{
				RenderItem aItem;
				aItem.mGridItem = aTrailGridItem;
				aItem.mRenderObjectType = RENDER_ITEM_GRID_ITEM;
				aItem.mZPos = aTrailGridItem->mRenderOrder;
				aRenderItems[aRenderItemCount++] = aItem;
			}
		}
	}

	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mDead || !aPlant->BeginDraw(g))
			continue;

		RenderItem aItem;
		aItem.mPlant = aPlant;
		aItem.mRenderObjectType = RENDER_ITEM_PLANT;
		aItem.mZPos = aPlant->mRenderOrder;
		aRenderItems[aRenderItemCount++] = aItem;
	}

	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mDead || !aZombie->BeginDraw(g))
			continue;

		if (aZombie->mZombieType == ZOMBIE_BOSS)
		{
			AddBossRenderItem(aZombie, aRenderItems, aRenderItemCount);
		}
		else
		{
			RenderItem aItem;
			aItem.mZombie = aZombie;
			aItem.mRenderObjectType = RENDER_ITEM_ZOMBIE;
			aItem.mZPos = MakeRenderOrder(RENDER_LAYER_ZOMBIE, aZombie->mRow, aZombie->mRenderOrder);
			aRenderItems[aRenderItemCount++] = aItem;
		}
	}

	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mDead || !aCoin->BeginDraw(g))
			continue;

		RenderItem aItem;
		aItem.mCoin = aCoin;
		aItem.mRenderObjectType = RENDER_ITEM_COIN;
		aItem.mZPos = aCoin->mRenderOrder;
		aRenderItems[aRenderItemCount++] = aItem;
	}

	Projectile* aProjectile = NULL;
	while (IterateProjectiles(aProjectile))
	{
		if (aProjectile->mDead || !aProjectile->BeginDraw(g))
			continue;

		RenderItem aItem;
		aItem.mProjectile = aProjectile;
		aItem.mRenderObjectType = RENDER_ITEM_PROJECTILE;
		aItem.mZPos = MakeRenderOrder(RENDER_LAYER_PROJECTILE, aProjectile->mRow, aProjectile->mRenderOrder);
		aRenderItems[aRenderItemCount++] = aItem;

		RenderItem aShadowItem;
		aShadowItem.mProjectile = aProjectile;
		aShadowItem.mRenderObjectType = RENDER_ITEM_PROJECTILE_SHADOW;
		aShadowItem.mZPos = MakeRenderOrder(RENDER_LAYER_PROJECTILE_SHADOW, aProjectile->mRow, 0);
		aRenderItems[aRenderItemCount++] = aShadowItem;
	}

	if (mCursorPreview->mVisible)
	{
		RenderItem aItem;
		aItem.mCursorPreview = mCursorPreview;
		aItem.mRenderObjectType = RENDER_ITEM_CURSOR_PREVIEW;
		aItem.mZPos = MakeRenderOrder(RENDER_LAYER_PARTICLE, 0, 10000);
		aRenderItems[aRenderItemCount++] = aItem;
	}

	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE || aGridItem->mGridItemType == GRIDITEM_CRATER ||
			aGridItem->mGridItemType == GRIDITEM_SCARY_POT || aGridItem->mGridItemType == GRIDITEM_BRAIN ||
			aGridItem->mGridItemType == GRIDITEM_SQUIRREL || aGridItem->mGridItemType == GRIDITEM_PORTAL_CIRCLE ||
			aGridItem->mGridItemType == GRIDITEM_ZEN_TOOL || aGridItem->mGridItemType == GRIDITEM_RAKE)
		{
			if (aGridItem->mDead)
				continue;

			RenderItem aItem;
			aItem.mGridItem = aGridItem;
			aItem.mRenderObjectType = RENDER_ITEM_GRID_ITEM;
			aItem.mZPos = aGridItem->mRenderOrder;
			aRenderItems[aRenderItemCount++] = aItem;

			RenderItem aOverlayItem;
			aOverlayItem.mGridItem = aGridItem;
			aOverlayItem.mRenderObjectType = RENDER_ITEM_GRID_ITEM_OVERLAY;
			aOverlayItem.mZPos = aGridItem->mRenderOrder + 1;
			aRenderItems[aRenderItemCount++] = aOverlayItem;
		}
	}

	TodParticleSystem* aParticle = NULL;
	while (IterateParticles(aParticle))
	{
		if (aParticle->mDead)
			continue;

		RenderItem aItem;
		aItem.mParticleSytem = aParticle;
		aItem.mRenderObjectType = RENDER_ITEM_PARTICLE;
		aItem.mZPos = aParticle->mRenderOrder;
		aRenderItems[aRenderItemCount++] = aItem;
	}

	Reanimation* aReanim = NULL;
	while (IterateReanimations(aReanim))
	{
		if (aReanim->mDead)
			continue;

		RenderItem aItem;
		aItem.mReanimation = aReanim;
		aItem.mRenderObjectType = RENDER_ITEM_REANIMATION;
		aItem.mZPos = aReanim->mRenderOrder;
		aRenderItems[aRenderItemCount++] = aItem;
	}

	RenderItem aCoinBankItem;
	aCoinBankItem.mRenderObjectType = RENDER_ITEM_COIN_BANK;
	aCoinBankItem.mZPos = MakeRenderOrder(RENDER_LAYER_UI_BOTTOM, 0, 100);
	aRenderItems[aRenderItemCount++] = aCoinBankItem;

	RenderItem aFogItem;
	aFogItem.mRenderObjectType = RENDER_ITEM_FOG;
	aFogItem.mZPos = MakeRenderOrder(RENDER_LAYER_FOG, 0, 100);
	aRenderItems[aRenderItemCount++] = aFogItem;

	RenderItem aShovelItem;
	aShovelItem.mRenderObjectType = RENDER_ITEM_SHOVEL;
	aShovelItem.mZPos = MakeRenderOrder(RENDER_LAYER_UI_BOTTOM, 0, 0);
	aRenderItems[aRenderItemCount++] = aShovelItem;

	RenderItem aBackdropItem;
	aBackdropItem.mRenderObjectType = RENDER_ITEM_BACKDROP;
	aBackdropItem.mZPos = MakeRenderOrder(static_cast<RenderLayer>(RENDER_LAYER_BACKDROP), 0, 0);
	aRenderItems[aRenderItemCount++] = aBackdropItem;

	// Simple bubble sort (no std::sort available in Symbian SDK)
	for (int i = 0; i < aRenderItemCount; i++)
		for (int j = i + 1; j < aRenderItemCount; j++)
			if (RenderItemSortFunc(aRenderItems[j], aRenderItems[i]))
			{
				RenderItem t = aRenderItems[i];
				aRenderItems[i] = aRenderItems[j];
				aRenderItems[j] = t;
			}

	for (int aIndex = 0; aIndex < aRenderItemCount; aIndex++)
	{
		const RenderItem& aRenderItem = aRenderItems[aIndex];
		switch (aRenderItem.mRenderObjectType)
		{
		case RENDER_ITEM_BACKDROP:
			DrawBackdrop(g);
			break;

		case RENDER_ITEM_PLANT:
		{
			Plant* aPlantDraw = aRenderItem.mPlant;
			if (aPlantDraw->BeginDraw(g))
			{
				aPlantDraw->Draw(g);
				aPlantDraw->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_ZOMBIE:
		{
			Zombie* aZombieDraw = aRenderItem.mZombie;
			if (aZombieDraw->BeginDraw(g))
			{
				aZombieDraw->Draw(g);
				aZombieDraw->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_COIN:
		{
			Coin* aCoinDraw = aRenderItem.mCoin;
			if (aCoinDraw->BeginDraw(g))
			{
				aCoinDraw->Draw(g);
				aCoinDraw->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_PROJECTILE:
		{
			Projectile* aProjectileDraw = aRenderItem.mProjectile;
			if (aProjectileDraw->BeginDraw(g))
			{
				aProjectileDraw->Draw(g);
				aProjectileDraw->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_PROJECTILE_SHADOW:
		{
			Projectile* aProjectileDraw = aRenderItem.mProjectile;
			if (aProjectileDraw->BeginDraw(g))
			{
				aProjectileDraw->DrawShadow(g);
				aProjectileDraw->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_CURSOR_PREVIEW:
		{
			CursorPreview* aCursorPreview = aRenderItem.mCursorPreview;
			if (aCursorPreview->BeginDraw(g))
			{
				aCursorPreview->Draw(g);
				aCursorPreview->EndDraw(g);
			}
			break;
		}

		case RENDER_ITEM_GRID_ITEM:
		{
			GridItem* aGridItemDraw = aRenderItem.mGridItem;
			aGridItemDraw->DrawGridItem(g);
			break;
		}

		case RENDER_ITEM_GRID_ITEM_OVERLAY:
		{
			GridItem* aGridItemDraw = aRenderItem.mGridItem;
			aGridItemDraw->DrawGridItemOverlay(g);
			break;
		}

		case RENDER_ITEM_ICE:
			DrawIce(g, aRenderItem.mBoardGridY);
			break;

		case RENDER_ITEM_PARTICLE:
		{
			TodParticleSystem* aParticleDraw = aRenderItem.mParticleSytem;
			aParticleDraw->Draw(g);
			break;
		}

		case RENDER_ITEM_REANIMATION:
		{
			Reanimation* aReanimDraw = aRenderItem.mReanimation;
			aReanimDraw->Draw(g);
			break;
		}

		case RENDER_ITEM_COIN_BANK:
			DrawUICoinBank(g);
			break;

		case RENDER_ITEM_FOG:
			if (StageHasFog())
			{
				DrawFog(g);
			}
			break;

		case RENDER_ITEM_SHOVEL:
			if (mShowShovel)
			{
				DrawShovel(g);
			}
			break;

		default:
			TOD_ASSERT(false);
			break;
		}
	}
}

bool Board::HasProgressMeter()
{
	if (mApp->IsAdventureMode() && mLevel == 1)
		return false;
	return mNumWaves > 0;
}

bool Board::ProgressMeterHasFlags()
{
	return mNumWaves / GetNumWavesPerFlag() >= 2;
}

void Board::DrawProgressMeter(Graphics* g)
{
	if (!HasProgressMeter())
		return;

	int aFlagWidth = 12;
	int aFlagPos[50];
	int aFlagCount = 0;
	for (int aWave = 0; aWave < mNumWaves; aWave++)
	{
		if (IsFlagWave(aWave))
		{
			float aX = 236 + (aFlagWidth + (150 - aFlagWidth * mNumWaves / GetNumWavesPerFlag()) * (aWave + 1) / mNumWaves);
			aFlagPos[aFlagCount++] = (int)aX;
		}
	}

	int aMeterX = 251;
	int aMeterY = 588;
	int aMeterWidth = 150;
	int aMeterHeight = 8;

	g->SetColor(Color(0, 0, 0, 128));
	g->FillRect(aMeterX, aMeterY, aMeterWidth, aMeterHeight);

	g->SetColor(Color(255, 255, 255, 200));
	g->FillRect(aMeterX + 1, aMeterY + 1, mProgressMeterWidth - 2, aMeterHeight - 2);

	if (ProgressMeterHasFlags())
	{
		for (int i = 0; i < aFlagCount; i++)
		{
			g->DrawImage(Sexy::IMAGE_FLAG, aFlagPos[i] - 5, aMeterY - 10);
		}
	}
}

void Board::DrawHouseDoorBottom(Graphics* g)
{
	switch (mBackground)
	{
	case BACKGROUND_1_DAY:
	case BACKGROUND_2_NIGHT:
	case BACKGROUND_3_POOL:
	case BACKGROUND_4_FOG:
		g->DrawImage(Sexy::IMAGE_HOUSEDOOR_BOTTOM, 770, 5);
		break;
	case BACKGROUND_5_ROOF:
	case BACKGROUND_6_BOSS:
		g->DrawImage(Sexy::IMAGE_ROOFDOOR_BOTTOM, 740, 5);
		break;
	default:
		break;
	}
}

void Board::DrawHouseDoorTop(Graphics* g)
{
	switch (mBackground)
	{
	case BACKGROUND_1_DAY:
	case BACKGROUND_2_NIGHT:
	case BACKGROUND_3_POOL:
	case BACKGROUND_4_FOG:
		g->DrawImage(Sexy::IMAGE_HOUSEDOOR_TOP, 770, 5);
		break;
	case BACKGROUND_5_ROOF:
	case BACKGROUND_6_BOSS:
		g->DrawImage(Sexy::IMAGE_ROOFDOOR_TOP, 740, 5);
		break;
	default:
		break;
	}
}

void Board::UpdateMousePosition()
{
	int aPrevGridX = PixelToGridX(mPrevMouseX, mPrevMouseY);
	int aPrevGridY = PixelToGridY(mPrevMouseX, mPrevMouseY);
	int aCurGridX = PixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	int aCurGridY = PixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	if (aPrevGridX != aCurGridX || aPrevGridY != aCurGridY)
	{
		DirtyAll();
	}
	mPrevMouseX = mApp->mWidgetManager->mLastMouseX;
	mPrevMouseY = mApp->mWidgetManager->mLastMouseY;
}

void Board::UpdateZenCursor()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_TREE_OF_WISDOM)
	{
		GameObjectType aTool = ToolHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
		if (aTool != OBJECT_TYPE_NONE)
		{
			mCursorObject->mCursorType = static_cast<CursorType>(aTool - OBJECT_TYPE_WATERING_CAN + CURSOR_TYPE_WATERING_CAN);
		}
	}
}

bool Board::ToolHitTestHelper(int x, int y, GameObjectType theObjectType)
{
	if (theObjectType == OBJECT_TYPE_SHOVEL)
	{
		Rect aShovelRect = GetShovelButtonRect();
		return aShovelRect.Contains(x, y);
	}
	else if (theObjectType >= OBJECT_TYPE_WATERING_CAN && theObjectType <= OBJECT_TYPE_WHEELBARROW)
	{
		Rect aZenRect;
		GetZenButtonRect(theObjectType, aZenRect);
		return aZenRect.Contains(x, y);
	}
	return false;
}

GameObjectType Board::ToolHitTest(int x, int y)
{
	for (int anObject = 0; anObject < NUM_OBJECT_TYPES; anObject++)
	{
		if (CanUseGameObject((GameObjectType)anObject) && ToolHitTestHelper(x, y, (GameObjectType)anObject))
		{
			return (GameObjectType)anObject;
		}
	}
	return OBJECT_TYPE_NONE;
}

void Board::DrawLevel(Graphics* g)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		DrawBackdrop(g);
		DrawGameObjects(g);
		DrawUITop(g);
		return;
	}

	if (mApp->mGameScene == SCENE_LEVEL_INTRO)
	{
		DrawBackdrop(g);
		DrawHouseDoorBottom(g);
		DrawGameObjects(g);
		DrawHouseDoorTop(g);
		DrawUITop(g);
		return;
	}

	if (mApp->mGameScene == SCENE_ZOMBIES_WON)
	{
		DrawBackdrop(g);
		DrawHouseDoorBottom(g);
		DrawGameObjects(g);
		DrawHouseDoorTop(g);
		g->SetColor(Color(0, 0, 0, 128));
		g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
		DrawUITop(g);
		return;
	}

	DrawBackdrop(g);
	DrawHouseDoorBottom(g);
	DrawGameObjects(g);
	DrawHouseDoorTop(g);

	if (HasProgressMeter())
	{
		DrawProgressMeter(g);
	}

	DrawUITop(g);
	DrawDebugText(g);
	DrawDebugObjectRects(g);

	if (mBoardFadeOutCounter >= 0)
	{
		DrawFadeOut(g);
	}
}

void Board::DrawZenWheelBarrowButton(Graphics* g)
{
	if (!mApp->mZenGarden->mWheelbarrowPlant)
		return;

	Rect aRect;
	GetZenButtonRect(OBJECT_TYPE_WHEELBARROW, aRect);
	g->DrawImage(Sexy::IMAGE_WHEELBARROW, aRect.mX, aRect.mY);
}

void Board::DrawZenButtons(Graphics* g)
{
	for (int anObject = OBJECT_TYPE_WATERING_CAN; anObject <= OBJECT_TYPE_WHEELBARROW; anObject++)
	{
		if (!CanUseGameObject((GameObjectType)anObject))
			continue;

		Rect aRect;
		GetZenButtonRect((GameObjectType)anObject, aRect);

		Image* aImage = NULL;
		switch (anObject)
		{
		case OBJECT_TYPE_WATERING_CAN: aImage = Sexy::IMAGE_WATERINGCAN; break;
		case OBJECT_TYPE_FERTILIZER: aImage = Sexy::IMAGE_FERTILIZER; break;
		case OBJECT_TYPE_BUG_SPRAY: aImage = Sexy::IMAGE_BUGSPRAY; break;
		case OBJECT_TYPE_PHONOGRAPH: aImage = Sexy::IMAGE_PHONOGRAPH; break;
		case OBJECT_TYPE_CHOCOLATE: aImage = Sexy::IMAGE_CHOCOLATE; break;
		case OBJECT_TYPE_GLOVE: aImage = Sexy::IMAGE_GLOVE; break;
		case OBJECT_TYPE_MONEY_SIGN: aImage = Sexy::IMAGE_MONEY_SIGN; break;
		case OBJECT_TYPE_WHEELBARROW: continue; break;
		case OBJECT_TYPE_TREE_FOOD: aImage = Sexy::IMAGE_TREE_FOOD; break;
		}
		if (aImage)
		{
			g->DrawImage(aImage, aRect.mX, aRect.mY);
		}
	}
	DrawZenWheelBarrowButton(g);
}

void Board::DrawShovel(Graphics* g)
{
	if (!mShowShovel)
		return;
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		DrawZenButtons(g);
		return;
	}

	Rect aShovelRect = GetShovelButtonRect();
	g->DrawImage(Sexy::IMAGE_SHOVELBANK, aShovelRect.mX, aShovelRect.mY);
	if (mCursorObject->mCursorType == CURSOR_TYPE_SHOVEL)
	{
		g->DrawImage(Sexy::IMAGE_SHOVEL, mPrevMouseX - 35, mPrevMouseY - 35);
	}
}

void Board::DrawDebugText(Graphics* g)
{
	if (mDebugTextMode == DEBUG_TEXT_NONE)
		return;

	g->SetFont(Sexy::FONT_DWARVEN);
	g->SetColor(Color(255, 255, 255));
	char aDebugText[256];

	switch (mDebugTextMode)
	{
	case DEBUG_TEXT_FPS:
		sprintf(aDebugText, "FPS: %.0f", 1000.0f / mMinFPS);
		g->DrawString(aDebugText, 5, 595);
		break;
	case DEBUG_TEXT_ROW:
	{
		int aRow = PixelToGridY(mPrevMouseX, mPrevMouseY);
		sprintf(aDebugText, "Row: %d", aRow);
		g->DrawString(aDebugText, 5, 595);
		break;
	}
	case DEBUG_TEXT_GRID:
	{
		int aGridX = PixelToGridX(mPrevMouseX, mPrevMouseY);
		int aGridY = PixelToGridY(mPrevMouseX, mPrevMouseY);
		sprintf(aDebugText, "Grid: %d, %d", aGridX, aGridY);
		g->DrawString(aDebugText, 5, 595);
		break;
	}
	default:
		break;
	}
}

void Board::DrawDebugObjectRects(Graphics* g)
{
	if (mDebugTextMode == DEBUG_TEXT_NONE)
		return;

	g->SetColorizeImages(false);

	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->BeginDraw(g))
		{
			Rect aRect = aZombie->GetZombieRect();
			g->SetColor(Color(0, 255, 0));
			g->DrawRect(aRect.mX, aRect.mY, aRect.mWidth, aRect.mHeight);
			aZombie->EndDraw(g);
		}
	}

	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->BeginDraw(g))
		{
			Rect aRect = aPlant->GetPlantRect();
			g->SetColor(Color(255, 0, 0));
			g->DrawRect(aRect.mX, aRect.mY, aRect.mWidth, aRect.mHeight);
			aPlant->EndDraw(g);
		}
	}
}

void Board::DrawFadeOut(Graphics* g)
{
	if (mBoardFadeOutCounter < 0)
		return;

	int aFadeAlpha = 0;
	if (mBoardFadeOutCounter > 500)
	{
		aFadeAlpha = ClampInt((mBoardFadeOutCounter - 500) * 255 / 100, 0, 255);
	}
	else if (mBoardFadeOutCounter < 100)
	{
		aFadeAlpha = ClampInt((100 - mBoardFadeOutCounter) * 255 / 100, 0, 255);
	}

	if (aFadeAlpha > 0)
	{
		g->SetColor(Color(0, 0, 0, aFadeAlpha));
		g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	}
}

void Board::DrawTopRightUI(Graphics* g)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
		return;

	if (mSunMoney >= 0)
	{
		g->DrawImage(Sexy::IMAGE_SUN, 85, 53);
		char aMoneyStr[16];
		sprintf(aMoneyStr, "%d", mSunMoney);
		g->SetColor(Color(255, 255, 0));
		g->SetFont(Sexy::FONT_COUNTER);
		g->DrawString(aMoneyStr, 111, 77);
	}
}

void Board::DrawUIBottom(Graphics* g)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK_BLANK, 0, 508);
		DrawTopRightUI(g);
		return;
	}

	if (mApp->IsSlotMachineLevel())
	{
		g->DrawImage(Sexy::IMAGE_SLOT_MACHINE_BACKGROUND, 220, 510);
	}
	else if (mApp->IsIZombieLevel())
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK_ZOMBIE, 0, 510);
	}
	else if (HasConveyorBeltSeedBank())
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK_CONVEYORBELT, 0, 510);
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE)
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK_ICE, 0, 508);
	}
	else if (mApp->IsSquirrelLevel())
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK_BLANK, 0, 508);
	}
	else
	{
		g->DrawImage(Sexy::IMAGE_SEEDBANK, 0, 508);
	}

	mSeedBank->Draw(g);
	DrawTopRightUI(g);
}

void Board::DrawUICoinBank(Graphics* g)
{
	if (mCoinBankFadeCount == 0)
		return;

	int aFade = ClampInt(mCoinBankFadeCount * 25, 0, 255);
	if (aFade <= 0)
		return;

	g->SetColor(Color(0, 0, 0, aFade / 2));
	g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	if (mApp->mPlayerInfo)
	{
		char aCoinsStr[64];
		sprintf(aCoinsStr, "Coins: %d", mApp->mPlayerInfo->mCoins);
		g->SetFont(Sexy::FONT_DWARVEN);
		g->SetColor(Color(255, 255, 255, aFade));
		g->DrawString(aCoinsStr, 350, 300);
	}
}

void Board::ClearFogAroundPlant(Plant* thePlant, int theRange)
{
	if (!StageHasFog())
		return;

	int aGridX = thePlant->mPlantCol;
	int aGridY = thePlant->mRow;

	for (int x = aGridX - theRange; x <= aGridX + theRange; x++)
	{
		for (int y = aGridY - theRange; y <= aGridY + theRange; y++)
		{
			if (x >= 0 && x < MAX_GRID_SIZE_X && y >= 0 && y < MAX_GRID_SIZE_Y + 1)
			{
				int aValue = MAX(mGridCelFog[x][y] - 85, 0);
				mGridCelFog[x][y] = aValue;
			}
		}
	}
}

void Board::UpdateFog()
{
	if (!StageHasFog())
		return;

	if (mFogBlownCountDown > 0)
	{
		mFogBlownCountDown--;
		return;
	}

	mFogBlownCountDown = FOG_BLOW_RETURN_TIME;

	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++)
		{
			if (mGridCelFog[x][y] < 255)
			{
				mGridCelFog[x][y] = MIN(mGridCelFog[x][y] + 5, 255);
			}
		}
	}

	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SEED_PLANTERN)
		{
			ClearFogAroundPlant(aPlant, 7);
		}
		else if (aPlant->mSeedType == SEED_TORCHWOOD && aPlant->mGlowCounter > 0)
		{
			ClearFogAroundPlant(aPlant, 1);
		}
	}
}

void Board::DrawFog(Graphics* g)
{
	Image* aImageFog = mApp->Is3DAccelerated() ? Sexy::IMAGE_FOG : Sexy::IMAGE_FOG_SOFTWARE;
	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++)
		{
			int aFadeAmount = mGridCelFog[x][y];
			if (aFadeAmount == 0)
				continue;

			int aCelLook = mGridCelLook[x][y % MAX_GRID_SIZE_Y];
			int aCelCol = aCelLook % 8;
			float aPosX = static_cast<float>(x * 80 + static_cast<int>(mFogOffset) - 15);
			float aPosY = static_cast<float>(y * 85 + 20);

			const uint32_t FOG_ANIM_PERIOD = 4500;
			float aTime = static_cast<float>(mMainCounter % FOG_ANIM_PERIOD) * static_cast<float>(PI) * 2.0f;
			float aPhaseX = 6.0f * static_cast<float>(PI) * static_cast<float>(x) / static_cast<float>(MAX_GRID_SIZE_X);
			float aPhaseY = 6.0f * static_cast<float>(PI) * static_cast<float>(y) / static_cast<float>(MAX_GRID_SIZE_Y + 1);
			float aMotion = 13.0f + 4.0f * sin(aTime / 900.0f + aPhaseY) + 8.0f * sin(aTime / 500.0f + aPhaseX);

			int aColorVariant = static_cast<int>(255.0f - static_cast<float>(aCelLook) * 1.5f - aMotion * 1.5f);
			int aLightnessVariant = static_cast<int>(255.0f - static_cast<float>(aCelLook) - aMotion);
			if (!mApp->Is3DAccelerated())
			{
				aPosX += 10.0f;
				aPosY += 3.0f;
				aCelCol = aCelLook % Sexy::IMAGE_FOG_SOFTWARE->mNumCols;
				aColorVariant = 255;
				aLightnessVariant = 255;
			}

			g->SetColorizeImages(true);
			g->SetColor(Color(aColorVariant, aColorVariant, aLightnessVariant, aFadeAmount));
			g->DrawImageCel(aImageFog, aPosX, aPosY, aCelCol, 0);

			if (x == MAX_GRID_SIZE_X - 1)
			{
				g->DrawImageCel(aImageFog, aPosX + 80, aPosY, aCelCol, 0);
			}
			g->SetColorizeImages(false);
		}
	}
}

bool Board::IsScaryPotterDaveTalking()
{
	return mApp->IsScaryPotterLevel() && mApp->mCrazyDaveState != CRAZY_DAVE_OFF;
}

void Board::DrawUITop(Graphics* g)
{
	mAdvice->Draw(g);

	if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && IsScaryPotterDaveTalking())
	{
		return;
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE && mIceTrapCounter > 0)
	{
		return;
	}

	if (mApp->mGameScene == SCENE_PLAYING)
	{
		DrawUIBottom(g);
	}

	mMenuButton->Draw(g);
	if (mStoreButton && !mStoreButton->mBtnNoDraw)
	{
		mStoreButton->Draw(g);
	}

	if (mShowShovel && mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		DrawShovel(g);
	}

	mToolTip->Draw(g);
	mCursorPreview->Draw(g);
	mCursorObject->Draw(g);
}

void Board::Draw(Graphics* g)
{
	g->SetLinearBlend(true);
	DrawLevel(g);
	g->SetLinearBlend(false);
	MarkDirtyFull();
}

void Board::SetMustacheMode(bool theMustacheMode)
{
	mMustacheMode = theMustacheMode;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		aZombie->ZombieInitializeMustache();
	}
}

void Board::SetFutureMode(bool theFutureMode)
{
	mFutureMode = theFutureMode;
}

void Board::SetPinataMode(bool thePinataMode)
{
	mPinataMode = thePinataMode;
}

void Board::SetDanceMode(bool theDanceMode)
{
	mDanceMode = theDanceMode;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		aZombie->ZombieInitializeMustache();
	}
}

void Board::SetSuperMowerMode(bool theSuperMowerMode)
{
	mSuperMowerMode = theSuperMowerMode;
}

void Board::SetDaisyMode(bool theDaisyMode)
{
	mDaisyMode = theDaisyMode;
}

void Board::SetSukhbirMode(bool theSukhbirMode)
{
	mSukhbirMode = theSukhbirMode;
}

void Board::DoTypingCheck()
{
	if (mApp->mWidgetManager->mWidgetsEnabled && gLawnApp->mTodCheatKeys && mApp->mGameScene == SCENE_PLAYING)
	{
		if (mApp->mWidgetManager->mKeyDown[13])
		{
			KeyChar(' ');
		}

		TypingCheck* aCheck = mApp->mTypingCheck;
		aCheck->TypingCheckAddKeyCheck(' ', " ") ;
		aCheck->TypingCheckAddKeyCheck(KEYCODE_LEFT, "l") ;
		aCheck->TypingCheckAddKeyCheck(KEYCODE_RIGHT, "r") ;
		aCheck->TypingCheckAddKeyCheck(KEYCODE_UP, "u") ;
		aCheck->TypingCheckAddKeyCheck(KEYCODE_DOWN, "d") ;
		aCheck->TypingCheckAddKeyCheck('t', "t") ;
		aCheck->TypingCheckAddKeyCheck('j', "j") ;
		aCheck->TypingCheckAddKeyCheck('g', "g") ;
		aCheck->TypingCheckAddKeyCheck('G', "G") ;
		aCheck->TypingCheckAddKeyCheck('i', "i") ;
		aCheck->TypingCheckAddKeyCheck('C', "C") ;
		aCheck->TypingCheckAddKeyCheck('1', "1") ;
		aCheck->TypingCheckAddKeyCheck('B', "B") ;
		aCheck->TypingCheckAddKeyCheck('r', "r") ;
		aCheck->TypingCheckAddKeyCheck('0', "0") ;
		aCheck->TypingCheckAddKeyCheck('9', "9") ;
		aCheck->TypingCheckAddKeyCheck('$', "$") ;
		aCheck->TypingCheckAddKeyCheck('m', "m") ;
		aCheck->TypingCheckAddKeyCheck('+', "+") ;
		aCheck->TypingCheckAddKeyCheck('-', "-") ;
		aCheck->TypingCheckAddKeyCheck('Q', "Q") ;
		aCheck->TypingCheckAddKeyCheck('F', "F") ;
		aCheck->TypingCheckAddKeyCheck('X', "X") ;
		aCheck->TypingCheckAddKeyCheck('Z', "Z") ;
		aCheck->TypingCheckAddKeyCheck('R', "R") ;
		aCheck->TypingCheckAddKeyCheck('K', "K") ;
		aCheck->TypingCheckAddKeyCheck('S', "S") ;
		aCheck->TypingCheckAddKeyCheck('O', "O") ;
		aCheck->TypingCheckAddKeyCheck('P', "P") ;
		aCheck->TypingCheckAddKeyCheck('Y', "Y") ;
		aCheck->TypingCheckAddKeyCheck('E', "E") ;
		aCheck->TypingCheckAddKeyCheck('N', "N") ;
		aCheck->TypingCheckAddKeyCheck('L', "L") ;
		aCheck->TypingCheckAddKeyCheck('H', "H") ;
	}
}

void Board::KeyDown(int theKey)
{
	if (theKey == KEYCODE_ESCAPE)
	{
		mApp->BoardButtonPress();
		return;
	}

	if (theKey == KEYCODE_SPACE || theKey == KEYCODE_RETURN)
	{
		DirtyAll();
		if (mApp->mGameScene == SCENE_PLAYING)
		{
			if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
			{
				SeedType aSeedType = static_cast<SeedType>(mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].mPacketType);
				int aCost = GetCurrentPlantCost(aSeedType, mCursorObject->mSeedBankIndex);
				if (aCost <= mSunMoney && mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
				{
					mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
					ClearCursor();
				}
			}
			else
			{
				mSeedBank->Activate();
			}
		}
		return;
	}

	if (theKey == KEYCODE_BACK)
	{
		if (mCursorObject->mCursorType == CURSOR_TYPE_SHOVEL)
		{
			ClearCursor();
			return;
		}
		if (mCursorObject->mCursorType == CURSOR_TYPE_PLANT_FROM_BANK)
		{
			ClearCursor();
			return;
		}
		if (mCursorObject->mCursorType == CURSOR_TYPE_WATERING_CAN ||
			mCursorObject->mCursorType == CURSOR_TYPE_FERTILIZER ||
			mCursorObject->mCursorType == CURSOR_TYPE_BUG_SPRAY ||
			mCursorObject->mCursorType == CURSOR_TYPE_PHONOGRAPH ||
			mCursorObject->mCursorType == CURSOR_TYPE_CHOCOLATE ||
			mCursorObject->mCursorType == CURSOR_TYPE_GLOVE ||
			mCursorObject->mCursorType == CURSOR_TYPE_MONEY_SIGN)
		{
			ClearCursor();
			return;
		}
	}

	if (mApp->mTodCheatKeys)
	{
		if (theKey == KEYCODE_LEFT)
		{
			AddZombie(ZOMBIE_NORMAL, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theKey == KEYCODE_RIGHT)
		{
			AddZombie(ZOMBIE_TRAFFIC_CONE, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theKey == KEYCODE_UP)
		{
			AddZombie(ZOMBIE_PAIL, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theKey == KEYCODE_DOWN)
		{
			AddZombie(ZOMBIE_SCREEN_DOOR, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theKey == KEYCODE_SPACE)
		{
			mApp->PlayFoley(FOLEY_SPAWN_SUN);
			AddCoin(RandRangeInt(40, 760), 0, COIN_SUN, COIN_MOTION_FROM_SKY);
			return;
		}
	}
}

void Board::KeyChar(char theChar)
{
	if (mApp->mTodCheatKeys && mApp->mGameScene == SCENE_PLAYING)
	{
		if (theChar == ' ')
		{
			mApp->PlayFoley(FOLEY_SPAWN_SUN);
			AddCoin(RandRangeInt(40, 760), 0, COIN_SUN, COIN_MOTION_FROM_SKY);
			return;
		}
		if (theChar == 'm')
		{
			AddZombie(ZOMBIE_NEWSPAPER, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'j')
		{
			AddZombie(ZOMBIE_JACK_IN_THE_BOX, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'g')
		{
			AddZombie(ZOMBIE_GARGANTUAR, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'G')
		{
			AddZombie(ZOMBIE_REDEYE_GARGANTUAR, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'i')
		{
			AddZombie(ZOMBIE_ZAMBONI, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'C')
		{
			AddZombie(ZOMBIE_CATAPULT, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == '1')
		{
			Plant* aPlant = GetTopPlantAt(0, 0, TOPPLANT_ANY);
			if (aPlant)
			{
				aPlant->Die();
				mChallenge->ZombieAtePlant(aPlant);
				return;
			}
		}
		if (theChar == 'B')
		{
			mFogBlownCountDown = 2200;
			return;
		}
		if (theChar == 't')
		{
			if (!CanAddBobSled())
			{
				int aRow = Rand(5);
				int aPos = 400;
				if (StageHasPool())
				{
					aRow = Rand(2);
				}
				else if (StageHasRoof())
				{
					aPos = 500;
				}
				mIceTimer[aRow] = 3000;
				mIceMinX[aRow] = aPos;
			}
			AddZombie(ZOMBIE_BOBSLED, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'r')
		{
			SpawnZombiesFromGraves();
			return;
		}
		if (theChar == '0')
		{
			AddSunMoney(100);
			mApp->PlaySample(SOUND_BUTTONCLICK);
			return;
		}
		if (theChar == '9')
		{
			AddSunMoney(999999);
			mApp->PlaySample(SOUND_BUTTONCLICK);
			return;
		}
		if (theChar == '$')
		{
			mApp->mPlayerInfo->AddCoins(100);
			mApp->PlaySample(SOUND_BUTTONCLICK);
			ShowCoinBank();
			return;
		}
		if (theChar == 'F' || theChar == 'f')
		{
			mApp->mMusic->GameMusicFadeInStop();
			mApp->mMusic->StopAllMusic();
			return;
		}
		if (theChar == 'X' || theChar == 'x')
		{
			mApp->mSoundSystem->StopAllSounds();
			return;
		}
		if (theChar == 'Z' || theChar == 'z')
		{
			if (mApp->mWidgetManager->mWidgetsEnabled)
			{
				mDebugTextMode = (DebugTextMode)(((int)mDebugTextMode + 1) % 3);
			}
			return;
		}
		if (theChar == 'R' || theChar == 'r')
		{
			mApp->mPlayerInfo->mHasUsedGardeningGlove = false;
			return;
		}
		if (theChar == 'K' || theChar == 'k')
		{
			Zombie* aZombie = NULL;
			while (IterateZombies(aZombie))
			{
				if (!aZombie->IsDeadOrDying())
				{
					aZombie->Die();
				}
			}
			return;
		}
		if (theChar == 'S' || theChar == 's')
		{
			AddZombie(ZOMBIE_SNORKEL, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'O' || theChar == 'o')
		{
			AddZombie(ZOMBIE_DOLPHIN_RIDER, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'P' || theChar == 'p')
		{
			AddZombie(ZOMBIE_POGO, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'Y' || theChar == 'y')
		{
			AddZombie(ZOMBIE_YETI, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'E' || theChar == 'e')
		{
			AddZombie(ZOMBIE_BUNGEE, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'N' || theChar == 'n')
		{
			AddZombie(ZOMBIE_BALLOON, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'L' || theChar == 'l')
		{
			AddZombie(ZOMBIE_LADDER, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'H' || theChar == 'h')
		{
			AddZombie(ZOMBIE_DIGGER, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'a' || theChar == 'A')
		{
			AddZombie(ZOMBIE_DANCER, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'b' || theChar == 'B')
		{
			AddZombie(ZOMBIE_BOSS, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'd' || theChar == 'D')
		{
			AddZombie(ZOMBIE_DUCKY_TUBE, ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'f' || theChar == 'F')
		{
			AddZombie(ZOMBIE_FOOTBALL, ZOMBIE_WAVE_DEBUG);
			return;
		}

		if (theChar == '+' || theChar == '=')
		{
			mApp->mMusic->GameMusicFadeInStop();
			mApp->mMusic->StopAllMusic();
			int aTune = mApp->mMusic->mCurrentMusicTuneIndex + 1;
			int aNumTunes = mApp->mMusic->GetNumTune();
			if (aTune >= aNumTunes)
				aTune = 0;
			mApp->mMusic->PlayMusic(aTune, 1, false, true);
			return;
		}
		if (theChar == '-' || theChar == '_')
		{
			mApp->mMusic->GameMusicFadeInStop();
			mApp->mMusic->StopAllMusic();
			int aTune = mApp->mMusic->mCurrentMusicTuneIndex - 1;
			int aNumTunes = mApp->mMusic->GetNumTune();
			if (aTune < 0)
				aTune = aNumTunes - 1;
			mApp->mMusic->PlayMusic(aTune, 1, false, true);
			return;
		}
		if (theChar == 'Q' || theChar == 'q')
		{
			mApp->mBoardResult = BOARDRESULT_RESTART;
			mApp->BoardSetUpdate();
			return;
		}
	}
}

void Board::AddSunMoney(int theAmount)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mPlayerInfo->mSunMoney += theAmount;
		return;
	}
	mSunMoney += theAmount;
}

int Board::CountSunBeingCollected()
{
	int aCount = 0;
	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mType == COIN_SUN && aCoin->mIsBeingCollected)
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountCoinsBeingCollected()
{
	int aCount = 0;
	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mIsBeingCollected)
		{
			aCount++;
		}
	}
	return aCount;
}

void Board::TakeSunMoney(int theAmount)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mPlayerInfo->mSunMoney -= theAmount;
		return;
	}
	mSunMoney -= theAmount;
}

bool Board::CanTakeSunMoney(int theAmount)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		return mApp->mPlayerInfo->mSunMoney >= theAmount;
	}
	return mSunMoney >= theAmount;
}

void Board::ProcessDeleteQueue()
{
	mPlants.DataArrayFreeAll();
	mZombies.DataArrayFreeAll();
	mProjectiles.DataArrayFreeAll();
	mCoins.DataArrayFreeAll();
	mLawnMowers.DataArrayFreeAll();
	mGridItems.DataArrayFreeAll();
}

bool Board::HasConveyorBeltSeedBank()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE || mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM || mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
		return false;
	return mChallenge->HasConveyorBeltSeedBank();
}

int Board::GetNumSeedsInBank()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE)
		return 6;
	if (mApp->IsSlotMachineLevel())
		return 3;
	if (mApp->IsIZombieLevel())
	{
		switch (mApp->mGameMode)
		{
		case GAMEMODE_PUZZLE_I_ZOMBIE_1: return 3;
		case GAMEMODE_PUZZLE_I_ZOMBIE_2: return 3;
		case GAMEMODE_PUZZLE_I_ZOMBIE_3: return 3;
		case GAMEMODE_PUZZLE_I_ZOMBIE_4: return 3;
		case GAMEMODE_PUZZLE_I_ZOMBIE_5: return 4;
		case GAMEMODE_PUZZLE_I_ZOMBIE_6: return 4;
		case GAMEMODE_PUZZLE_I_ZOMBIE_7: return 4;
		case GAMEMODE_PUZZLE_I_ZOMBIE_8: return 6;
		case GAMEMODE_PUZZLE_I_ZOMBIE_9: return 8;
		case GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS: return 9;
		default: return 0;
		}
	}
	if (mApp->IsScaryPotterLevel())
		return 1;
	if (mApp->IsWhackAZombieLevel())
		return 3;
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
		return 2;

	if (HasConveyorBeltSeedBank())
		return 4;

	return SEEDBANK_MAX;
}

bool Board::StageIsNight()
{
	return (mBackground == BACKGROUND_2_NIGHT || mBackground == BACKGROUND_MUSHROOM_GARDEN);
}

bool Board::StageHasGraveStones()
{
	return StageIsNight() && StageHasZombieWalkInFromRight();
}

bool Board::StageHasRoof()
{
	return (mBackground == BACKGROUND_5_ROOF || mBackground == BACKGROUND_6_BOSS);
}

bool Board::StageHasPool()
{
	return (mBackground == BACKGROUND_3_POOL || mBackground == BACKGROUND_4_FOG || mBackground == BACKGROUND_ZOMBIQUARIUM);
}

bool Board::StageHas6Rows()
{
	return StageHasPool();
}

bool Board::StageHasZombieWalkInFromRight()
{
	return !StageHasRoof() && mBackground != BACKGROUND_GREENHOUSE && mBackground != BACKGROUND_ZOMBIQUARIUM && mBackground != BACKGROUND_TREEOFWISDOM;
}

bool Board::StageHasFog()
{
	return mBackground == BACKGROUND_4_FOG;
}

bool Board::StageIsDayWithoutPool()
{
	return mBackground == BACKGROUND_1_DAY || mBackground == BACKGROUND_5_ROOF;
}

bool Board::StageIsDayWithPool()
{
	return mBackground == BACKGROUND_3_POOL;
}

int Board::LeftFogColumn()
{
	if (!StageHasFog())
		return 0;

	if (mApp->IsFirstTimeAdventureMode())
		return 3;

	return 2;
}

int Board::GetSeedPacketPositionX(int theIndex)
{
	return theIndex * 58 + 17;
}

int Board::GetSeedBankExtraWidth()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
		return 0;
	if (mApp->IsSlotMachineLevel())
		return 0;

	return GetNumSeedsInBank() < 7 ? 65 : 0;
}

int Board::OffsetYForPlanting(float theGridY)
{
	if (mBackground == BACKGROUND_5_ROOF || mBackground == BACKGROUND_6_BOSS)
	{
		return (int)(theGridY * 10);
	}
	return 0;
}

int Board::PlantingPixelToGridX(int theX, int theY)
{
	int aGridX = PixelToGridX(theX, theY);
	if (aGridX < 0 || aGridX >= MAX_GRID_SIZE_X)
		return -1;

	return aGridX;
}

int Board::PlantingPixelToGridY(int theX, int theY)
{
	int aGridY = PixelToGridY(theX, theY);
	if (aGridY < 0 || aGridY >= MAX_GRID_SIZE_Y)
		return -1;

	return aGridY;
}

int Board::PixelToGridX(int theX, int theY)
{
	return (theX - LAWN_XMIN) / 80;
}

int Board::PixelToGridXKeepOnBoard(int theX, int theY)
{
	int aGridX = (theX - LAWN_XMIN) / 80;
	return ClampInt(aGridX, 0, MAX_GRID_SIZE_X - 1);
}

int Board::PixelToGridY(int theX, int theY)
{
	return (theY - LAWN_YMIN) / 100;
}

int Board::PixelToGridYKeepOnBoard(int theX, int theY)
{
	int aGridY = (theY - LAWN_YMIN) / 100;
	return ClampInt(aGridY, 0, MAX_GRID_SIZE_Y - 1);
}

int Board::GridToPixelX(int theGridX, int theGridY)
{
	return theGridX * 80 + LAWN_XMIN;
}

int Board::GetPosYBasedOnRow(int theRow)
{
	return theRow * 85 + 80;
}

float Board::GetPosYBasedOnRow(float thePosX, int theRow)
{
	if (StageHasRoof())
		return (float)GetPosYBasedOnRow(theRow) + thePosX * 0.125f;
	return (float)GetPosYBasedOnRow(theRow);
}

int Board::GridToPixelY(int theGridX, int theGridY)
{
	return theGridY * 100 + LAWN_YMIN;
}

unsigned int Board::ZombieGetID(Zombie* theZombie)
{
	if (theZombie == NULL)
		return ZOMBIEID_NULL;
	return mZombies.DataArrayGetID(theZombie);
}

Zombie* Board::ZombieGet(unsigned int theID)
{
	return mZombies.DataArrayGet(theID);
}

Zombie* Board::ZombieTryToGet(unsigned int theID)
{
	return mZombies.DataArrayTryToGet(theID);
}

int GetRectOverlap(const Rect& theRect1, const Rect& theRect2)
{
	int aOverlapX = MIN(theRect1.mX + theRect1.mWidth, theRect2.mX + theRect2.mWidth) - MAX(theRect1.mX, theRect2.mX);
	int aOverlapY = MIN(theRect1.mY + theRect1.mHeight, theRect2.mY + theRect2.mHeight) - MAX(theRect1.mY, theRect2.mY);
	return aOverlapX * aOverlapY;
}

bool GetCircleRectOverlap(int theCircleX, int theCircleY, int theRadius, const Rect& theRect)
{
	float aNearX = MAX((float)theRect.mX, MIN((float)theCircleX, (float)(theRect.mX + theRect.mWidth)));
	float aNearY = MAX((float)theRect.mY, MIN((float)theCircleY, (float)(theRect.mY + theRect.mHeight)));
	float aDeltaX = (float)theCircleX - aNearX;
	float aDeltaY = (float)theCircleY - aNearY;
	return (aDeltaX * aDeltaX + aDeltaY * aDeltaY) < (float)(theRadius * theRadius);
}

bool Board::IterateZombies(Zombie*& theZombie)
{
	if (theZombie == NULL)
	{
		theZombie = mZombies.DataArrayFirst();
	}
	else
	{
		theZombie = mZombies.DataArrayNext(theZombie);
	}
	return theZombie != NULL;
}

bool Board::IteratePlants(Plant*& thePlant)
{
	if (thePlant == NULL)
	{
		thePlant = mPlants.DataArrayFirst();
	}
	else
	{
		thePlant = mPlants.DataArrayNext(thePlant);
	}
	return thePlant != NULL;
}

bool Board::IterateProjectiles(Projectile*& theProjectile)
{
	if (theProjectile == NULL)
	{
		theProjectile = mProjectiles.DataArrayFirst();
	}
	else
	{
		theProjectile = mProjectiles.DataArrayNext(theProjectile);
	}
	return theProjectile != NULL;
}

bool Board::IterateCoins(Coin*& theCoin)
{
	if (theCoin == NULL)
	{
		theCoin = mCoins.DataArrayFirst();
	}
	else
	{
		theCoin = mCoins.DataArrayNext(theCoin);
	}
	return theCoin != NULL;
}

bool Board::IterateLawnMowers(LawnMower*& theMower)
{
	if (theMower == NULL)
	{
		theMower = mLawnMowers.DataArrayFirst();
	}
	else
	{
		theMower = mLawnMowers.DataArrayNext(theMower);
	}
	return theMower != NULL;
}

bool Board::IterateGridItems(GridItem*& theGridItem)
{
	if (theGridItem == NULL)
	{
		theGridItem = mGridItems.DataArrayFirst();
	}
	else
	{
		theGridItem = mGridItems.DataArrayNext(theGridItem);
	}
	return theGridItem != NULL;
}

bool Board::IterateParticles(TodParticleSystem*& theParticle)
{
	if (theParticle == NULL)
	{
		theParticle = mApp->mEffectSystem->mParticleSystems.DataArrayFirst();
	}
	else
	{
		theParticle = mApp->mEffectSystem->mParticleSystems.DataArrayNext(theParticle);
	}
	return theParticle != NULL;
}

bool Board::IterateReanimations(Reanimation*& theReanimation)
{
	if (theReanimation == NULL)
	{
		theReanimation = mApp->mEffectSystem->mReanimations.DataArrayFirst();
	}
	else
	{
		theReanimation = mApp->mEffectSystem->mReanimations.DataArrayNext(theReanimation);
	}
	return theReanimation != NULL;
}

void Board::KillAllPlantsInRadius(int theGridX, int theGridY, int theRadius)
{
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mDead || aPlant->NotOnGround())
			continue;

		if (GridInRange(aPlant->mPlantCol, aPlant->mRow, theGridX, theGridY, theRadius, theRadius))
		{
			aPlant->Die();
		}
	}
}

bool Board::GridInRange(int theGridX, int theGridY, int theTargetX, int theTargetY, int theRangeX, int theRangeY)
{
	return abs(theGridX - theTargetX) <= theRangeX && abs(theGridY - theTargetY) <= theRangeY;
}

SeedNotRecommend Board::SeedNotRecommendedForLevel(SeedType theSeedType)
{
	bool aIsPlant = !Plant::IsFungus(theSeedType);
	bool aIsWater = Plant::IsAquatic(theSeedType);
	bool aIsHighGround = Plant::CanPlaceOnHighGround(theSeedType);

	if (StageHasPool() && !aIsWater && !aIsPlant)
		return NOT_RECOMMENDED_WATER;
	if (StageIsNight() && aIsPlant && theSeedType != SEED_SUNFLOWER && !Plant::IsFungus(theSeedType) && theSeedType != SEED_IMITATER)
		return NOT_RECOMMENDED_NOCTURNAL;
	if (StageHasRoof() && aIsHighGround)
		return NOT_RECOMMENDED_HIGHGROUND;
	if (StageHasFog() && theSeedType == SEED_PLANTERN)
		return NOT_RECOMMENDED_FOG;
	return NOT_RECOMMENDED_NONE;
}

int Board::CountCoinByType(CoinType theCoinType)
{
	int aCount = 0;
	Coin* aCoin = NULL;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mType == theCoinType && aCoin->mBoard)
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::GetGraveStoneCount()
{
	return GetGraveStonesCount();
}

void Board::DropLootPiece(float thePosX, float thePosY, CoinType aCoinType)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
		return;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		if (aCoinType == COIN_GOLD || aCoinType == COIN_GOLD_SMALL || aCoinType == COIN_SILVER)
			return;
	}
	if (mApp->IsWallnutBowlingLevel() && Coin::IsMoney(aCoinType))
		return;

	if (mApp->IsFirstTimeAdventureMode() && mLevel == 11)
	{
		int aMoney = Coin::GetCoinValue(COIN_GOLD) * mLawnMowers.mSize;
		int aCost = StoreScreen::GetItemCost(STORE_ITEM_PACKET_UPGRADE);
		aMoney += mApp->mPlayerInfo->mCoins + CountCoinsBeingCollected();
		if (Coin::GetCoinValue(aCoinType) + aMoney >= aCost)
		{
			return;
		}
	}

	mApp->PlayFoley(FOLEY_SPAWN_SUN);
	AddCoin((int)thePosX - 40, (int)thePosY, aCoinType, COIN_MOTION_COIN);
	mDroppedFirstCoin = true;
}

bool Board::CanDropLoot()
{
	return !mCutScene->ShouldRunUpsellBoard() && (!mApp->IsFirstTimeAdventureMode() || mLevel >= 11);
}

bool Board::BungeeIsTargetingCell(int theGridX, int theGridY)
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mZombieType == ZOMBIE_BUNGEE && aZombie->mRow == theGridY && aZombie->mTargetCol == theGridX)
		{
			return true;
		}
	}
	return false;
}

Zombie* Board::GetBossZombie()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mZombieType == ZOMBIE_BOSS)
		{
			return aZombie;
		}
	}
	return NULL;
}

Plant* Board::FindUmbrellaPlant(int theGridX, int theGridY)
{
	Plant* aPlant = NULL;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SEED_UMBRELLA && !aPlant->NotOnGround() && GridInRange(theGridX, theGridY, aPlant->mPlantCol, aPlant->mRow, 1, 1))
		{
			return aPlant;
		}
	}
	return NULL;
}

void Board::DoFwoosh(int theRow)
{
	int aRenderOrder = MakeRenderOrder(RENDER_LAYER_PARTICLE, theRow, 10000);
	Reanimation* aReanim = mApp->AddReanimation(0, 0, aRenderOrder, REANIM_FIRE_BREATH);
	aReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
	aReanim->mAnimRate = 30.0f;
	aReanim->mAnimTime = 0.0f;

	for (int x = 0; x < 12; x++)
	{
		mFwooshID[theRow][x] = mApp->ReanimationGetID(aReanim);
	}
}

void Board::UpdateFwoosh()
{
	for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < 12; x++)
		{
			Reanimation* aReanim = mApp->ReanimationTryToGet(mFwooshID[y][x]);
			if (aReanim)
			{
				aReanim->mAnimRate = 30.0f;
			}
		}
	}
}

void Board::UpdateGridItems()
{
	GridItem* aGridItem = NULL;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mDead)
			continue;

		aGridItem->Update();
	}
}

bool Board::PlantingRequirementsMet(SeedType theSeedType, int theGridX, int theGridY)
{
	if (!mApp->IsFirstTimeAdventureMode() || mLevel > 1)
		return true;

	if (theSeedType == SEED_PEASHOOTER && theGridX == 4 && theGridY == 2)
		return true;

	return false;
}

int Board::KillAllZombiesInRadius(int theGridX, int theGridY, int theRadius, int theRow, int theDamageRange, bool theBurn, int theMinZombiesKilled)
{
	(void)theDamageRange;
	(void)theMinZombiesKilled;
	int aCount = 0;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mDead)
			continue;

		if (GridInRange(PixelToGridX((int)aZombie->mPosX, (int)aZombie->mPosY), aZombie->mRow, theGridX, theGridY, theRadius, theRadius))
		{
			aZombie->Die();
			aCount++;
			if (theBurn)
			{
				mApp->AddTodParticle(aZombie->mPosX, aZombie->mPosY, RENDER_LAYER_PARTICLE, PARTICLE_ZOMBIE_BURNING);
			}
		}
	}
	return aCount;
}

int Board::GetNumWavesPerSurvivalStage()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
		return 20;
	return 10;
}

void Board::RemoveParticleByType(ParticleEffect theEffectType)
{
	TodParticleSystem* aParticle = NULL;
	while (IterateParticles(aParticle))
	{
		if (aParticle->mEffectType == theEffectType)
		{
			aParticle->ParticleSystemDie();
		}
	}
}

bool Board::PlantUsesAcceleratedPricing(SeedType theSeedType)
{
	if (mApp->IsFirstTimeAdventureMode())
		return false;

	int aCount = CountPlantByType(theSeedType);
	return aCount > 3;
}

int Board::GetCurrentPlantCost(SeedType theSeedType, int theSeedIndex)
{
	int aCost = GetSeedDefinition(theSeedType).mSunCost;
	if (PlantUsesAcceleratedPricing(theSeedType))
	{
		int aCount = CountPlantByType(theSeedType);
		aCost += aCount * 50;
	}
	return aCost;
}

bool Board::CanUseGameObject(GameObjectType theObjectType)
{
	if (theObjectType == OBJECT_TYPE_SHOVEL)
	{
		if (!mApp->mPlayerInfo->mPurchases[STORE_ITEM_SHOVEL])
			return false;

		if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM)
			return false;

		return true;
	}

	if (theObjectType >= OBJECT_TYPE_WATERING_CAN && theObjectType <= OBJECT_TYPE_WHEELBARROW)
	{
		if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM)
			return false;

		switch (theObjectType)
		{
		case OBJECT_TYPE_WATERING_CAN:
			return true;
		case OBJECT_TYPE_FERTILIZER:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_FERTILIZER];
		case OBJECT_TYPE_BUG_SPRAY:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_BUG_SPRAY];
		case OBJECT_TYPE_PHONOGRAPH:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_PHONOGRAPH];
		case OBJECT_TYPE_CHOCOLATE:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_CHOCOLATE];
		case OBJECT_TYPE_GLOVE:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_GLOVE];
		case OBJECT_TYPE_MONEY_SIGN:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_MONEY_SIGN];
		case OBJECT_TYPE_WHEELBARROW:
			return mApp->mPlayerInfo->mPurchases[STORE_ITEM_WHEELBARROW];
		case OBJECT_TYPE_TREE_FOOD:
			return mChallenge->mTreeFoodCount > 0;
		case OBJECT_TYPE_TREE_OF_WISDOM:
			return true;
		default:
			return false;
		}
	}

	return false;
}

void Board::ShakeBoard(int theShakeCounter, int theShakeAmount)
{
	mShakeCounter = theShakeCounter;
	mShakeAmountX = theShakeAmount;
	mShakeAmountY = theShakeAmount;
}

LawnMower* Board::FindLawnMowerInRow(int theRow)
{
	LawnMower* aMower = NULL;
	while (IterateLawnMowers(aMower))
	{
		if (aMower->mRow == theRow && aMower->mMowerState == MOWER_READY)
		{
			return aMower;
		}
	}
	return NULL;
}

Zombie* Board::GetWinningZombie()
{
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->GetZombieRect().mX + aZombie->GetZombieRect().mWidth <= -20)
		{
			return aZombie;
		}
	}
	return NULL;
}

int Board::CountZombieByType(ZombieType theZombieType)
{
	int aCount = 0;
	Zombie* aZombie = NULL;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mZombieType == theZombieType)
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::NumberZombiesInWave(int theWave)
{
	int aCount = 0;
	for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
	{
		if (mZombiesInWave[theWave][i] == ZOMBIE_INVALID)
			break;
		aCount++;
	}
	return aCount;
}

bool Board::IsZombieTypeSpawnedOnly(ZombieType theZombieType)
{
	return (theZombieType == ZOMBIE_BUNGEE || theZombieType == ZOMBIE_BALLOON);
}
// Stub definition for linker
void UpdateSeedBankMouseOver() {}
