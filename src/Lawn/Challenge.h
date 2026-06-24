/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include "../ConstEnums.h"
#include "../GameConstants.h"
#include "../Sexy.TodLib/FilterEffect.h"
#include "../engine/Graphics.h"

#define BEGHOULED_MAX_GRIDSIZEX 8
#define BEGHOULED_MAX_GRIDSIZEY 5
#define ART_CHALLEGE_SIZE_X MAX_GRID_SIZE_X
#define MAX_PICK_GRID_SIZE 50

const int BEGHOULED_WINNING_SCORE = 75;
const int SLOT_MACHINE_WINNING_SCORE = 2000;
const int ZOMBIQUARIUM_WINNING_SCORE = 1000;
const int I_ZOMBIE_WINNING_SCORE = 5;
const int MAX_PORTALS = 4;
const int MAX_SQUIRRELS = 7;
const int MAX_SCARY_POTS = 54;
const int STORM_FLASH_TIME = 150;

namespace Sexy { class LawnApp; }
class Board;
class Plant;
class Zombie;
class GridItem;
class SeedPacket;
class HitResult;
struct TodWeightedGridArray;

enum BeghouledUpgrade
{
    BEGHOULED_UPGRADE_REPEATER,
    BEGHOULED_UPGRADE_FUMESHROOM,
    BEGHOULED_UPGRADE_TALLNUT,
    NUM_BEGHOULED_UPGRADES
};

struct BeghouledBoardState
{
	SeedType		        mSeedType[9][6];
};

class Challenge
{
public:
	Sexy::LawnApp*			mApp;
	Board*					mBoard;
	int32_t					mBeghouledMouseCapture;
    int32_t                 mBeghouledMouseDownX;
    int32_t                 mBeghouledMouseDownY;
    int32_t                mBeghouledEated[9][6];
    int32_t                mBeghouledPurcasedUpgrade[NUM_BEGHOULED_UPGRADES];
    int32_t                 mBeghouledMatchesThisMove;
    ChallengeState          mChallengeState;
    int32_t                 mChallengeStateCounter;
    int32_t                 mConveyorBeltCounter;
    int32_t                 mChallengeScore;
    int32_t                mShowBowlingLine;
    SeedType                mLastConveyorSeedType;
    int32_t                 mSurvivalStage;
    int32_t                 mSlotMachineRollCount;
    ReanimationID           mReanimChallenge;
    ReanimationID           mReanimClouds[6];
    int32_t                 mCloudsCounter[6];
    int32_t                 mChallengeGridX;
    int32_t                 mChallengeGridY;
    int32_t                 mScaryPotterPots;
    int32_t                 mRainCounter;
    int32_t                 mTreeOfWisdomTalkIndex;
    int32_t                 mTreeFoodCount;

public:
    Challenge() {}

    void                    StartLevel() {}
    void                    BeghouledPopulateBoard() {}
    void                    LoadBeghouledBoardState(BeghouledBoardState* theState) { (void)theState; }
    SeedType                BeghouledPickSeed(int theGridX, int theGridY, BeghouledBoardState* theBoardState, int theAllowMatches) { (void)theGridX; (void)theGridY; (void)theBoardState; (void)theAllowMatches; return SEED_NONE; }
    int                    BeghouledBoardHasMatch(BeghouledBoardState* theBoardState) { (void)theBoardState; return 0; }
    /*inline*/ SeedType     BeghouledGetPlantAt(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; return SEED_NONE; }
    int                     BeghouledVerticalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; return 0; }
    int                     BeghouledHorizontalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; return 0; }
    /*inline*/ void         BeghouledDragStart(int x, int y) { (void)x; (void)y; }
    void                    BeghouledDragUpdate(int x, int y) { (void)x; (void)y; }
    inline void             BeghouledDragCancel() { mBeghouledMouseCapture = false; }
    int                    MouseMove(int x, int y) { (void)x; (void)y; return 0; }
    int                    MouseDown(int x, int y, int theClickCount, HitResult* theHitResult) { (void)x; (void)y; (void)theClickCount; (void)theHitResult; return 0; }
    int                    MouseUp(int x, int y) { (void)x; (void)y; return 0; }
    void                    ClearCursor() {}
    void                    BeghouledRemoveHorizontalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; }
    void                    BeghouledRemoveVerticalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; }
    void                    BeghouledRemoveMatches(BeghouledBoardState* theBoardState) { (void)theBoardState; }
    void                    Update() {}
    void                    UpdateBeghouled() {}
    int                    UpdateBeghouledPlant(Plant* thePlant) { (void)thePlant; return 0; }
    void                    BeghouledFallIntoSquare(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; }
    void                    BeghouledMakePlantsFall(BeghouledBoardState* theBoardState) { (void)theBoardState; }
    void                    ZombieAtePlant(/*Zombie* theZombie,*/ Plant* thePlant) { (void)thePlant; }
    void                    DrawBackdrop(Sexy::Graphics* g) { (void)g; }
    void                    DrawArtChallenge(Sexy::Graphics* g) { (void)g; }
    void                    CheckForCompleteArtChallenge(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; }
    /*inline*/ SeedType     GetArtChallengeSeed(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; return SEED_NONE; }
    inline void             PlantAdded(Plant*) {}
    PlantingReason          CanPlantAt(int theGridX, int theGridY, SeedType theSeedType) { (void)theGridX; (void)theGridY; (void)theSeedType; return PLANTING_OK; }
    void                    DrawBeghouled(Sexy::Graphics* g) { (void)g; }
    int                    BeghouledIsValidMove(int theFromX, int theFromY, int theToX, int theToY, BeghouledBoardState* theBoardState) { (void)theFromX; (void)theFromY; (void)theToX; (void)theToY; (void)theBoardState; return 0; }
    int                    BeghouledCheckForPossibleMoves(BeghouledBoardState* theBoardState) { (void)theBoardState; return 0; }
    void                    BeghouledCheckStuckState() {}
    void                    InitZombieWavesSurvival() {}
    /*inline*/ void         InitZombieWavesFromList(ZombieType* theZombieList, int theListLength) { (void)theZombieList; (void)theListLength; }
    void                    InitZombieWaves() {}
    /*inline*/ Rect         SlotMachineGetHandleRect() { return Rect(); }
    void                    UpdateSlotMachine() {}
    void                    DrawSlotMachine(Sexy::Graphics* g) { (void)g; }
    int                    UpdateToolTip(int theX, int theY) { (void)theX; (void)theY; return 0; }
    void                    WhackAZombieSpawning() {}
    int                    UpdateZombieSpawning() { return 0; }
    void                    BeghouledClearCrater(int theCount) { (void)theCount; }
    void                    MouseDownWhackAZombie(int theX, int theY) { (void)theX; (void)theY; }
    void                    DrawStormNight(Sexy::Graphics* g) { (void)g; }
    void                    UpdateStormyNight() {}
    void                    InitLevel() {}
    void                    SpawnZombieWave() {}
    void                    GraveDangerSpawnRandomGrave() {}
    void                    GraveDangerSpawnGraveAt(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; }
    void                    SpawnLevelAward(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; }
    void                    BeghouledScore(int theGridX, int theGridY, int theNumPlants, int theIsHorizontal) { (void)theGridX; (void)theGridY; (void)theNumPlants; (void)theIsHorizontal; }
    void                    DrawStormFlash(Sexy::Graphics* g, int theTime, int theMaxAmount) { (void)g; (void)theTime; (void)theMaxAmount; }
    void                    UpdateRainingSeeds() {}
    /*inline*/ void         PlayBossEnter() {}
    void                    UpdateConveyorBelt() {}
    inline void             PortalStart() {}
    void                    UpdatePortalCombat() {}
    GridItem*               GetOtherPortal(GridItem* thePortal) { (void)thePortal; return NULL; }
    void                    UpdatePortal(GridItem* thePortal) { (void)thePortal; }
    inline int              PortalCombatRowSpawnWeight(int) { return 0; }
    inline bool             CanTargetZombieWithPortals(Plant*, Zombie*) { return false; }
    GridItem*               GetPortalToRight(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; return NULL; }
    GridItem*               GetPortalAt(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; return NULL; }
    void                    MoveAPortal() {}
    int                     GetPortalDistanceToMower(int theGridY) { (void)theGridY; return 0; }
    GridItem*               GetPortalToLeft(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; return NULL; }
    void                    BeghouledPacketClicked(SeedPacket* theSeedPacket) { (void)theSeedPacket; }
    void                    BeghouledShuffle() {}
    inline bool             BeghouledCanClearCrater() { return true; }
    inline void             BeghouledUpdateCraters() {}
    Zombie*                 ZombiquariumSpawnSnorkle() { return NULL; }
    void                    ZombiquariumPacketClicked(SeedPacket* theSeedPacket) { (void)theSeedPacket; }
    void                    ZombiquariumMouseDown(int x, int y) { (void)x; (void)y; }
    void                    ZombiquariumDropBrain(int x, int y) { (void)x; (void)y; }
    void                    ZombiquariumUpdate() {}
    /*inline*/ void         ShovelAddWallnuts() {}
    void                    ScaryPotterPlacePot(ScaryPotType theScaryPotType, ZombieType theZombieType, SeedType theSeedType, int theCount, TodWeightedGridArray* theGridArray, int theGridArrayCount) { (void)theScaryPotType; (void)theZombieType; (void)theSeedType; (void)theCount; (void)theGridArray; (void)theGridArrayCount; }
    void                    ScaryPotterStart() {}
    void                    ScaryPotterUpdate() {}
    void                    ScaryPotterOpenPot(GridItem* theScaryPot) { (void)theScaryPot; }
    void                    ScaryPotterJackExplode(int thePosX, int thePosY) { (void)thePosX; (void)thePosY; }
    int                    ScaryPotterIsCompleted() { return 0; }
    void                    ScaryPotterChangePotType(GridItemState thePotType, int theCount) { (void)thePotType; (void)theCount; }
    inline void             ScaryPotterPopulate() {}
    /*inline*/ void         ScaryPotterDontPlaceInCol(int theCol, TodWeightedGridArray* theGridArray, int theGridArrayCount) { (void)theCol; (void)theGridArray; (void)theGridArrayCount; }
    void                    ScaryPotterFillColumnWithPlant(int theCol, SeedType theSeedType, TodWeightedGridArray* theGridArray, int theGridArrayCount) { (void)theCol; (void)theSeedType; (void)theGridArray; (void)theGridArrayCount; }
    inline void             PuzzleNextStageClear() {}
    void                    ScaryPotterMalletPot(GridItem* theScaryPot) { (void)theScaryPot; }
    static inline ZombieType IZombieSeedTypeToZombieType(SeedType t) { (void)t; return ZOMBIE_NORMAL; }
    static inline bool      IsZombieSeedType(SeedType t) { (void)t; return false; }
    int                    IsZombieSeed(SeedType theSeedType) { return IsZombieSeedType(theSeedType); }
    int                    HasConveyorBeltSeedBank() { return EFalse; }
    void                   StoreButtonPressed() {}
    void                   TreeOfWisdomFeed() {}
    void                   TreeOfWisdomFeed(class Plant* thePlant) { (void)thePlant; }
    void                   TreeOfWisdomBoost() {}
    void                    IZombieMouseDownWithZombie(int theX, int theY, int theClickCount) { (void)theX; (void)theY; (void)theClickCount; }
    inline void             IZombieStart() {}
    void                    IZombiePlacePlants(SeedType theSeedType, int theCount, int theGridY = -1) { (void)theSeedType; (void)theCount; (void)theGridY; }
    void                    IZombieUpdate() {}
    inline void             IZombieDrawPlant(Sexy::Graphics*, Plant*) {}
    void                    IZombieSetPlantFilterEffect(Plant* thePlant, FilterEffect theFilterEffect) { (void)thePlant; (void)theFilterEffect; }
    inline int              ScaryPotterCountSunInPot(GridItem*) { return 0; }
    int                     ScaryPotterCountPots() { return 0; }
    inline void             IZombieInitLevel() {}
    void                    DrawRain(Sexy::Graphics* g) { (void)g; }
    void                    DrawWeather(Sexy::Graphics* g) { (void)g; }
    void                    SquirrelUpdate() {}
    /*inline*/ int          SquirrelCountUncaught() { return 0; }
    void                    SquirrelStart() {}
    void                    SquirrelFound(GridItem* theSquirrel) { (void)theSquirrel; }
    void                    SquirrelPeek(GridItem* theSquirrel) { (void)theSquirrel; }
    void                    SquirrelChew(GridItem* theSquirrel) { (void)theSquirrel; }
    void                    SquirrelUpdateOne(GridItem* theSquirrel) { (void)theSquirrel; }
    void                    IZombieSetupPlant(Plant* thePlant) { (void)thePlant; }
    void                    UpdateRain() {}
    int                    IZombieEatBrain(Zombie* theZombie) { (void)theZombie; return 0; }
    inline GridItem*          IZombieGetBrainTarget(Zombie*) { return NULL; }
    /*inline*/ void         IZombiePlacePlantInSquare(SeedType theSeedType, int theGridX, int theGridY = -1) { (void)theSeedType; (void)theGridX; (void)theGridY; }
    void                    AdvanceCrazyDaveDialog() {}
    void                    BeghouledFlashPlant(int theFlashX, int theFlashY, int theFromX, int theFromY, int theToX, int theToY) { (void)theFlashX; (void)theFlashY; (void)theFromX; (void)theFromY; (void)theToX; (void)theToY; }
    void                    BeghouledFlashAMatch() {}
    int                    BeghouledFlashFromBoardState(BeghouledBoardState* theBoardState, int theFromX, int theFromY, int theToX, int theToY) { (void)theBoardState; (void)theFromX; (void)theFromY; (void)theToX; (void)theToY; return 0; }
    inline void             IZombiePlantDropRemainingSun(Plant*) {}
    void                    IZombieSquishBrain(GridItem* theBrain) { (void)theBrain; }
    void                    IZombieScoreBrain(GridItem* theBrain) { (void)theBrain; }
    void                    LastStandUpdate() {}
    void                    WhackAZombiePlaceGraves(int theGraveCount) {}
    int                    BeghouledTwistSquareFromMouse(int theX, int theY, int& theGridX, int& theGridY) { (void)theX; (void)theY; (void)theGridX; (void)theGridY; return 0; }
    int                    BeghouledTwistValidMove(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; return 0; }
    void                    BeghouledTwistMouseDown(int x, int y) { (void)x; (void)y; }
    int                    BeghouledTwistMoveCausesMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState) { (void)theGridX; (void)theGridY; (void)theBoardState; return 0; }
    int                    BeghouledTwistFlashMatch(BeghouledBoardState* theBoardState, int theGridX, int theGridY) { (void)theBoardState; (void)theGridX; (void)theGridY; return 0; }
    /*inline*/ void         BeghouledCancelMatchFlashing() {}
    void                    BeghouledStartFalling(ChallengeState theState) { (void)theState; }
    void                    BeghouledFillHoles(BeghouledBoardState* theBoardState, int theAllowMatches) { (void)theBoardState; (void)theAllowMatches; }
    /*inline*/ void         BeghouledMakeStartBoard() {}
    void                    BeghouledCreatePlants(BeghouledBoardState* theOldBoardState, BeghouledBoardState* theNewBoardState) { (void)theOldBoardState; (void)theNewBoardState; }
    void                    PuzzlePhaseComplete(int theGridX, int theGridY) { (void)theGridX; (void)theGridY; }
    /*inline*/ int         PuzzleIsAwardStage() { return 0; }
    void                    IZombiePlaceZombie(ZombieType theZombieType, int theGridX, int theGridY) { (void)theZombieType; (void)theGridX; (void)theGridY; }
    void                    WhackAZombieUpdate() {}
    inline bool             LastStandCompletedStage() { return true; }
    void                    TreeOfWisdomUpdate() {}
    void                    TreeOfWisdomFertilize() {}
    void                    TreeOfWisdomInit() {}
    /*inline*/ int         TreeOfWisdomMouseOn(int theX, int theY) { (void)theX; (void)theY; return 0; }
    /*inline*/ int          TreeOfWisdomGetSize() { return 0; }
    void                    TreeOfWisdomDraw(Sexy::Graphics* g) { (void)g; }
    /*inline*/ void         TreeOfWisdomNextGarden() {}
    /*inline*/ void         TreeOfWisdomToolUpdate(GridItem* theZenTool) { (void)theZenTool; }
    void                    TreeOfWisdomOpenStore() {}
    void                    TreeOfWisdomLeave() {}
    void                    TreeOfWisdomGrow() {}
    /*inline*/ void         TreeOfWisdomTool(int theMouseX, int theMouseY) { (void)theMouseX; (void)theMouseY; }
    int                    TreeOfWisdomHitTest(int theX, int theY, HitResult* theHitResult) { (void)theX; (void)theY; (void)theHitResult; return 0; }
    void                    TreeOfWisdomBabble() {}
    void                    TreeOfWisdomGiveWisdom() {}
    void                    TreeOfWisdomSayRepeat() {}
    int                    TreeOfWisdomCanFeed() { return 0; }

    GridItem*               GetPortalLeftRight(int theGridX, int theGridY, int theToLeft = true) { (void)theGridX; (void)theGridY; (void)theToLeft; return NULL; }
};

extern SeedType gArtChallengeWallnut[6][9];
extern SeedType gArtChallengeSunFlower[6][9];
extern SeedType gArtChallengeStarFruit[6][9];

class ZombieAllowedLevels
{
public:
    ZombieType                      mZombieType;
    int32_t                         mAllowedOnLevel[50];
};
extern int gZombieWaves[NUM_LEVELS];
extern ZombieAllowedLevels gZombieAllowedLevels[NUM_ZOMBIE_TYPES];

#endif
