#ifndef __LAWNAPP_H__
#define __LAWNAPP_H__

#include <e32base.h>
#include <string>
#include <map>
#include <list>
#include "engine/Common.h"
#include "engine/Color.h"
#include "engine/Rect.h"
#include "engine/Graphics.h"
#include "engine/WidgetManager.h"
#include "engine/SexyAppBase.h"

#include "ConstEnums.h"

// Forward declarations
class Board;
namespace Sexy { class Dialog; }
namespace Sexy { class Checkbox; }
namespace Sexy { class TitleScreen; }
namespace Sexy { class ChallengeScreen; }
namespace Sexy { class SeedChooserScreen; }
namespace Sexy { class AwardScreen; }
namespace Sexy { class CreditScreen; }
namespace Sexy { class GameSelector; }
class StoreScreen;
class CursorObject;
class CursorPreview;
class GameButton;
class MessageWidget;
class ToolTipWidget;
class Reanimation;
class TodParticleSystem;
class EffectSystem;
class ZenGarden;
class TodFoley;
class Music;
class PoolEffect;
class ProfileMgr;
class PlayerInfo;
class ReanimatorCache;
class TypingCheck;
class DataSync;
class NewOptionsDialog;
class NewUserDialog;
class ContinueDialog;
class UserDialog;
class CheatDialog;
class AlmanacDialog;
class ResourceManager;
class LawnDialog;
struct PottedPlant;
struct ChallengeDefinition;

struct LevelStats
{
    TInt mScore;
    TInt mSunUsed;
    TInt mPlantsUsed;
    TInt mZombiesKilled;
    TInt mUnusedLawnMowers;

    LevelStats() : mScore(0), mSunUsed(0), mPlantsUsed(0), mZombiesKilled(0), mUnusedLawnMowers(0) {}
    void Reset() { mScore = 0; mSunUsed = 0; mPlantsUsed = 0; mZombiesKilled = 0; mUnusedLawnMowers = 0; }
};

// Global application pointer
namespace Sexy { class LawnApp; }
extern Sexy::LawnApp* gLawnApp;

// Free helper functions
bool LawnGetCloseRequest();
bool LawnHasUsedCheatKeys();

// Determine if we can show the Pinata / Dance / Daisy modes
bool CanDoPinataMode();
bool CanDoDanceMode();
bool CanDoDaisyMode();

namespace Sexy {
class LawnApp : public SexyAppBase
{
public:
    LawnApp();
    ~LawnApp();

    void Init();
    void UpdateFrames();
    void Shutdown();
    void Start();
    void KillBoard();

    // Focus
    void GotFocus();
    void LostFocus();

    // Registry
    void WriteToRegistry();
    void ReadFromRegistry();
    bool WriteCurrentUserConfig();

    // Game flow
    void PreNewGame(GameMode theGameMode, bool theLookForSavedGame);
    void MakeNewBoard();
    void StartPlaying();
    bool SaveFileExists();
    bool FileExists(const std::string& theFile) { (void)theFile; return false; }
    bool TryLoadGame();
    void NewGame();
    void EndLevel();
    void CheckForGameEnd();
    bool CanPauseNow();
    void BoardSetUpdate() {}
    bool DebugKeyDown(int key);
    void CloseRequestAsync();
    bool UpdateAppStep(bool*);
    bool UpdateApp();

    // Screens
    void ShowGameSelector();
    void KillGameSelector();
    void ShowAwardScreen(AwardType theAwardType, bool theShowAchievements);
    void KillAwardScreen();
    void ShowCreditScreen();
    void KillCreditScreen();
    void ShowChallengeScreen(ChallengePage thePage);
    void KillChallengeScreen();
    StoreScreen* ShowStoreScreen();
    void KillStoreScreen();
    void ShowSeedChooserScreen();
    void KillSeedChooserScreen();

    // Navigation dialogs
    void DoBackToMain();
    void DoConfirmBackToMain();
    void DoNewOptions(bool theFromGameSelector);
    AlmanacDialog* DoAlmanacDialog(SeedType theSeedType, ZombieType theZombieType);
    void DoContinueDialog();
    void DoPauseDialog();
    int  LawnMessageBox(int theDialogId, const char* theHeaderName, const char* theLinesName, const char* theButton1Name, const char* theButton2Name, int theButtonMode);
    void DoUserDialog();
    void FinishUserDialog(bool isYes);
    void DoCreateUserDialog();
    void FinishCreateUserDialog(bool isYes);
    void DoConfirmDeleteUserDialog(const std::string& theName);
    void FinishConfirmDeleteUserDialog(bool isYes);
    void DoRenameUserDialog(const std::string& theName);
    void FinishRenameUserDialog(bool isYes);
    void FinishNameError(int theId);
    void FinishRestartConfirmDialog();
    void DoCheatDialog();
    void FinishCheatDialog(bool isYes);
    void FinishTimesUpDialog();
    void DoConfirmSellDialog(const std::string& theMessage);
    void DoConfirmPurchaseDialog(const std::string& theMessage);
    bool KillNewOptionsDialog();
    bool KillAlmanacDialog();
    bool NeedPauseGame();
    void ModalOpen();
    void ModalClose();
    bool KillDialog(int theDialogId);
    void KillAllDialogs();
    int  DialogCount();
    void ShowResourceError(bool doExit = false);

    // Dialog helpers
    Sexy::Dialog* DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
    Sexy::Dialog* DoDialogDelay(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
    Sexy::Dialog* NewDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
    void CenterDialog(Sexy::Dialog* theDialog, int theWidth, int theHeight);

    // Level type queries
    bool IsAdventureMode();
    bool IsMiniGameMode();
    bool IsPuzzleMode();
    bool IsSurvivalMode();
    bool IsChallengeMode();
    bool IsZenGardenMode();
    bool IsCoOpMode();
    bool IsFirstTimeAdventureMode();
    bool IsWhackAZombieLevel();
    bool IsMiniBossLevel();
    bool IsSquirrelLevel();
    bool IsStormyNightLevel();
    bool IsLittleTroubleLevel();
    bool IsBungeeBlitzLevel();
    bool IsShovelLevel();
    bool IsSlotMachineLevel();
    bool IsIZombieLevel();
    bool IsBeghouledLevel();
    bool IsBossLevel();
    bool IsFinalBossLevel();
    bool IsLastStandLevel();
    bool IsQuickPlayMode();
    bool IsCoopZenGardenMode();
    bool IsTreeOfWisdomLevel();
    bool IsTutorialLevel();
    bool IsWallnutBowlingLevel();
    bool IsScaryPotterLevel();
    bool IsNight();
    bool IsArtChallenge();
    bool IsIceDemo() { return false; }

    // Survival / challenge helpers
    bool IsSurvivalNormal();
    bool IsSurvivalNormal(GameMode mode);
    bool IsSurvivalHard(GameMode mode);
    bool IsSurvivalEndless();
    bool IsContinuousChallenge();
    bool IsChallengeWithoutSeedBank();
    bool IsIceTrap();
    inline bool IsIceTrap(SeedType t) { (void)t; return false; }
    bool IsEndlessScaryPotter();
    bool IsEndlessIZombie();
    bool IsTrialStageLocked();

    // Static helpers
    static bool IsSurvivalEndless(GameMode mode);
    static bool IsEndlessScaryPotter(GameMode mode);
    static bool IsEndlessIZombie(GameMode mode);

    // Crazy Dave
    void CrazyDaveDie();
    void CrazyDaveEnter();
    void CrazyDaveLeave();
    int  CrazyDaveTalkIndex();
    void CrazyDaveTalkIndex(int idx);
    bool AdvanceCrazyDaveText();
    void CrazyDaveDoneHanding();
    void CrazyDaveStopSound();
    void CrazyDaveTalkMessage(const std::string& theMessage);
    void CrazyDaveStopTalking();
    void UpdateCrazyDave();
    void DrawCrazyDave(Sexy::Graphics* g);
    std::string GetCrazyDaveText(int theGuid);
    std::string Pluralize(int theCount, const char* theSingular, const char* thePlural);

    // Game achievements / progression
    bool HasFinishedAdventure();
    bool HasBeatenChallenge(GameMode theGameMode);
    int  GetNumTrophies(ChallengePage thePage);
    int  TrophiesNeedForGoldSunflower();
    bool EarnedGoldTrophy();
    bool UpdatePlayerProfileForFinishingLevel();
    void UpdatePlayTimeStats();
    void FinishZenGardenToturial();
    bool CanSpawnYetis();

    // Store
    bool CanShowStore();
    bool CanShowAlmanac();
    bool CanShowZenGarden();
    bool CanDoPinataMode();
    bool CanDoDanceMode();
    bool CanDoDaisyMode();

    // Seed / level
    int  GetSeedsAvailable();
    bool HasSeedType(SeedType theSeedType);
    SeedType GetAwardSeedForLevel(int theLevel);
    int  GetCurrentChallengeIndex();
    std::string GetStageString(int theLevel);
    std::string GetMoneyString(int theMoney);

    // Loading
    void LoadGroup(const char* theGroup, int theNumResources);
    void LoadingThreadProc();
    void LoadingCompleted();
    void LoadingThreadCompleted();
    void FastLoad(GameMode theGameMode);
    int  GetNumPreloadingTasks();
    void PreloadForUser();
    void InitHook();

    // Cheats
    void ToggleSlowMo();
    void ToggleFastMo();

    // URL
    void ConfirmQuit();
    bool OpenURL(const std::string& theURL, bool shutdown);
    void URLOpenFailed(const std::string& theURL);
    void URLOpenSucceeded(const std::string& theURL);

    // Button events
    void ButtonPress(int theId);
    void ButtonDownTick(int theId);
    void ButtonMouseEnter(int theId);
    void ButtonMouseLeave(int theId);
    void ButtonMouseMove(int theId, int x, int y);
    void ButtonDepress(int theId);

    // Sound
    void PlaySample(intptr_t soundId);
    void PlayFoley(int foleyType);
    void PlayFoleyPitch(int foleyType, float pitch);

    // Misc
    void SwitchScreenMode(bool wantWindowed, bool want3D, bool changeVideoMode);
    ChallengeDefinition& GetCurrentChallengeDef();
    void FinishModelessDialogs();
    void DoRegister();
    void DoRegisterError();
    bool CanDoRegisterDialog();
    void DoNeedRegisterDialog();
    bool NeedRegister();
    void UpdateRegisterInfo();
    void DoHighScoreDialog();
    void ClearUpdateBacklog(bool clearAll = false) { (void)clearAll; }
    void BoardButtonPress() {}
    void DoParseCmdLine() {}

    // Reanimation / particle
    Reanimation* AddReanimation(float x, float y, int renderOrder, int reanimType);
    TodParticleSystem* AddTodParticle(float x, float y, int renderOrder, int effectType);
    int  ParticleGetID(TodParticleSystem* p);
    TodParticleSystem* ParticleGet(int theParticleID);
    TodParticleSystem* ParticleTryToGet(int id);
    Reanimation* ReanimationTryToGet(ReanimationID id);
    Reanimation* ReanimationGet(ReanimationID id);
    ReanimationID ReanimationGetID(Reanimation* theReanimation);
    void RemoveReanimation(ReanimationID id);
    void RemoveParticle(int id);
    void RemoveReanimation(Reanimation* r);
    void RemoveParticle(TodParticleSystem* p);

    // Dialogs
    Sexy::Dialog* GetDialog(int theDialogId) { return NULL; }
    void KillDialog(Sexy::Dialog* dlg);
    int GetDialogCount() { return 0; }
    bool Is3DAccelerated() { return false; }

    // Cursor
    void SetCursor(int cursorType) { (void)cursorType; }

    // Potted plant
    PottedPlant* GetPottedPlantByIndex(int theIndex);

    // Game state
    GameMode mGameMode;
    BoardResult mBoardResult;
    Board*  mBoard;
    TitleScreen* mTitleScreen;
    ChallengeScreen* mChallengeScreen;
    SeedChooserScreen* mSeedChooserScreen;
    AwardScreen* mAwardScreen;
    CreditScreen* mCreditScreen;
    GameSelector* mGameSelector;
    CursorObject* mCursorObject;
    EffectSystem* mEffectSystem;
    class SoundManager* mSoundManager;
    TodFoley* mSoundSystem;
    Music* mMusic;
    ZenGarden* mZenGarden;
    ReanimatorCache* mReanimatorCache;
    PoolEffect* mPoolEffect;

    // Profile
    ProfileMgr* mProfileMgr;
    PlayerInfo* mPlayerInfo;
    LevelStats* mLastLevelStats;

    // Game flags
    TBool mLoadingFailed;
    TBool mShutdown;
    TBool mCloseRequest;
    TBool mDebugKeysEnabled;
    TBool mTodCheatKeys;
    TBool mFirstTimeGameSelector;
    TBool mSawYeti;
    TBool mMustacheMode;
    TBool mSuperMowerMode;
    TBool mFutureMode;
    TBool mPinataMode;
    TBool mDanceMode;
    TBool mDaisyMode;
    TBool mSukhbirMode;

    TInt mAppCounter;
    TInt mMusicVolume;
    TInt mSfxVolume;
    TInt mPlayTimeActiveSession;

    // Cmd-line cheats
    TypingCheck* mKonamiCheck;
    TypingCheck* mMustacheCheck;
    TypingCheck* mMoustacheCheck;
    TypingCheck* mSuperMowerCheck;
    TypingCheck* mSuperMowerCheck2;
    TypingCheck* mFutureCheck;
    TypingCheck* mPinataCheck;
    TypingCheck* mDanceCheck;
    TypingCheck* mDaisyCheck;
    TypingCheck* mSukhbirCheck;

    int mGameScene;
    int mAppRandSeed;
    int mCrazyDaveState;
    bool mEasyPlantingCheat;

    // Resource manager
    class ResourceManager* mResourceManager;

    // Additional members used by LawnApp.cpp
    int  mFullscreenBits;
    int  mTrialType;
    bool mDebugTrialLocked;
    bool mMuteSoundsForCutscene;
    bool mAutoStartLoadingThread;
    std::string mProdName;
    bool mCustomCursorsEnabled;
    bool mAutoEnable3D;
    int  mGamesPlayed;
    int  mMaxExecutions;
    int  mMaxPlays;
    int  mMaxTime;
    int  mCrazyDaveBlinkCounter;
    int  mCrazyDaveMessageIndex;
    int  mNumLoadingThreadTasks;
    std::string mCrazyDaveMessageText;

    // Crazy Dave reanimation IDs
    int mCrazyDaveReanimID;
    int mCrazyDaveBlinkReanimID;

    // Dialog tracking
    std::list<Sexy::Dialog*> mDialogList;
    std::map<int, Sexy::Dialog*> mDialogMap;

    // Typing check helper
    TypingCheck* mTypingCheck;

    // Loading thread
    TBool mLoadingThreadCompleted;
    TBool mRegisterResourcesLoaded;
    TInt  mCompletedLoadingThreadTasks;

};

}  // namespace Sexy

#endif // __LAWNAPP_H__
