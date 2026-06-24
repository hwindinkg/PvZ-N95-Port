/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1.
 */

#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "GameObject.h"
#include "../GameConstants.h"

class HitResult;

#define MAX_ZOMBIE_FOLLOWERS 4
#define NUM_BOBSLED_FOLLOWERS 3
#define NUM_BACKUP_DANCERS 4
#define NUM_BOSS_BUNGEES 3

const int ZOMBIE_START_RANDOM_OFFSET = 40;
const int BUNGEE_ZOMBIE_HEIGHT = 3000;
const int RENDER_GROUP_SHIELD = 1;
const int RENDER_GROUP_ARMS = 2;
const int RENDER_GROUP_OVER_SHIELD = 3;
const int RENDER_GROUP_BOSS_BACK_LEG = 4;
const int RENDER_GROUP_BOSS_FRONT_LEG = 5;
const int RENDER_GROUP_BOSS_BACK_ARM = 6;
const int RENDER_GROUP_BOSS_FIREBALL_ADDITIVE = 7;
const int RENDER_GROUP_BOSS_FIREBALL_TOP = 8;
const int ZOMBIE_LIMP_SPEED_FACTOR = 2;
const int POGO_BOUNCE_TIME = 80;
const int DOLPHIN_JUMP_TIME = 120;
const int JackInTheBoxZombieRadius = 115;
const int JackInTheBoxPlantRadius = 90;
const int BOBSLED_CRASH_TIME = 150;
const int ZOMBIE_BACKUP_DANCER_RISE_HEIGHT = -200;
const int BOSS_FLASH_HEALTH_FRACTION = 10;
const int TICKS_BETWEEN_EATS = 4;
const int DAMAGE_PER_EAT = TICKS_BETWEEN_EATS;
const float THOWN_ZOMBIE_GRAVITY = 0.05f;
const float CHILLED_SPEED_FACTOR = 0.4f;
const float CLIP_HEIGHT_LIMIT = -100.0f;
const float CLIP_HEIGHT_OFF = -200.0f;
const Color ZOMBIE_MINDCONTROLLED_COLOR = Color(128, 0, 192, 255);

enum ZombieAttackType
{
    ATTACKTYPE_CHEW,
    ATTACKTYPE_DRIVE_OVER,
    ATTACKTYPE_VAULT,
    ATTACKTYPE_LADDER
};

enum ZombieParts
{
    PART_BODY,
    PART_HEAD,
    PART_HEAD_EATING,
    PART_TONGUE,
    PART_ARM,
    PART_HAIR,
    PART_HEAD_YUCKY,
    PART_ARM_PICKAXE,
    PART_ARM_POLEVAULT,
    PART_ARM_LEASH,
    PART_ARM_FLAG,
    PART_POGO,
    PART_DIGGER
};

class ZombieDrawPosition
{
public:
    int                             mHeadX;
    int                             mHeadY;
    int                             mArmY;
    float                           mBodyY;
    float                           mImageOffsetX;
    float                           mImageOffsetY;
    float                           mClipHeight;
};

class Plant;
class Reanimation;
class TodParticleSystem;
class Zombie : public GameObject
{
public:
    enum
    {
        ZOMBIE_WAVE_DEBUG = -1,
        ZOMBIE_WAVE_CUTSCENE = -2,
        ZOMBIE_WAVE_UI = -3,
        ZOMBIE_WAVE_WINNER = -4
    };

public:
	ZombieType			            mZombieType;
	ZombiePhase			            mZombiePhase;
	float				            mPosX;
	float				            mPosY;
	float				            mVelX;
    int32_t                         mAnimCounter;
    int32_t                         mGroanCounter;
    int32_t                         mAnimTicksPerFrame;
    int32_t                         mAnimFrames;
    int32_t                         mFrame;
    int32_t                         mPrevFrame;
    bool                            mVariant;
    bool                            mIsEating;
    int32_t                         mJustGotShotCounter;
    int32_t                         mShieldJustGotShotCounter;
    int32_t                         mShieldRecoilCounter;
    int32_t                         mZombieAge;
    ZombieHeight                    mZombieHeight;
    int32_t                         mPhaseCounter;
    int32_t                         mFromWave;
    bool                            mDroppedLoot;
    int32_t                         mZombieFade;
    bool                            mFlatTires;
    int32_t                         mUseLadderCol;
    int32_t                         mTargetCol;
    float                           mAltitude;
    bool                            mHitUmbrella;
    Rect                            mZombieRect;
    Rect                            mZombieAttackRect;
    int32_t                         mChilledCounter;
    int32_t                         mButteredCounter;
    int32_t                         mIceTrapCounter;
    bool                            mMindControlled;
    bool                            mBlowingAway;
    bool                            mHasHead;
    bool                            mHasArm;
    bool                            mHasObject;
    bool                            mInPool;
    bool                            mOnHighGround;
    bool                            mYuckyFace;
    int32_t                         mYuckyFaceCounter;
    HelmType                        mHelmType;
    int32_t                         mBodyHealth;
    int32_t                         mBodyMaxHealth;
    int32_t                         mHelmHealth;
    int32_t                         mHelmMaxHealth;
    ShieldType                      mShieldType;
    int32_t                         mShieldHealth;
    int32_t                         mShieldMaxHealth;
    int32_t                         mFlyingHealth;
    int32_t                         mFlyingMaxHealth;
    bool                            mDead;
    ZombieID                        mRelatedZombieID;
    ZombieID                        mFollowerZombieID[MAX_ZOMBIE_FOLLOWERS];
    bool                            mPlayingSong;
    int32_t                         mParticleOffsetX;
    int32_t                         mParticleOffsetY;
    AttachmentID                    mAttachmentID;
    int32_t                         mSummonCounter;
    ReanimationID                   mBodyReanimID;
    float                           mScaleZombie;
    float                           mVelZ;
    float                           mOriginalAnimRate;
    int32_t                         mRiseFromGraveCounter;
    int32_t                         mRiseFromGraveGridX;
    PlantID                         mTargetPlantID;
    int32_t                         mBossMode;
    int32_t                         mTargetRow;
    int32_t                         mBossBungeeCounter;
    int32_t                         mBossStompCounter;
    int32_t                         mBossHeadCounter;
    ReanimationID                   mBossFireBallReanimID;
    ReanimationID                   mSpecialHeadReanimID;
    int32_t                         mFireballRow;
    bool                            mIsFireBall;
    ReanimationID                   mMoweredReanimID;
    int32_t                         mLastPortalX;

public:
    Zombie() {}
    ~Zombie() {}

    void                            ZombieInitialize(int theRow, ZombieType theType, bool theVariant, Zombie* theParentZombie, int theFromWave) { (void)theRow; (void)theType; (void)theVariant; (void)theParentZombie; (void)theFromWave; }
    void                            Animate() {}
    void                            CheckIfPreyCaught() {}
    void                            EatZombie(Zombie* theZombie) { (void)theZombie; }
    void                            EatPlant(Plant* thePlant) { (void)thePlant; }
    void                            Update() {}
    inline void                     DieNoLoot() {}
    /*inline*/ void                 DieWithLoot() {}
    void                            Draw(Graphics* g) { (void)g; }
//  void                            DrawZombie(Graphics* g, const ZombieDrawPosition& theDrawPos);
//  void                            DrawZombieWithParts(Graphics* g, const ZombieDrawPosition& theDrawPos);
    void                            DrawZombiePart(Graphics* g, Image* theImage, int theFrame, int theRow, const ZombieDrawPosition& theDrawPos) { (void)g; (void)theImage; (void)theFrame; (void)theRow; (void)theDrawPos; }
    void                            DrawBungeeCord(Graphics* g, int theOffsetX) { (void)g; (void)theOffsetX; }
    void                            TakeDamage(int theDamage, unsigned int theDamageFlags) { (void)theDamage; (void)theDamageFlags; }
    /*inline*/ void                 SetRow(int theRow) { (void)theRow; }
    float                           GetPosYBasedOnRow(int theRow) { (void)theRow; return 0.0f; }
    void                            ApplyChill(bool theIsIceTrap) { (void)theIsIceTrap; }
    void                            UpdateZombieBungee() {}
    void                            BungeeLanding() {}
    bool                            EffectedByDamage(unsigned int theDamageRangeFlags) { (void)theDamageRangeFlags; return false; }
    void                            PickRandomSpeed() {}
    void                            UpdateZombiePolevaulter() {}
    void                            UpdateZombieDolphinRider() {}
    void                            PickBungeeZombieTarget(int theColumn) { (void)theColumn; }
    int                             CountBungeesTargetingSunFlowers() { return 0; }
    Plant*                          FindPlantTarget(ZombieAttackType theAttackType) { (void)theAttackType; return NULL; }
    void                            CheckSquish(ZombieAttackType theAttackType) { (void)theAttackType; }
    void                            RiseFromGrave(int theCol, int theRow) { (void)theCol; (void)theRow; }
    void                            UpdateZombieRiseFromGrave() {}
    void                            UpdateDamageStates(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            UpdateZombiePool() {}
    void                            CheckForPool() {}
    void                            GetDrawPos(ZombieDrawPosition& theDrawPos) { (void)theDrawPos; }
    void                            UpdateZombieHighGround() {}
    void                            CheckForHighGround() {}
    bool                            IsOnHighGround() { return false; }
    void                            DropLoot() {}
    bool                            TrySpawnLevelAward() { return false; }
    /*inline*/ void                 StartZombieSound() {}
    void                            StopZombieSound() {}
    void                            UpdateZombieJackInTheBox() {}
    void                            DrawZombieHead(Graphics* g, const ZombieDrawPosition& theDrawPos, int theFrame) { (void)g; (void)theDrawPos; (void)theFrame; }
    void                            UpdateZombiePosition() {}
    Rect                            GetZombieRect() { return Rect(); }
    Rect                            GetZombieAttackRect() { return Rect(); }
    void                            UpdateZombieWalking() {}
    void                            UpdateZombieBobsled() {}
    void                            BobsledCrash() {}
    Plant*                          IsStandingOnSpikeweed() { return NULL; }
    void                            CheckForZombieStep() {}
    void                            CountExpectedMowers() { ; }
    /*inline*/ void                 OverrideParticleColor(TodParticleSystem* aParticle) { (void)aParticle; }
    /*inline*/ void                 OverrideParticleScale(TodParticleSystem* aParticle) { (void)aParticle; }
    void                            PoolSplash(bool theInToPoolSound) { (void)theInToPoolSound; }
    void                            UpdateZombieFlyer() {}
    void                            UpdateZombiePogo() {}
    void                            UpdateZombieNewspaper() {}
    void                            LandFlyer(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            UpdateZombieDigger() {}
    bool                            IsWalkingBackwards() { return false; }
    TodParticleSystem*              AddAttachedParticle(int thePosX, int thePosY, ParticleEffect theEffect) { (void)thePosX; (void)thePosY; (void)theEffect; return NULL; }
    void                            PogoBreak(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            UpdateZombieFalling() {}
    void                            UpdateZombieDancer() {}
    ZombieID                        SummonBackupDancer(int theRow, int thePosX) { (void)theRow; (void)thePosX; return ZOMBIEID_NULL; }
    void                            SummonBackupDancers() {}
    int                             GetDancerFrame() { return 0; }
    void                            BungeeStealTarget() {}
    void                            BungeeLiftTarget() {}
    void                            UpdateYuckyFace() {}
    void                            DrawIceTrap(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theFront) { (void)g; (void)theDrawPos; (void)theFront; }
    void                            HitIceTrap() {}
    int                             GetHelmDamageIndex() { return 0; }
    int                             GetShieldDamageIndex() { return 0; }
    void                            DrawReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, int theBaseRenderGroup) { (void)g; (void)theDrawPos; (void)theBaseRenderGroup; }
    void                            UpdatePlaying() {}
    bool                            NeedsMoreBackupDancers() { return false; }
    void                            ConvertToNormalZombie() {}
    void                            UpdateDancerWalking() { ; }
    void                            StartEating() {}
    void                            StopEating() {}
    void                            UpdateAnimSpeed() {}
    static inline const char*       ReanimShowPrefix(const char* prefix, int variant) { (void)prefix; (void)variant; return ""; }
    void                            PlayDeathAnim(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            UpdateDeath() {}
    void                            DrawShadow(Graphics* g) { (void)g; }
    bool                            HasShadow() { return false; }
    Reanimation*                    LoadReanim(ReanimationType theReanimationType) { (void)theReanimationType; return NULL; }
    /*inline*/ int                  TakeFlyingDamage(int theDamage, unsigned int theDamageFlags) { (void)theDamage; (void)theDamageFlags; return 0; }
    int                             TakeShieldDamage(int theDamage, unsigned int theDamageFlags) { (void)theDamage; (void)theDamageFlags; return 0; }
    int                             TakeHelmDamage(int theDamage, unsigned int theDamageFlags) { (void)theDamage; (void)theDamageFlags; return 0; }
    void                            TakeBodyDamage(int theDamage, unsigned int theDamageFlags) { (void)theDamage; (void)theDamageFlags; }
    void                            AttachShield() {}
    void                            DetachShield() {}
    void                            UpdateReanim() {}
    void                            GetTrackPosition(const char* theTrackName, float& thePosX, float& thePosY) { (void)theTrackName; (void)thePosX; (void)thePosY; }
    void                            LoadPlainZombieReanim() {}
    void                            ShowDoorArms(bool theShow) { (void)theShow; }
    /*inline*/ void                 ReanimShowTrack(const char* theTrackName, int theRenderGroup) { (void)theTrackName; (void)theRenderGroup; }
    /*inline*/ void                 PlayZombieAppearSound() {}
    void                            StartMindControlled() {}
    bool                            IsFlying() { return false; }
    void                            DropHead(unsigned int theDamageFlags) { (void)theDamageFlags; }
    bool                            CanTargetPlant(Plant* thePlant, ZombieAttackType theAttackType) { (void)thePlant; (void)theAttackType; return false; }
    void                            UpdateZombieCatapult() {}
    Plant*                          FindCatapultTarget() { return NULL; }
    void                            ZombieCatapultFire(Plant* thePlant) { (void)thePlant; }
    void                            UpdateClimbingLadder() {}
    void                            UpdateZombieGargantuar() {}
    int                             GetBodyDamageIndex() { return 0; }
    void                            ApplyBurn() {}
    void                            UpdateBurn() {}
    bool                            ZombieNotWalking() { return false; }
    Zombie*                         FindZombieTarget() { return NULL; }
    /*inline*/ void                 PlayZombieReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate) { (void)theTrackName; (void)theLoopType; (void)theBlendTime; (void)theAnimRate; }
    void                            UpdateZombieBackupDancer() {}
    ZombiePhase                     GetDancerPhase() { return PHASE_ZOMBIE_NORMAL; }
    bool                            IsMovingAtChilledSpeed() { return false; }
    void                            StartWalkAnim(int theBlendTime) { (void)theBlendTime; }
    Reanimation*                    AddAttachedReanim(int thePosX, int thePosY, ReanimationType theReanimType) { (void)thePosX; (void)thePosY; (void)theReanimType; return NULL; }
    void                            DragUnder() {}
    static /*inline*/ void          SetupDoorArms(Reanimation* aReanim, bool theShow) { (void)aReanim; (void)theShow; }
    static void                     SetupReanimLayers(Reanimation* aReanim, ZombieType theZombieType) { (void)aReanim; (void)theZombieType; }
    /*inline*/ bool                 IsOnBoard() { return false; }
    void                            DrawButter(Graphics* g, const ZombieDrawPosition& theDrawPos) { (void)g; (void)theDrawPos; }
    bool                            IsImmobilizied() { return false; }
    void                            ApplyButter() {}
    float                           ZombieTargetLeadX(float theTime) { (void)theTime; return 0.0f; }
    void                            UpdateZombieImp() {}
    void                            SquishAllInSquare(int theX, int theY, ZombieAttackType theAttackType) { (void)theX; (void)theY; (void)theAttackType; }
    void                            RemoveIceTrap() {}
    bool                            IsBouncingPogo() { return false; }
    int                             GetBobsledPosition() { return 0; }
    float                           PosX() { return mPosX; }
    float                           PosY() { return mPosY; }
    void                            Die() { DieNoLoot(); }
    bool                            MouseHitTest(int x, int y, HitResult* theHitResult) { (void)x; (void)y; (void)theHitResult; return false; }
    void                            StopZombieSounds() { StopZombieSound(); }
    void                            ZombieInitializeMustache(bool theEnable) { (void)theEnable; }
    void                            ZombieInitializeMustache() { ZombieInitializeMustache(false); }
    void                            DrawBobsledReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theBeforeZombie) { (void)g; (void)theDrawPos; (void)theBeforeZombie; }
    void                            BobsledDie() {}
    void                            BobsledBurn() {}
    bool                            IsBobsledTeamWithSled() { return false; }
    bool                            CanBeFrozen() { return false; }
    bool                            CanBeChilled() { return false; }
    void                            UpdateZombieSnorkel() {}
    void                            ReanimIgnoreClipRect(const char* theTrackName, bool theIgnoreClipRect) { (void)theTrackName; (void)theIgnoreClipRect; }
    void                            SetAnimRate(float theAnimRate) { (void)theAnimRate; }
    void                            ApplyAnimRate(float theAnimRate) { (void)theAnimRate; }
    /*inline*/ bool                 IsDeadOrDying() { return false; }
    void                            DrawDancerReanim(Graphics* g) { (void)g; }
    void                            DrawBungeeReanim(Graphics* g) { (void)g; }
    void                            DrawBungeeTarget(Graphics* g) { (void)g; }
    void                            BungeeDie() {}
    void                            ZamboniDeath(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            CatapultDeath(unsigned int theDamageFlags) { (void)theDamageFlags; }
    bool                            SetupDrawZombieWon(Graphics* g) { (void)g; return false; }
    void                            WalkIntoHouse() {}
    void                            UpdateZamboni() {}
    void                            UpdateZombieChimney() {}
    void                            UpdateLadder() {}
    void                            DropArm(unsigned int theDamageFlags) { (void)theDamageFlags; }
    bool                            CanLoseBodyParts() { return false; }
    void                            DropHelm(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            DropShield(unsigned int theDamageFlags) { (void)theDamageFlags; }
    void                            ReanimReenableClipping() {}
    void                            UpdateBoss() {}
    void                            BossPlayIdle() {}
    void                            BossRVLanding() {}
    void                            BossStompContact() {}
    bool                            BossAreBungeesDone() { return false; }
    void                            BossBungeeSpawn() {}
    void                            BossSpawnAttack() {}
    void                            BossBungeeAttack() {}
    void                            BossRVAttack() {}
    void                            BossSpawnContact() {}
    void                            BossBungeeLeave() {}
    void                            BossStompAttack() {}
    bool                            BossCanStompRow(int theRow) { (void)theRow; return false; }
    void                            BossDie() {}
    void                            BossHeadAttack() {}
    void                            BossHeadSpitContact() {}
    void                            BossHeadSpit() {}
    void                            UpdateBossFireball() {}
    void                            BossDestroyFireball() {}
    void                            BossDestroyIceballInRow() {}
    void                            DiggerLoseAxe() {}
    void                            BungeeDropZombie(Zombie* theDroppedZombie, int theGridX, int theGridY) { (void)theDroppedZombie; (void)theGridX; (void)theGridY; }
    void                            ShowYuckyFace(bool theShow) { (void)theShow; }
    void                            AnimateChewSound() {}
    void                            AnimateChewEffect() {}
    void                            UpdateActions() {}
    void                            CheckForBoardEdge() {}
    void                            UpdateYeti() {}
    void                            DrawBossPart(Graphics* g, BossPart theBossPart) { (void)g; (void)theBossPart; }
    void                            BossSetupReanim() {}
    void                            MowDown() {}
    void                            UpdateMowered() {}
    void                            DropFlag() {}
    void                            DropPole() {}
    void                            DrawBossBackArm(Graphics* g, const ZombieDrawPosition& theDrawPos) { (void)g; (void)theDrawPos; }
    static void                     PreloadZombieResources(ZombieType theZombieType) { (void)theZombieType; }
    void                            BossStartDeath() {}
    void                            RemoveColdEffects() {}
    void                            BossHeadSpitEffect() {}
    void                            DrawBossFireBall(Graphics* g) { (void)g; }
    void                            UpdateZombiePeaHead() {}
    void                            UpdateZombieJalapenoHead() {}
    void                            ApplyBossSmokeParticles(bool theEnable) { (void)theEnable; }
    void                            UpdateZombiquarium() {}
    bool                            ZombiquariumFindClosestBrain() { return false; }
    void                            UpdateZombieGatlingHead() {}
    void                            UpdateZombieSquashHead() {}
    bool                            IsTanglekelpTarget() { return false; }
    bool                            HasYuckyFaceImage() { return false; }
    bool                            IsTangleKelpTarget() { return false; }
    bool                            IsFireResistant() { return false; }
    /*inline*/ void                 EnableMustache(bool theEnableMustache) { (void)theEnableMustache; }
    /*inline*/ void                 EnableFuture(bool theEnableFuture) { (void)theEnableFuture; }
    /*inline*/ void                 EnableDance() {}
    void                            BungeeDropPlant() {}
    void                            RemoveButter() {}
    void                            BalloonPropellerHatSpin(bool theSpinning) { (void)theSpinning; }
    void                            DoDaisies() {}
    static inline bool              ZombieTypeCanGoOnHighGround(ZombieType t) { (void)t; return false; }
    static inline bool              ZombieTypeCanGoInPool(ZombieType t) { (void)t; return false; }
    void                            SetupWaterTrack(const char* theTrackName) { (void)theTrackName; }
    void                            BurnRow(int theRow) { (void)theRow; }
    void                            SetupReanimForLostHead() {}
    void                            SetupReanimForLostArm(unsigned int theDamageFlags) { (void)theDamageFlags; }
    bool                            IsSquashTarget(Plant* theExcept) { (void)theExcept; return false; }
    static /*inline*/ bool			IsZombotany(ZombieType theZombieType) { (void)theZombieType; return false; }
};

class ZombieDefinition
{
public:
    ZombieType                      mZombieType;
    ReanimationType                 mReanimationType;
    int                             mZombieValue;
    int                             mStartingLevel;
    int                             mFirstAllowedWave;
    int                             mPickWeight;
    const char*                     mZombieName;
    int                             mBodyHealth;
};
extern ZombieDefinition gZombieDefs[NUM_ZOMBIE_TYPES];

    inline ZombieDefinition&            GetZombieDefinition(ZombieType theZombieType) { (void)theZombieType; static ZombieDefinition d; d.mZombieType = ZOMBIE_NORMAL; d.mReanimationType = ReanimationType(0); d.mZombieValue = 0; d.mStartingLevel = 0; d.mFirstAllowedWave = 0; d.mPickWeight = 0; d.mZombieName = ""; d.mBodyHealth = 0; return d; }

#endif
