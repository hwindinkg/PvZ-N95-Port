/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Ported to Symbian S60 3rd FP1 (GCCE / C++03).
 * Mechanical transformations applied:
 *   - nullptr -> NULL, auto -> explicit types
 *   - enum class scoping removed (plain enums)
 *   - std::min/std::max -> MIN()/MAX()
 *   - Range-based for -> index-based for
 *   - Modern C++ includes removed
 *   - SDL-specific code removed
 *   - SexyApp::Init / HandleCmdLineParam / RegistryReadString removed
 */

#include <time.h>
#include <e32std.h>
#include <f32file.h>
#include "engine/Stubs.h"
#include "LawnApp.h"
#include "Resources.h"   // for FONT_BRIANNETOD16 / FONT_PICO129 (were #define macros in Stubs.h, now extern in Resources.h)
#include "Sexy.TodLib/Reanimator.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Cutscene.h"
#include "GameConstants.h"
#include "Lawn/Challenge.h"
#include "Lawn/ZenGarden.h"
#include "Sexy.TodLib/Trail.h"
#include "Lawn/System/Music.h"
#include "Lawn/System/SaveGame.h"
#include "Sexy.TodLib/TodDebug.h"
#include "Sexy.TodLib/TodFoley.h"
#include "Sexy.TodLib/Attachment.h"
#include "Lawn/System/PlayerInfo.h"
#include "Lawn/System/PoolEffect.h"
#include "Lawn/System/ProfileMgr.h"
#include "Lawn/Widget/GameButton.h"
#include "Sexy.TodLib/Reanimator.h"
#include "Lawn/Widget/UserDialog.h"
#include "Lawn/System/TypingCheck.h"
#include "Sexy.TodLib/TodParticle.h"
#include "Lawn/Widget/AwardScreen.h"
#include "Lawn/Widget/TitleScreen.h"
#include "Lawn/Widget/StoreScreen.h"
#include "Lawn/Widget/CheatDialog.h"
#include "Lawn/Widget/GameSelector.h"
#include "Lawn/Widget/CreditScreen.h"
#include "Sexy.TodLib/EffectSystem.h"
#include "Sexy.TodLib/FilterEffect.h"
#include "engine/Graphics.h"
#include "Sexy.TodLib/TodStringFile.h"
#include "Lawn/Widget/AlmanacDialog.h"
#include "Lawn/Widget/NewUserDialog.h"
#include "Lawn/Widget/ContinueDialog.h"
#include "Lawn/System/ReanimationLawn.h"
#include "Lawn/Widget/ChallengeScreen.h"
#include "Lawn/Widget/NewOptionsDialog.h"
#include "Lawn/Widget/SeedChooserScreen.h"
#include "engine/WidgetManager.h"
#include "engine/ResourceManager.h"

#include "engine/Checkbox.h"
#include "engine/Dialog.h"

using namespace Sexy;

bool gIsPartnerBuild = false; // GOTY @Patoke: 0x729659
bool gSlowMo = false;
bool gFastMo = false;
LawnApp* gLawnApp = NULL;
int gSlowMoCounter = 0;

static bool HasUnshownAchievements(PlayerInfo* thePlayerInfo)
{
    if (thePlayerInfo == NULL)
    {
        return false;
    }

    for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        if (thePlayerInfo->mEarnedAchievements[i] && !thePlayerInfo->mShownAchievements[i])
        {
            return true;
        }
    }

    return false;
}

bool LawnGetCloseRequest()
{
    if (gLawnApp == NULL)
        return false;

    return gLawnApp->mCloseRequest;
}

bool LawnHasUsedCheatKeys()
{
    return gLawnApp && gLawnApp->mPlayerInfo && gLawnApp->mPlayerInfo->mHasUsedCheatKeys;
}

LawnApp::LawnApp()
{
    mBoard = NULL;
    mGameSelector = NULL;
    mChallengeScreen = NULL;
    mSeedChooserScreen = NULL;
    mAwardScreen = NULL;
    mCreditScreen = NULL;
    mTitleScreen = NULL;
    mSoundSystem = NULL;
    mMusic = NULL;
    mKonamiCheck = NULL;
    mMustacheCheck = NULL;
    mMoustacheCheck = NULL;
    mSuperMowerCheck = NULL;
    mSuperMowerCheck2 = NULL;
    mFutureCheck = NULL;
    mPinataCheck = NULL;
    mDanceCheck = NULL;
    mDaisyCheck = NULL;
    mSukhbirCheck = NULL;
    mMustacheMode = false;
    mSuperMowerMode = false;
    mFutureMode = false;
    mPinataMode = false;
    mDanceMode = false;
    mDaisyMode = false;
    mSukhbirMode = false;
    mGameScene = SCENE_LOADING;
    mPoolEffect = NULL;
    mZenGarden = NULL;
    mEffectSystem = NULL;
    mReanimatorCache = NULL;
    mCloseRequest = false;
    mWidth = BOARD_WIDTH/IMG_DOWNSCALE;
    mHeight = BOARD_HEIGHT/IMG_DOWNSCALE;
    mFullscreenBits = 32;
    mAppCounter = 0;
    mAppRandSeed = time(0);
    mTrialType = TRIALTYPE_NONE;
    mDebugTrialLocked = false;
    mMuteSoundsForCutscene = false;
    mMusicVolume = 0.85;
    mSfxVolume = 0.5525;
    mAutoStartLoadingThread = false;
    mDebugKeysEnabled = false;
    mProdName = "io.github.wszqkzqk.pvz-portable";
    std::string aTitleName = "PvZ Portable";
    mTitle = aTitleName;
    mCustomCursorsEnabled = false;
    mPlayerInfo = NULL;
    mLastLevelStats = new LevelStats();
    mFirstTimeGameSelector = true;
    mGameMode = GAMEMODE_ADVENTURE;
    mEasyPlantingCheat = false;
    mAutoEnable3D = true;
    Tod_SWTri_AddAllDrawTriFuncs();
    mLoadingThreadCompleted = true;
    mGamesPlayed = 0;
    mMaxExecutions = 0;
    mMaxPlays = 0;
    mMaxTime = 0;
    mCompletedLoadingThreadTasks = 0;
    mProfileMgr = new ProfileMgr();
    mRegisterResourcesLoaded = false;
    mTodCheatKeys = false;
    mCrazyDaveReanimID = REANIMATIONID_NULL;
    mCrazyDaveState = CRAZY_DAVE_OFF;
    mCrazyDaveBlinkCounter = 0;
    mCrazyDaveBlinkReanimID = REANIMATIONID_NULL;
    mCrazyDaveMessageIndex = -1;
    //mBigArrowCursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR1));
}

LawnApp::~LawnApp()
{
    while (!mDialogMap.empty())
    {
        KillDialog(mDialogMap.begin()->first);
    }

    if (mBoard)
    {
        mBoardResult = BOARDRESULT_QUIT_APP;
        mBoard->TryToSaveGame();
        WriteCurrentUserConfig();
        KillBoard();
    }
    ProcessSafeDeleteList();

    if (mTitleScreen)
    {
        mWidgetManager->RemoveWidget(mTitleScreen);
        delete mTitleScreen;
    }

    delete mSoundSystem;
    delete mMusic;

    if (mKonamiCheck)
    {
        delete mKonamiCheck;
    }
    if (mMustacheCheck)
    {
        delete mMustacheCheck;
    }
    if (mMoustacheCheck)
    {
        delete mMoustacheCheck;
    }
    if (mSuperMowerCheck)
    {
        delete mSuperMowerCheck;
    }
    if (mSuperMowerCheck2)
    {
        delete mSuperMowerCheck2;
    }
    if (mFutureCheck)
    {
        delete mFutureCheck;
    }
    if (mPinataCheck)
    {
        delete mPinataCheck;
    }
    if (mDanceCheck)
    {
        delete mDanceCheck;
    }
    if (mDaisyCheck)
    {
        delete mDaisyCheck;
    }
    if (mSukhbirCheck)
    {
        delete mSukhbirCheck;
    }

    if (mGameSelector)
    {
        mWidgetManager->RemoveWidget(mGameSelector);
        delete mGameSelector;
    }
    if (mChallengeScreen)
    {
        mWidgetManager->RemoveWidget(mChallengeScreen);
        delete mChallengeScreen;
    }
    if (mSeedChooserScreen)
    {
        mWidgetManager->RemoveWidget(mSeedChooserScreen);
        delete mSeedChooserScreen;
    }
    if (mAwardScreen)
    {
        mWidgetManager->RemoveWidget(mAwardScreen);
        delete mAwardScreen;
    }
    if (mCreditScreen)
    {
        mWidgetManager->RemoveWidget(mCreditScreen);
        delete mCreditScreen;
    }

    if (mPoolEffect)
    {
        mPoolEffect->PoolEffectDispose();
        delete mPoolEffect;
    }

    if (mZenGarden)
    {
        delete mZenGarden;
    }

    if (mEffectSystem)
    {
        mEffectSystem->EffectSystemDispose();
        delete mEffectSystem;
    }

    if (mReanimatorCache)
    {
        mReanimatorCache->ReanimatorCacheDispose();
        delete mReanimatorCache;
    }

    FilterEffectDisposeForApp();
    TodParticleFreeDefinitions();
    ReanimatorFreeDefinitions();
    TrailFreeDefinitions();
    FreeGlobalAllocators();
    UpdateRegisterInfo();

    delete mProfileMgr;
    delete mLastLevelStats;

    mResourceManager->DeleteResources("");
    /*
#ifdef PVZ_DEBUG
    BetaSubmit(true);
#endif
    */
}

void LawnApp::Shutdown()
{
    if (!mLoadingThreadCompleted)
    {
        mLoadingFailed = true;
        SexyAppBase::Shutdown();
        return;
    }

    if (!mShutdown)
    {
        SexyAppBase::Shutdown();
    }
}

// GOTY @Patoke : 0x452640
void LawnApp::KillBoard()
{
    FinishModelessDialogs();
    KillSeedChooserScreen();
    if (mBoard)
    {
        if (mPlayerInfo && (
            mBoardResult == BOARDRESULT_WON ||
            mBoardResult == BOARDRESULT_LOST ||
            mBoardResult == BOARDRESULT_RESTART ||
            mBoardResult == BOARDRESULT_CHEAT))
        {
            std::string aFileName = GetSavedGameName(mGameMode, mPlayerInfo->mId);
            EraseFile(aFileName);
            std::string aLegacyFileName = GetLegacySavedGameName(mGameMode, mPlayerInfo->mId);
            EraseFile(aLegacyFileName);
        }

/*
#ifdef PVZ_DEBUG
        BetaRecordLevelStats();
#endif
*/
        mBoard->DisposeBoard();
        mWidgetManager->RemoveWidget(mBoard);
        SafeDeleteWidget(mBoard);
        mBoard = NULL;
    }

    SetCursor(CURSOR_POINTER);
}

bool LawnApp::CanPauseNow()
{
    if (mBoard == NULL)  // 不在关卡内
        return false;

    if (mSeedChooserScreen && mSeedChooserScreen->mMouseVisible)  // 处于选卡界面
        return false;

    if (mBoard->mBoardFadeOutCounter >= 0)  // 退出关卡过程中
        return false;

    if (mCrazyDaveState != CRAZY_DAVE_OFF)  // 存在戴夫
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN || mGameMode == GAMEMODE_TREE_OF_WISDOM)  // 处于禅境花园或智慧树
        return false;

    return GetDialogCount() <= 0;  // 不存在对话
}

void LawnApp::GotFocus()
{
}

void LawnApp::LostFocus()
{
    if (!mTodCheatKeys && CanPauseNow())
    {
        DoPauseDialog();
    }
}

void LawnApp::WriteToRegistry()
{
    if (mPlayerInfo)
    {
        RegistryWriteString("CurUser", mPlayerInfo->mName);
        mPlayerInfo->SaveDetails();
    }

    SexyAppBase::WriteToRegistry();
}

void LawnApp::ReadFromRegistry()
{
}

// GOTY @Patoke: 0x452800
bool LawnApp::WriteCurrentUserConfig()
{
    if (mPlayerInfo)
        mPlayerInfo->SaveDetails();

    return true;
}

// GOTY @Patoke: 0x452820
void LawnApp::PreNewGame(GameMode theGameMode, bool theLookForSavedGame)
{
    //if (NeedRegister())
    //{
    //  ShowGameSelector();
    //  return;
    //}

    mGameMode = theGameMode;
    if (theLookForSavedGame && TryLoadGame())
        return;

    // [M4 #1 fix] mPlayerInfo is NULL in this port -- ProfileMgr is a no-op
    // stub (GetAnyProfile returns NULL). Accessing mPlayerInfo->mId crashes
    // with KERN-EXEC 3. Use a default id of 0 when no profile is loaded.
    // Once ProfileMgr is properly ported, this guard can be removed.
    int playerId = (mPlayerInfo != NULL) ? mPlayerInfo->mId : 0;
    std::string aFileName = GetSavedGameName(mGameMode, playerId);
    EraseFile(aFileName);
    std::string aLegacyFileName = GetLegacySavedGameName(mGameMode, playerId);
    EraseFile(aLegacyFileName);
    NewGame();
}

// GOTY @Patoke: 0x4528B0
void LawnApp::MakeNewBoard()
{
    KillBoard();
    mBoard = new Board(this);
    mBoard->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mBoard);
    mWidgetManager->BringToBack(mBoard);
    mWidgetManager->SetFocus(mBoard);
}

// GOTY @Patoke: 0x452970
void LawnApp::StartPlaying()
{
    KillSeedChooserScreen();
    mBoard->StartLevel();
    mGameScene = SCENE_PLAYING;
}

bool LawnApp::SaveFileExists()
{
    std::string aFileName = GetSavedGameName(GAMEMODE_ADVENTURE, mPlayerInfo->mId);
    if (this->FileExists(aFileName))
        return true;
    std::string aLegacyFileName = GetLegacySavedGameName(GAMEMODE_ADVENTURE, mPlayerInfo->mId);
    return this->FileExists(aLegacyFileName);
}

// GOTY @Patoke: 0x452A50
bool LawnApp::TryLoadGame()
{
    std::string aSaveName = GetSavedGameName(mGameMode, mPlayerInfo->mId);
    std::string aLegacySaveName = GetLegacySavedGameName(mGameMode, mPlayerInfo->mId);
    mMusic->StopAllMusic();

    if (this->FileExists(aSaveName))
    {
        MakeNewBoard();
        if (mBoard->LoadGame(aSaveName))
        {
            mFirstTimeGameSelector = false;
            if (mBoard->mLevelAwardSpawned) // Ensure save cleanup after award collection
                mBoardResult = BOARDRESULT_WON;
            DoContinueDialog();
            return true;
        }

        KillBoard();
    }
    if (this->FileExists(aLegacySaveName))
    {
        MakeNewBoard();
        if (mBoard->LoadGame(aLegacySaveName))
        {
            if (LawnSaveGame(mBoard, aSaveName))
            {
                EraseFile(aLegacySaveName);
            }
            mFirstTimeGameSelector = false;
            if (mBoard->mLevelAwardSpawned) // Ensure save cleanup after award collection
                mBoardResult = BOARDRESULT_WON;
            DoContinueDialog();
            return true;
        }

        KillBoard();
    }

    return false;
}

// GOTY @Patoke: 0x452B30
void LawnApp::NewGame()
{
    mFirstTimeGameSelector = false;

    MakeNewBoard();
    mBoard->InitLevel();
    mBoardResult = BOARDRESULT_NONE;
    mGameScene = SCENE_LEVEL_INTRO;

    ShowSeedChooserScreen();
    mBoard->mCutScene->StartLevelIntro();
}

// GOTY @Patoke: 0x452B80
void LawnApp::ShowGameSelector()
{
    KillBoard();
    //UpdateRegisterInfo();
    if (mGameSelector)
    {
        mWidgetManager->RemoveWidget(mGameSelector);
        SafeDeleteWidget(mGameSelector);
    }

    mGameScene = SCENE_MENU;
    mGameSelector = new GameSelector(this);
    mGameSelector->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mGameSelector);
    // [M4 #1 fix] GameSelector::Draw paints IMAGE_TITLESCREEN as the background.
    // The 10 GameButton children are top-level widgets in the manager (NOT
    // children of GameSelector -- port's WidgetContainer::AddWidget doesn't
    // call AddedToManager, so we add them directly). For the buttons to be
    // visible ON TOP of the background, GameSelector must be drawn FIRST
    // (index 0 = bottom layer). The previous BringToFront placed it LAST
    // (top layer), so its titlescreen background covered all the buttons --
    // the user saw "just the loading screen" because that's literally what
    // GameSelector was drawing over the buttons.
    //
    // In M3 BringToFront was needed because a stale loader/TitleScreen overlay
    // was still in the widget list and had to be covered. That overlay is now
    // properly removed in LoadingCompleted(), so BringToBack is safe.
    mWidgetManager->BringToBack(mGameSelector);
    mWidgetManager->SetFocus(mGameSelector);

    // (widget-list diagnostic dump is in LoadingCompleted, writing to
    // wgt_dump.txt -- shows the state BEFORE ShowGameSelector adds new widgets.)

    //if (NeedRegister())
    //{
    //  DoNeedRegisterDialog();
    //}
}

// GOTY @Patoke: 0x452C70
void LawnApp::KillGameSelector()
{
    if (mGameSelector)
    {
        mWidgetManager->RemoveWidget(mGameSelector);
        SafeDeleteWidget(mGameSelector);
        mGameSelector = NULL;
    }
}

// GOTY @Patoke: 0x452CB0
void LawnApp::ShowAwardScreen(AwardType theAwardType, bool theShowAchievements)
{
    mGameScene = SCENE_AWARD;
    mAwardScreen = new AwardScreen(this, theAwardType, theShowAchievements);
    mAwardScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mAwardScreen);
    mWidgetManager->BringToBack(mAwardScreen);
    mWidgetManager->SetFocus(mAwardScreen);
}

// GOTY @Patoke: 0x452D80
void LawnApp::KillAwardScreen()
{
    if (mAwardScreen)
    {
        mWidgetManager->RemoveWidget(mAwardScreen);
        SafeDeleteWidget(mAwardScreen);
        mAwardScreen = NULL;
    }
}

// GOTY @Patoke: 0x452DC0
void LawnApp::ShowCreditScreen()
{
    mCreditScreen = new CreditScreen(this);
    mCreditScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mCreditScreen);
    mWidgetManager->BringToBack(mCreditScreen);
    mWidgetManager->SetFocus(mCreditScreen);
}

void LawnApp::KillCreditScreen()
{
    if (mCreditScreen)
    {
        mWidgetManager->RemoveWidget(mCreditScreen);
        SafeDeleteWidget(mCreditScreen);
        mCreditScreen = NULL;
    }
}

// GOTY @Patoke: 0x452EC0
void LawnApp::ShowChallengeScreen(ChallengePage thePage)
{
    mGameScene = SCENE_CHALLENGE;
    mChallengeScreen = new ChallengeScreen(this, thePage);
    mChallengeScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mChallengeScreen);
    mWidgetManager->BringToBack(mChallengeScreen);
    mWidgetManager->SetFocus(mChallengeScreen);
}

void LawnApp::KillChallengeScreen()
{
    if (mChallengeScreen)
    {
        mWidgetManager->RemoveWidget(mChallengeScreen);
        SafeDeleteWidget(mChallengeScreen);
        mChallengeScreen = NULL;
    }
}

// GOTY @Patoke: 0x452FD0
StoreScreen* LawnApp::ShowStoreScreen()
{
    //FinishModelessDialogs();
    TOD_ASSERT(!GetDialog(DIALOG_STORE));

    StoreScreen* aStoreScreen = new StoreScreen(this);
    AddDialog(aStoreScreen);
    mWidgetManager->SetFocus(aStoreScreen);

    return aStoreScreen;
}

void LawnApp::KillStoreScreen()
{
    if (GetDialog(DIALOG_STORE))
    {
        KillDialog(DIALOG_STORE);
        ClearUpdateBacklog(false);
    }
}

// GOTY @Patoke: 0x453050
void LawnApp::ShowSeedChooserScreen()
{
    TOD_ASSERT(mSeedChooserScreen == NULL);

    mSeedChooserScreen = new SeedChooserScreen();
    mSeedChooserScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mSeedChooserScreen);
    mWidgetManager->BringToBack(mSeedChooserScreen);
}

void LawnApp::KillSeedChooserScreen()
{
    if (mSeedChooserScreen)
    {
        mWidgetManager->RemoveWidget(mSeedChooserScreen);
        SafeDeleteWidget(mSeedChooserScreen);
        mSeedChooserScreen = NULL;
    }
}

void LawnApp::EndLevel()
{
    KillBoard();
    if (IsAdventureMode())
    {
        NewGame();
    }

    mFirstTimeGameSelector = true;

    MakeNewBoard();
    mBoard->InitLevel();
    mBoardResult = BOARDRESULT_NONE;
    mGameScene = SCENE_LEVEL_INTRO;
    ShowSeedChooserScreen();
    mBoard->mCutScene->StartLevelIntro();
}

void LawnApp::DoBackToMain()
{
    mMusic->StopAllMusic();
    mSoundSystem->CancelPausedFoley();
    WriteCurrentUserConfig();
    KillNewOptionsDialog();
    KillBoard();
    ShowGameSelector();
}

void LawnApp::DoConfirmBackToMain()
{
    LawnDialog* aDialog = (LawnDialog*)DoDialog(
        DIALOG_CONFIRM_BACK_TO_MAIN,
        true,
        "Leave Game?"/*"[LEAVE_GAME]"*/,
        "Do you want to return\nto the main menu?\n\nYour game will be saved."/*"[LEAVE_GAME_HEADER]"*/,
        "",
        Dialog::BUTTONS_YES_NO
    );

    aDialog->mLawnYesButton->mLabel = TodStringTranslate("[LEAVE_BUTTON]");
    aDialog->mLawnNoButton->mLabel = TodStringTranslate("[DIALOG_BUTTON_CANCEL]");
    //aDialog->CalcSize(0, 0);
}

// GOTY @Patoke: 0x453360
void LawnApp::DoNewOptions(bool theFromGameSelector)
{
    //FinishModelessDialogs();

    NewOptionsDialog* aDialog = new NewOptionsDialog(this, theFromGameSelector);
    // [M4 #1 fix] IMAGE_OPTIONS_MENUBACK may be NULL (one of the 51 NULL
    // symbols per README M4 #5). Dereferencing NULL->mWidth crashes with
    // KERN-EXEC 3. Use 0 (which CenterDialog will handle as "use dialog's
    // own size") when the image isn't loaded.
    int dlgW = IMAGE_OPTIONS_MENUBACK ? IMAGE_OPTIONS_MENUBACK->mWidth : 0;
    int dlgH = IMAGE_OPTIONS_MENUBACK ? IMAGE_OPTIONS_MENUBACK->mHeight : 0;
    CenterDialog(aDialog, dlgW, dlgH);
    AddDialog(DIALOG_NEWOPTIONS, aDialog);
    mWidgetManager->SetFocus(aDialog);
}

// GOTY @Patoke: 0x453410
AlmanacDialog* LawnApp::DoAlmanacDialog(SeedType theSeedType, ZombieType theZombieType)
{
    PerfTimer mTimer;
    mTimer.Start();

    //FinishModelessDialogs();

    AlmanacDialog* aDialog = new AlmanacDialog(this);
    AddDialog(DIALOG_ALMANAC, aDialog);
    mWidgetManager->SetFocus(aDialog);

    if (theSeedType != SEED_NONE)
    {
        aDialog->ShowPlant(theSeedType);
    }
    else if (theZombieType != ZOMBIE_INVALID)
    {
        aDialog->ShowZombie(theZombieType);
    }

    int aDuration = mTimer.GetDuration();
    TodTrace("almanac load time: %d ms", aDuration);

    return aDialog;
}

// GOTY @Patoke: 0x453590
void LawnApp::DoContinueDialog()
{
    ContinueDialog* aDialog = new ContinueDialog(this);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    AddDialog(DIALOG_CONTINUE, aDialog);
}

void LawnApp::DoPauseDialog()
{
    mBoard->Pause(true);
    //FinishModelessDialogs();

    LawnDialog* aDialog = (LawnDialog*)DoDialog(
        DIALOG_PAUSED,
        true,
        "GAME PAUSED"/*"[RESUME_GAME]"*/,
        "Click to resume game",
        "Resume Game"/*"[GAME_PAUSED]"*/,
        Dialog::BUTTONS_FOOTER
    );

    aDialog->mReanimation->AddReanimation(72.0f, 42.0f, REANIM_ZOMBIE_NEWSPAPER);
    aDialog->mSpaceAfterHeader = 155;
    aDialog->CalcSize(0, 10);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
}

// GOTY @Patoke: 0x4538A0
int LawnApp::LawnMessageBox(int theDialogId, const char* theHeaderName, const char* theLinesName, const char* theButton1Name, const char* theButton2Name, int theButtonMode)
{
    Widget* aOldFocus = mWidgetManager->mFocusWidget;

    LawnDialog* aDialog = (LawnDialog*)DoDialog(theDialogId, true, theHeaderName, theLinesName, theButton1Name, theButtonMode);
    if (aDialog->mLawnYesButton)
    {
        aDialog->mLawnYesButton->mLabel = TodStringTranslate(theButton1Name);
    }
    if (aDialog->mLawnNoButton)
    {
        aDialog->mLawnNoButton->mLabel = TodStringTranslate(theButton2Name);
    }
    //aDialog->CalcSize(0, 0);

    mWidgetManager->SetFocus(aDialog);
    int aResult = aDialog->WaitForResult(true);
    mWidgetManager->SetFocus(aOldFocus);

    return aResult;
}

Dialog* LawnApp::DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{
    std::string aHeader = TodStringTranslate(theDialogHeader);
    std::string aLines = TodStringTranslate(theDialogLines);
    std::string aFooter = TodStringTranslate(theDialogFooter);

    Dialog* aDialog = SexyAppBase::DoDialog(theDialogId, isModal, aHeader, aLines, aFooter, theButtonMode);
    if (mWidgetManager->mFocusWidget == NULL)
    {
        mWidgetManager->mFocusWidget = aDialog;
    }

    return aDialog;
}

Dialog* LawnApp::DoDialogDelay(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{
    LawnDialog* aDialog = (LawnDialog*)SexyAppBase::DoDialog(theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);
    aDialog->SetButtonDelay(30);
    return aDialog;
}

// GOTY @Patoke: 0x453C60
void LawnApp::DoUserDialog()
{
    KillDialog(DIALOG_USERDIALOG);

    UserDialog* aDialog = new UserDialog(this);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    AddDialog(DIALOG_USERDIALOG, aDialog);
    mWidgetManager->SetFocus(aDialog);
}

void LawnApp::FinishUserDialog(bool isYes)
{
    UserDialog* aUserDialog = (UserDialog*)GetDialog(DIALOG_USERDIALOG);
    if (aUserDialog)
    {
        if (isYes)
        {
            PlayerInfo* aProfile = mProfileMgr->GetProfile(aUserDialog->GetSelName());
            if (aProfile)
            {
                mPlayerInfo = aProfile;
                mWidgetManager->MarkAllDirty();

                if (mGameSelector)
                {
                    mGameSelector->SyncProfile(true);
                }
            }
        }

        KillDialog(DIALOG_USERDIALOG);
    }
}

// GOTY @Patoke: 0x453DE0
void LawnApp::DoCreateUserDialog()
{
    KillDialog(DIALOG_CREATEUSER);

    NewUserDialog* aDialog = new NewUserDialog(this, false);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    AddDialog(DIALOG_CREATEUSER, aDialog);
}

void LawnApp::FinishCreateUserDialog(bool isYes)
{
    NewUserDialog* aNewUserDialog = (NewUserDialog*)GetDialog(DIALOG_CREATEUSER);
    if (aNewUserDialog == NULL)
        return;

    std::string aName = aNewUserDialog->GetName();

    if (isYes && aName.empty())
    {
        DoDialog(
            DIALOG_CREATEUSERERROR,
            true,
            "Enter Your Name",
            "Please enter your name to create a new user profile for storing high score data and game progress",
            "[DIALOG_BUTTON_OK]",
            Dialog::BUTTONS_FOOTER
        );
    }
    else if (mPlayerInfo == NULL && (!isYes || aName.empty()))
    {
        DoDialog(
            DIALOG_CREATEUSERERROR,
            true,
            "Enter Your Name"/*"[ENTER_YOUR_NAME]"*/,
            "Please enter your name to create a new user profile for storing high score data and game progress"/*"[ENTER_NEW_USER]"*/,
            "[DIALOG_BUTTON_OK]",
            Dialog::BUTTONS_FOOTER
        );
    }
    else if (!isYes)
    {
        KillDialog(DIALOG_CREATEUSER);
    }
    else
    {
        PlayerInfo* aProfile = mProfileMgr->AddProfile(aName);
        if (aProfile == NULL)
        {
            DoDialog(
                DIALOG_CREATEUSERERROR,
                true,
                "Name Conflict"/*"[NAME_CONFLICT]"*/,
                "The name you entered is already being used.  Please enter a unique player name"/*"[ENTER_UNIQUE_PLAYER_NAME]"*/,
                "[DIALOG_BUTTON_OK]",
                Dialog::BUTTONS_FOOTER
            );
        }
        else
        {
            mProfileMgr->Save();
            mPlayerInfo = aProfile;

            KillDialog(DIALOG_USERDIALOG);
            KillDialog(DIALOG_CREATEUSER);
            mWidgetManager->MarkAllDirty();

            if (mGameSelector)
            {
                mGameSelector->SyncProfile(true);
            }
        }
    }
}

// GOTY @Patoke: 0x4541F0
void LawnApp::DoConfirmDeleteUserDialog(const std::string& theName)
{
    KillDialog(DIALOG_CONFIRMDELETEUSER);
    DoDialog(
        DIALOG_CONFIRMDELETEUSER,
        true,
        "Are You Sure"/*"[ARE_YOU_SURE]"*/,
        // StrFormat(TodStringTranslate("[DELETE_USER_WARNING]").c_str(), theName)
        // @Patoke: didn't access this as 'const char*'
        StrFormat("This will permanently remove '%s' from the player roster!"/**/, theName.c_str()),
        "",
        Dialog::BUTTONS_YES_NO
    );
}

void LawnApp::FinishConfirmDeleteUserDialog(bool isYes)
{
    KillDialog(DIALOG_CONFIRMDELETEUSER);
    UserDialog* aUserDialog = (UserDialog*)GetDialog(DIALOG_USERDIALOG);
    if (aUserDialog == NULL)
        return;

    mWidgetManager->SetFocus(aUserDialog);

    if (!isYes)
        return;

    std::string aCurName = mPlayerInfo ? mPlayerInfo->mName : "";
    std::string aName = aUserDialog->GetSelName();
    if (aName == aCurName)
    {
        mPlayerInfo = NULL;
    }

    mProfileMgr->DeleteProfile(aName);
    aUserDialog->FinishDeleteUser();
    if (mPlayerInfo == NULL)
    {
        mPlayerInfo = mProfileMgr->GetProfile(aUserDialog->GetSelName());
        if (mPlayerInfo == NULL)
        {
            mPlayerInfo = mProfileMgr->GetAnyProfile();
        }
    }

    mProfileMgr->Save();
    if (mPlayerInfo == NULL)
    {
        DoCreateUserDialog();
    }

    mWidgetManager->MarkAllDirty();
    if (mGameSelector != NULL)
    {
        mGameSelector->SyncProfile(true);
    }
}

// GOTY @Patoke: 0x454560
void LawnApp::DoRenameUserDialog(const std::string& theName)
{
    KillDialog(DIALOG_RENAMEUSER);

    NewUserDialog* aDialog = new NewUserDialog(this, true);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    aDialog->SetName(theName);
    AddDialog(DIALOG_RENAMEUSER, aDialog);
}

void LawnApp::FinishRenameUserDialog(bool isYes)
{
    UserDialog* aUserDialog = (UserDialog*)GetDialog(DIALOG_USERDIALOG);
    if (!isYes)
    {
        KillDialog(DIALOG_RENAMEUSER);
        mWidgetManager->SetFocus(aUserDialog);
        return;
    }

    NewUserDialog* aNewUserDialog = (NewUserDialog*)GetDialog(DIALOG_RENAMEUSER);
    if (aUserDialog == NULL || aNewUserDialog == NULL)
        return;

    std::string anOldName = aUserDialog->GetSelName();
    std::string aNewName = aNewUserDialog->GetName();
    if (aNewName.empty())
        return;

    bool isCurrentUser = mProfileMgr->GetProfile(anOldName) == mPlayerInfo;
    if (!mProfileMgr->RenameProfile(anOldName, aNewName))
    {
        DoDialog(
            DIALOG_RENAMEUSERERROR,
            true,
            "Name Conflict"/*"[NAME_CONFLICT]"*/,
            "The name you entered is already being used.  Please enter a unique player name"/*"[ENTER_UNIQUE_PLAYER_NAME]"*/,
            "[DIALOG_BUTTON_OK]",
            Dialog::BUTTONS_FOOTER
        );
        return;
    }

    mProfileMgr->Save();
    if (isCurrentUser)
    {
        mPlayerInfo = mProfileMgr->GetProfile(aNewName);
    }

    aUserDialog->FinishRenameUser(aNewName);
    mWidgetManager->MarkAllDirty();
    KillDialog(DIALOG_RENAMEUSER);
    mWidgetManager->SetFocus(aUserDialog);
}

void LawnApp::FinishNameError(int theId)
{
    KillDialog(theId);

    NewUserDialog* aNewUserDialog = (NewUserDialog*)GetDialog(theId == DIALOG_CREATEUSERERROR ? DIALOG_CREATEUSER : DIALOG_RENAMEUSER);
    if (aNewUserDialog)
    {
        mWidgetManager->SetFocus(aNewUserDialog->mNameEditWidget);
    }
}

void LawnApp::FinishRestartConfirmDialog()
{
    mSawYeti = mBoard->mKilledYeti;

    KillDialog(DIALOG_CONTINUE);
    KillDialog(DIALOG_RESTARTCONFIRM);
    KillBoard();

    PreNewGame(mGameMode, false);
}

void LawnApp::DoCheatDialog()
{
    KillDialog(DIALOG_CHEAT);

    CheatDialog* aDialog = new CheatDialog(this);
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    AddDialog(DIALOG_CHEAT, aDialog);
}

void LawnApp::FinishCheatDialog(bool isYes)
{
    CheatDialog* aCheatDialog = (CheatDialog*)GetDialog(DIALOG_CHEAT);
    if (aCheatDialog == NULL)
        return;

    if (isYes && !aCheatDialog->ApplyCheat())
        return;

    KillDialog(DIALOG_CHEAT);
    if (isYes)
    {
        mMusic->StopAllMusic();
        mBoardResult = BOARDRESULT_CHEAT;
        PreNewGame(mGameMode, false);
    }
}

void LawnApp::FinishTimesUpDialog()
{
    KillDialog(DIALOG_TIMESUP);
}

// GOTY @Patoke: 0x5282E0
void LawnApp::DoConfirmSellDialog(const std::string& theMessage)
{
    Dialog* aConfirmDialog = DoDialog(DIALOG_ZEN_SELL, true, "[ZEN_SELL_HEADER]", theMessage, "", Dialog::BUTTONS_YES_NO);
    aConfirmDialog->mYesButton->mLabel = TodStringTranslate("[DIALOG_BUTTON_YES]");
    aConfirmDialog->mNoButton->mLabel = TodStringTranslate("[DIALOG_BUTTON_NO]");
}

void LawnApp::DoConfirmPurchaseDialog(const std::string& theMessage)
{
    LawnDialog* aComfirmDialog = (LawnDialog*)DoDialog(DIALOG_STORE_PURCHASE, true, "买下这个物品？", theMessage, "", Dialog::BUTTONS_YES_NO);
    aComfirmDialog->mLawnYesButton->mLabel = TodStringTranslate("[DIALOG_BUTTON_YES]");
    aComfirmDialog->mLawnNoButton->mLabel = TodStringTranslate("[DIALOG_BUTTON_NO]");
}

Dialog* LawnApp::NewDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{
    LawnDialog* aDialog = new LawnDialog(
        this,
        theDialogId,
        isModal,
        theDialogHeader,
        theDialogLines,
        theDialogFooter,
        theButtonMode
    );

    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
    return aDialog;
}

bool LawnApp::KillNewOptionsDialog()
{
    NewOptionsDialog* aNewOptionsDialog = (NewOptionsDialog*)GetDialog(DIALOG_NEWOPTIONS);
    if (aNewOptionsDialog == NULL)
        return false;

    bool wantWindowed = !aNewOptionsDialog->mFullscreenCheckbox->IsChecked();
    bool want3D = aNewOptionsDialog->mHardwareAccelerationCheckbox->IsChecked();
    SwitchScreenMode(wantWindowed, want3D, false);

    KillDialog(DIALOG_NEWOPTIONS);
    ClearUpdateBacklog();
    return true;
}

bool LawnApp::KillAlmanacDialog()
{
    if (GetDialog(DIALOG_ALMANAC))
    {
        KillDialog(DIALOG_ALMANAC);
        ClearUpdateBacklog(false);
        return true;
    }

    return false;
}

bool LawnApp::NeedPauseGame()
{
    if (mDialogList.size() == 0)
        return false;

    if (mDialogList.size() == 1 && mDialogList.front()->mId != DIALOG_NEW_GAME)
    {
        int anId = mDialogList.front()->mId;
        if (anId == DIALOG_CHOOSER_WARNING || anId == DIALOG_PURCHASE_PACKET_SLOT || anId == DIALOG_IMITATER)
        {
            return false;
        }
    }

    return (mBoard == NULL || mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN) && (mBoard == NULL || mGameMode != GAMEMODE_TREE_OF_WISDOM);
}

void LawnApp::ModalOpen()
{
    if (mBoard && NeedPauseGame())
    {
        mBoard->Pause(true);
    }
}

void LawnApp::ModalClose()
{
    if (mBoard && !NeedPauseGame())
    {
    mBoard->Pause(false);
    }
}

bool LawnApp::KillDialog(int theDialogId)
{
    if (SexyAppBase::KillDialog(theDialogId))
    {
        if (mDialogMap.size() == 0)
        {
            if (mBoard)
            {
                mWidgetManager->SetFocus(mBoard);
            }
            else if (mGameSelector)
            {
                mWidgetManager->SetFocus(mGameSelector);
            }
        }

        if (mBoard && !NeedPauseGame())
        {
            mBoard->Pause(false);
        }

        return true;
    }

    return false;
}

void LawnApp::ShowResourceError(bool doExit)
{
    SexyAppBase::ShowResourceError(doExit);
}

/*
void BetaSubmitFunc()
{
    if (gLawnApp)
    {
        gLawnApp->BetaSubmit(false);
    }
}
*/

// GOTY @Patoke: 0x454C60
void LawnApp::Init()
{
    DoParseCmdLine();
    if (!mTodCheatKeys)
    {
        mOnlyAllowOneCopyToRun = true;
    }

    // GOTY @Patoke: 0x60C590
    //if (!gSexyCache->Connected() &&
    //  gLawnApp->mTodCheatKeys &&
    //  MessageBox(gLawnApp->mHWnd, "Start SexyCache now?", "SexyCache", MB_YESNO) == IDYES &&
    //  WinExec("SexyCache.exe", SW_MINIMIZE) >= 32)
    //{
    //  // GOTY @Patoke: 0x60C490
    //  gSexyCache = SexyCache();
    //}
    //if (gSexyCache->Connected() && !gLawnApp->mTodCheatKeys)
    //{
    //  // GOTY @Patoke: 0x60C5B0
    //  gSexyCache->Disconnect();
    //}

    mSessionID = time(0);
    mPlayTimeActiveSession = 0;
    mPlayTimeInactiveSession = 0;
    mBoardResult = BOARDRESULT_NONE;
    mSawYeti = false;

    // SexyApp::Init() removed for Symbian

    if (mShutdown) // MakeWindow() failed
        return;

    // @Patoke: horrible debug checks, breaks the whole exe in release mode
//#ifdef PVZ_DEBUG
    TodAssertInitForApp();
    TodLog("session id: %u", mSessionID);
//#endif

    if (!mResourceManager->ParseResourcesFile("properties/resources.xml"))
    {
        ShowResourceError(true);
        return;
    }

    if (!TodLoadResources("Init"))
    {
        return;
    }

    PerfTimer mTimer;
    mTimer.Start();

    mProfileMgr->Load();

    std::string aCurUser;
    mPlayerInfo = mProfileMgr->GetAnyProfile();

    if (mPlayerInfo == NULL)
    {
        mPlayerInfo = mProfileMgr->GetAnyProfile();
    }

    // [M4 #1 fix] ProfileMgr is a no-op stub in this port -- GetAnyProfile
    // returns NULL. Many code paths (PreNewGame, Board::InitLevel, etc.)
    // dereference mPlayerInfo without a NULL check, causing KERN-EXEC 3.
    // Create a default PlayerInfo so all mPlayerInfo-> accesses are safe.
    // This gives the player id=0, level=1 (adventure start), no progress.
    // Once ProfileMgr is properly ported with real persistence, this fallback
    // can be removed.
    if (mPlayerInfo == NULL)
    {
        mPlayerInfo = new PlayerInfo();
        mPlayerInfo->mId = 0;
        mPlayerInfo->mName = "Player";
        mPlayerInfo->mLevel = 1;
    }

    mMaxExecutions = GetInteger("MaxExecutions", 0);
    mMaxPlays = GetInteger("MaxPlays", 0);
    mMaxTime = GetInteger("MaxTime", 60);

    mTitleScreen = new TitleScreen(this);
    mTitleScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mTitleScreen);
    mWidgetManager->SetFocus(mTitleScreen);

#ifdef PVZ_DEBUG
    int aDuration = mTimer.GetDuration();
    TodTrace("loading: 'profiles' %d ms", aDuration);
#endif
    mTimer.Start();

    mMusic = new Music();
    mSoundSystem = new TodFoley();
    mEffectSystem = new EffectSystem();
    mEffectSystem->EffectSystemInitialize();

    mKonamiCheck = new TypingCheck();
    mKonamiCheck->AddKeyCode(KEYCODE_UP);
    mKonamiCheck->AddKeyCode(KEYCODE_UP);
    mKonamiCheck->AddKeyCode(KEYCODE_DOWN);
    mKonamiCheck->AddKeyCode(KEYCODE_DOWN);
    mKonamiCheck->AddKeyCode(KEYCODE_LEFT);
    mKonamiCheck->AddKeyCode(KEYCODE_RIGHT);
    mKonamiCheck->AddKeyCode(KEYCODE_LEFT);
    mKonamiCheck->AddKeyCode(KEYCODE_RIGHT);
    mKonamiCheck->AddChar('b');
    mKonamiCheck->AddChar('a');
    mMustacheCheck = new TypingCheck("mustache");
    mMoustacheCheck = new TypingCheck("moustache");
    mSuperMowerCheck = new TypingCheck("trickedout");
    mSuperMowerCheck2 = new TypingCheck("tricked out");
    mFutureCheck = new TypingCheck("future");
    mPinataCheck = new TypingCheck("pinata");
    mDanceCheck = new TypingCheck("dance");
    mDaisyCheck = new TypingCheck("daisies");
    mSukhbirCheck = new TypingCheck("sukhbir");

#ifdef PVZ_DEBUG
    aDuration = mTimer.GetDuration();
    TodTrace("loading: 'system' %d ms", aDuration);
#endif
    mTimer.Start();

    ReanimatorLoadDefinitions(gLawnReanimationArray, NUM_REANIMS);
    ReanimatorEnsureDefinitionLoaded(REANIM_LOADBAR_SPROUT, true);
    ReanimatorEnsureDefinitionLoaded(REANIM_LOADBAR_ZOMBIEHEAD, true);

#ifdef PVZ_DEBUG
    aDuration = mTimer.GetDuration();
    TodTrace("loading: 'loaderbar' %d ms", aDuration);
#endif
    mTimer.Start();
}

void LawnApp::Start()
{
    if (mLoadingFailed)
        return;

    SexyAppBase::Start();
}

bool LawnApp::DebugKeyDown(int theKey)
{
    return SexyAppBase::DebugKeyDown(theKey);
}

// GOTY @Patoke: 0x41E420
bool LawnApp::UpdatePlayerProfileForFinishingLevel()
{
    bool aUnlockedNewChallenge = false;

    if (IsAdventureMode())
    {
        if (mBoard->mLevel == FINAL_LEVEL)
        {
            mPlayerInfo->SetLevel(1);  // 存档回到第 1-1 关
            mPlayerInfo->mFinishedAdventure++;  // 完成冒险模式周目数增加 1 次
            if (mPlayerInfo->mFinishedAdventure == 1)
            {
                mPlayerInfo->mNeedsMessageOnGameSelector = 1;
            }
            ReportAchievement::GiveAchievement(this, HomeSecurity, false); // @Patoke: add achievement
        }
        else
        {
            mPlayerInfo->SetLevel(mBoard->mLevel + 1);  // 存档进入下一关
        }

        if (!HasFinishedAdventure() && mBoard->mLevel == 34)
        {
            mPlayerInfo->mNeedsMagicTacoReward = 1;
        }
    }
    else if (IsSurvivalMode())
    {
        if (mBoard->IsFinalSurvivalStage())
        {
            aUnlockedNewChallenge = !HasBeatenChallenge(mGameMode);
            mBoard->SurvivalSaveScore();

            if (aUnlockedNewChallenge && HasFinishedAdventure())
            {
                int aNumTrophies = GetNumTrophies(CHALLENGE_PAGE_SURVIVAL);
                if (aNumTrophies != 8 && aNumTrophies != 9)
                {
                    mPlayerInfo->mHasNewSurvival = true;
                }
            }
        }
    }
    else if (IsPuzzleMode())
    {
        aUnlockedNewChallenge = !HasBeatenChallenge(mGameMode);
        mPlayerInfo->mChallengeRecords[GetCurrentChallengeIndex()]++;

        if (!HasFinishedAdventure() && (mGameMode == GAMEMODE_SCARY_POTTER_3 || mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_3))
        {
            aUnlockedNewChallenge = false;
        }

        if (aUnlockedNewChallenge)
        {
            if (IsScaryPotterLevel())
            {
                mPlayerInfo->mHasNewScaryPotter = 1;
            }
            else
            {
                mPlayerInfo->mHasNewIZombie = 1;
            }
        }
    }
    else
    {
        aUnlockedNewChallenge = !HasBeatenChallenge(mGameMode);
        mPlayerInfo->mChallengeRecords[GetCurrentChallengeIndex()]++;

        if (aUnlockedNewChallenge && HasFinishedAdventure())
        {
            int aNumTrophies = GetNumTrophies(CHALLENGE_PAGE_CHALLENGE);
            if (aNumTrophies <= 17)
            {
                mPlayerInfo->mHasNewMiniGame = 1;
            }
        }

        int aNumTrophies = GetNumTrophies(CHALLENGE_PAGE_CHALLENGE);
        if (aNumTrophies == 20)
            ReportAchievement::GiveAchievement(this, BeyondTheGrave, false);
    }

    if ((IsAdventureMode() || IsSurvivalMode()) && !IsScaryPotterLevel() && !IsWhackAZombieLevel()) {
        if (mBoard->StageIsDayWithPool() && !mBoard->mPeaShooterUsed) {
            ReportAchievement::GiveAchievement(this, DontPea, false);
        } else if (mBoard->StageHasRoof() && !mBoard->HasConveyorBeltSeedBank() && !mBoard->mCatapultPlantsUsed) {
            ReportAchievement::GiveAchievement(this, Grounded, false);
        } else if (mBoard->StageIsDayWithoutPool() && mBoard->mMushroomAndCoffeeBeansOnly) {
            ReportAchievement::GiveAchievement(this, GoodMorning, false);
        }
        if (mBoard->StageIsNight() && !mBoard->mMushroomsUsed) {
            ReportAchievement::GiveAchievement(this, NoFungusAmongUs, false);
        }
    }

    WriteCurrentUserConfig();

    return aUnlockedNewChallenge;
}

// GOTY @Patoke: 0x4558E0
void LawnApp::CheckForGameEnd()
{
    if (mBoard == NULL || !mBoard->mLevelComplete)
        return;

    bool aUnlockedNewChallenge = UpdatePlayerProfileForFinishingLevel();

    if (IsAdventureMode())
    {
        int aLevel = mBoard->mLevel;
        KillBoard();

        if (IsFirstTimeAdventureMode() && aLevel < 50)
        {
            ShowAwardScreen(AWARD_FORLEVEL, true);
        }
        else if (aLevel == FINAL_LEVEL)
        {
            if (mPlayerInfo->mFinishedAdventure == 1)
            {
                ShowAwardScreen(AWARD_FORLEVEL, true);
            }
            else
            {
                ShowAwardScreen(AWARD_CREDITS_ZOMBIENOTE, true);
            }
        }
        else if (aLevel == 9 || aLevel == 19 || aLevel == 29 || aLevel == 39 || aLevel == 49)
        {
            ShowAwardScreen(AWARD_FORLEVEL, true);
        }
        else if (HasUnshownAchievements(mPlayerInfo))
        {
            ShowAwardScreen(AWARD_ACHIEVEMENTONLY, true);
        }
        else
        {
            PreNewGame(mGameMode, false);
        }
    }
    else if (IsSurvivalMode())
    {
        if (mBoard->IsFinalSurvivalStage())
        {
            KillBoard();

            if (aUnlockedNewChallenge && HasFinishedAdventure())
            {
                ShowAwardScreen(AWARD_FORLEVEL, true);
            }
            else if (HasUnshownAchievements(mPlayerInfo))
            {
                ShowAwardScreen(AWARD_ACHIEVEMENTONLY, true);
            }
            else
            {
                ShowChallengeScreen(CHALLENGE_PAGE_SURVIVAL);
            }
        }
        else
        {
            mBoard->mChallenge->mSurvivalStage++;
            KillGameSelector();
            mBoard->InitSurvivalStage();
        }
    }
    else if (IsPuzzleMode())
    {
        KillBoard();

        if (aUnlockedNewChallenge)
        {
            ShowAwardScreen(AWARD_FORLEVEL, true);
        }
        else if (HasUnshownAchievements(mPlayerInfo))
        {
            ShowAwardScreen(AWARD_ACHIEVEMENTONLY, true);
        }
        else
        {
            ShowChallengeScreen(CHALLENGE_PAGE_PUZZLE);
        }
    }
    else
    {
        KillBoard();

        if (aUnlockedNewChallenge && HasFinishedAdventure())
        {
            ShowAwardScreen(AWARD_FORLEVEL, true);
        }
        else if (HasUnshownAchievements(mPlayerInfo))
        {
            ShowAwardScreen(AWARD_ACHIEVEMENTONLY, true);
        }
        else
        {
            ShowChallengeScreen(CHALLENGE_PAGE_CHALLENGE);
        }
    }
}

void LawnApp::UpdatePlayTimeStats()
{
    static int aLastTime = -1;

    // Symbian: use User::TickCount() instead of SDL_GetTicks()
    int aTickCount = User::TickCount();
    int aSession = (aTickCount - aLastTime) / 1000;

    if (mPlayerInfo && !mPlayerInfo->mHasUsedCheatKeys && !mDebugKeysEnabled && mTodCheatKeys)
    {
        mPlayerInfo->mHasUsedCheatKeys = 1;
    }

    if (aLastTime == -1)
    {
        aLastTime = aTickCount;
        return;
    }

    if (aSession > 0)
    {
        aLastTime = aTickCount;

        if ((mBoard == NULL || !mBoard->mPaused) && mHasFocus && mLastTimerTime - mLastUserInputTick <= 10000)
        {
            mPlayTimeActiveSession += aSession;

            if (mBoard)
            {
                mBoard->mPlayTimeActiveLevel += aSession;
            }

            if (mPlayerInfo)
            {
                mPlayerInfo->mPlayTimeActivePlayer += aSession;
            }
        }
        else
        {
            mPlayTimeInactiveSession += aSession;

            if (mBoard)
            {
                mBoard->mPlayTimeInactiveLevel += aSession;
            }

            if (mPlayerInfo)
            {
                mPlayerInfo->mPlayTimeInactivePlayer += aSession;
            }
        }
    }
}

void LawnApp::UpdateFrames()
{
    if ((!mActive || mMinimized) && mBoard)
    {
        mBoard->ResetFPSStats();
    }

#ifdef PVZ_DEBUG
    UpdatePlayTimeStats();
#endif

    int aUpdateCount = 1;
    if (gSlowMo)
    {
        ++gSlowMoCounter;
        if (gSlowMoCounter < 4)
        {
            aUpdateCount = 0;
        }
        else
        {
            gSlowMoCounter = 0;
        }
    }
    else if (gFastMo)
    {
        aUpdateCount = 20;
    }

    for (int i = 0; i < aUpdateCount; i++)
    {
        mAppCounter++;

        if (mBoard)
        {
            mBoard->ProcessDeleteQueue();
        }
        if (mLoadingThreadCompleted && mEffectSystem)
        {
            mEffectSystem->ProcessDeleteQueue();
        }

        SexyAppBase::UpdateFrames();

        mMusic->MusicUpdate();

        CheckForGameEnd();
    }
}

void LawnApp::ToggleSlowMo()
{
    gSlowMoCounter = 0;
    gSlowMo = !gSlowMo;
    gFastMo = false;
}

void LawnApp::ToggleFastMo()
{
    gSlowMo = false;
    gFastMo = !gFastMo;
}

void LawnApp::LoadGroup(const char* theGroupName, int theGroupAveMsToLoad)
{
    PerfTimer aTimer;
    aTimer.Start();

    mResourceManager->StartLoadResources(theGroupName);
    while (!mShutdown && !mCloseRequest && !mLoadingFailed && TodLoadNextResource())
    {
        mCompletedLoadingThreadTasks += theGroupAveMsToLoad;
    }

    if (mShutdown || mCloseRequest)
        return;

    if (mResourceManager->HadError() || !ExtractResourcesByName(mResourceManager, theGroupName))
    {
        ShowResourceError();
        mLoadingFailed = true;
    }

    //int aTotalGroupWeight = mResourceManager->GetNumResources(theGroupName) * theGroupAveMsToLoad;
    //int aGroupTime = max(aTimer.GetDuration(), 0.0);
    //TraceLoadGroup(theGroupName, aGroupTime, aTotalGroupWeight, theGroupAveMsToLoad);
}

void LawnApp::LoadingThreadProc()
{
    // [Session-10] Load ONLY essential images for the loading screen first,
    // so the first frame renders quickly. The original PvZ loads resources
    // PROGRESSIVELY during the loading bar animation — not all at once before
    // the first frame. The current port loads everything synchronously here,
    // which is slow (hundreds of images) and causes OOM crashes.
    //
    // For now: load LoaderBar group + PopCap logo + TitleScreen first, so the
    // loading screen + PopCap intro can render immediately. The rest
    // (LoadingImages, LoadingFonts, etc.) loads after.

    // Load PopCap logo early so TitleScreen can show it during the intro.
    if (mResourceManager)
        mResourceManager->GetImage("IMAGE_POPCAP_LOGO");

    // [M4 #1 fix] TodLoadResources is a no-op stub in this port (Stubs.h:
    // inline bool TodLoadResources(const char*) { return true; }). It returns
    // true but loads NOTHING. This means ExtractLoaderBarResources never ran,
    // so IMAGE_LOADBAR_DIRT / IMAGE_LOADBAR_GRASS / IMAGE_PVZ_LOGO stayed NULL.
    // Call ExtractResourcesByName directly to actually load the LoaderBar group.
    if (!ExtractResourcesByName(mResourceManager, "LoaderBar"))
    {
        // Continue anyway -- the loading screen will fall back to a plain bar.
    }

    TodStringListLoad("Properties/LawnStrings.txt");

    // Load localized properties AFTER LawnStrings so they can override string values
    LoadProperties("properties/default.xml", false, false);
    LoadProperties("properties/Layout.xml", false, false);

    if (mTitleScreen)
    {
        mTitleScreen->mLoaderScreenIsLoaded = true;
    }

    const char* groups[] = { "LoadingFonts", "LoadingImages", "LoadingSounds" };
    int group_ave_ms_to_load[] = { 54, 9, 54 };
    for (int i = 0; i < 3; i++)
    {
        mNumLoadingThreadTasks += mResourceManager->GetNumResources(groups[i]) * group_ave_ms_to_load[i];
    }
    mNumLoadingThreadTasks += 636;
    mNumLoadingThreadTasks += GetNumPreloadingTasks();
    mNumLoadingThreadTasks += mMusic->GetNumLoadingTasks();

    PerfTimer aTimer;
    aTimer.Start();

    TodHesitationTrace("start loading");
    TodHesitationBracket aHesitationResources("Resources");
    TodHesitationTrace("loading thread start");

    LoadGroup("LoadingImages", 9);
    LoadGroup("LoadingFonts", 54);
    // [M4 #3] Load IMAGE_BACKGROUND1 (lawn background) for the main menu.
    // This is a delay-load group (not loaded at boot in upstream), but the
    // menu needs it as background. PAK has images/background1.jpg (171KB).
    ExtractResourcesByName(mResourceManager, "DelayLoad_Background1");
    if (mLoadingFailed || mShutdown || mCloseRequest)
        return;

    aHesitationResources.EndBracket();
    TodTrace("loading '%s' %d ms", "resources", static_cast<int>(aTimer.GetDuration()));

    mMusic->MusicInit();
    // aDuration goes unused
    //int aDuration = max(aTimer.GetDuration(), 0.0);
    aTimer.Start();

    mPoolEffect = new PoolEffect();
    mPoolEffect->PoolEffectInitialize();
    mZenGarden = new ZenGarden();
    mReanimatorCache = new ReanimatorCache();
    mReanimatorCache->ReanimatorCacheInitialize();
    TodFoleyInitialize(gLawnFoleyParamArray, LENGTH(gLawnFoleyParamArray));

    TodTrace("loading '%s' %d ms", "stuff", static_cast<int>(aTimer.GetDuration()));
    aTimer.Start();

    TrailLoadDefinitions(gLawnTrailArray, LENGTH(gLawnTrailArray));
    TodTrace("loading '%s' %d ms", "trail", static_cast<int>(aTimer.GetDuration()));
    aTimer.Start();
    TodHesitationTrace("trail");

    TodParticleLoadDefinitions(gLawnParticleArray, LENGTH(gLawnParticleArray));
    //aDuration = max(aTimer.GetDuration(), 0.0);
    aTimer.Start();

    PreloadForUser();
    if (mLoadingFailed || mShutdown || mCloseRequest)
        return;

    //aDuration = max(aTimer.GetDuration(), 0.0);
    aTimer.Start();

    GetNumPreloadingTasks();
    LoadGroup("LoadingSounds", 54);
    TodHesitationTrace("finished loading");
}

void LawnApp::FastLoad(GameMode theGameMode)
{
    if (!mShutdown)
    {
        mWidgetManager->RemoveWidget(mTitleScreen);
        SafeDeleteWidget(mTitleScreen);
        mTitleScreen = NULL;

        PreNewGame(theGameMode, false);
    }
}

void LawnApp::LoadingThreadCompleted()
{
}

// GOTY @Patoke: 0x456150
void LawnApp::LoadingCompleted()
{
    // [M4 #1 diag] Dump widget list + mTitleScreen pointer BEFORE RemoveWidget,
    // so we can see whether mTitleScreen actually matches a widget in the array.
    // Previous run showed: after RemoveWidget+delete, 1 widget (ptr=0072a2a0)
    // remained -- meaning mTitleScreen pointed somewhere else, OR RemoveWidget
    // failed to find it. This dump resolves which.
    {
        RFs fs; RFile f;
        if (fs.Connect() == KErrNone)
        {
            fs.MkDirAll(_L("C:\\Data\\PvZ"));
            if (f.Replace(fs, _L("C:\\Data\\PvZ\\wgt_dump.txt"),
                          EFileWrite | EFileShareAny) == KErrNone)
            {
                TBuf8<96> hdr;
                hdr.Format(_L8("LoadingCompleted ENTER: mTitleScreen=%08x, %d widgets in manager\n"),
                           (TUint)mTitleScreen, mWidgetManager->GetWidgetCount());
                f.Write(hdr);
                for (int i = 0; i < mWidgetManager->GetWidgetCount(); i++)
                {
                    Sexy::Widget* w = mWidgetManager->GetWidgetAt(i);
                    if (w)
                    {
                        TBuf8<128> line;
                        line.Format(_L8("  [%d] ptr=%08x x=%d y=%d w=%d h=%d vis=%d\n"),
                                    i, (TUint)w,
                                    (TInt)w->mX, (TInt)w->mY,
                                    (TInt)w->mWidth, (TInt)w->mHeight,
                                    (TInt)w->mVisible);
                        f.Write(line);
                    }
                }
                f.Write(_L8("--- end ---\n"));
                f.Flush();
                f.Close();
            }
            fs.Close();
        }
    }

    mWidgetManager->RemoveWidget(mTitleScreen);
    SafeDeleteWidget(mTitleScreen);
    mTitleScreen = NULL;

    // [M4 #1 fix] STALE WIDGET CLEANUP -- safe version.
    // Previous run showed: after RemoveWidget(mTitleScreen) + delete, ONE widget
    // (ptr=0072a2a0, 0,0,400,300) was STILL in the manager. That widget drew
    // purple FillRect + unscaled titlescreen on top of GameSelector.
    // mTitleScreen didn't match its pointer (else RemoveWidget would have caught
    // it). So there's a SECOND full-screen widget in the array that we don't
    // have a named pointer to. Scan the array and remove any full-screen widget
    // (i.e. covers the whole 400x300 canvas). This is safe because at
    // LoadingCompleted time the ONLY legitimate full-screen widget would be
    // GameSelector -- which hasn't been created yet (ShowGameSelector runs
    // after this). We do NOT delete the widget objects (they may be owned
    // elsewhere); we only remove them from the manager's draw/hit-test array.
    //
    // Iterate BACKWARDS because RemoveWidget shifts array elements down --
    // forward iteration would skip the element after a removed one.
    for (int i = mWidgetManager->GetWidgetCount() - 1; i >= 0; i--)
    {
        Sexy::Widget* w = mWidgetManager->GetWidgetAt(i);
        if (w && w->mX == 0 && w->mY == 0 &&
            w->mWidth >= 400 && w->mHeight >= 300)
        {
            mWidgetManager->RemoveWidget(w);
        }
    }

    // [M4 #1 fix] HARD RESET -- if the loop above didn't clear the stale
    // widget (RemoveWidget appears unreliable under GCCE 3.4.3), zero the
    // widget count directly. mWidgetCount is public in WidgetContainer.h.
    // Safe because at LoadingCompleted time no legitimate widgets exist yet
    // (GameSelector + buttons are created in ShowGameSelector after this).
    if (mWidgetManager->GetWidgetCount() > 0)
    {
        mWidgetManager->mWidgetCount = 0;
    }

    // [M4 #1 diag] Dump AGAIN after RemoveWidget+delete to see what remains.
    {
        RFs fs; RFile f;
        if (fs.Connect() == KErrNone)
        {
            fs.MkDirAll(_L("C:\\Data\\PvZ"));
            if (f.Open(fs, _L("C:\\Data\\PvZ\\wgt_dump.txt"),
                       EFileWrite | EFileShareAny) == KErrNone)
            {
                TInt pos = 0; f.Seek(ESeekEnd, pos);
                TBuf8<96> hdr;
                hdr.Format(_L8("\nLoadingCompleted AFTER RemoveWidget+delete: %d widgets remain\n"),
                           mWidgetManager->GetWidgetCount());
                f.Write(hdr);
                for (int i = 0; i < mWidgetManager->GetWidgetCount(); i++)
                {
                    Sexy::Widget* w = mWidgetManager->GetWidgetAt(i);
                    if (w)
                    {
                        TBuf8<128> line;
                        line.Format(_L8("  [%d] ptr=%08x x=%d y=%d w=%d h=%d vis=%d\n"),
                                    i, (TUint)w,
                                    (TInt)w->mX, (TInt)w->mY,
                                    (TInt)w->mWidth, (TInt)w->mHeight,
                                    (TInt)w->mVisible);
                        f.Write(line);
                    }
                }
                f.Write(_L8("--- end ---\n"));
                f.Flush();
                f.Close();
            }
            fs.Close();
        }
    }

    // NOTE: Keep IMAGE_TITLESCREEN in the ResourceManager cache even though the
    // loading screen is gone. GameSelector::Draw() calls
    //   mResourceManager->GetImage("IMAGE_TITLESCREEN")
    // EACH FRAME to obtain the title art. If we DeleteImage here, every frame
    // triggers a full ICL re-decode (LoadImageByResName -> LoadImageFromPak ->
    // CImageDecoder -> CActiveSchedulerWait). The SECOND ICL decode of the same
    // JPEG hangs/fails on the N95 (rmgr_log showed endless "Convert via CActive"
    // with no "convert done"), causing either a freeze or heap exhaustion and a
    // silent exit shortly after the loader finishes.
    //
    // Old crash fix (nulling IMAGE_TITLESCREEN) was for the TitleScreen dangling-
    // pointer KERN-EXEC 3 -- that is no longer relevant because TitleScreen is
    // destroyed AND removed from the widget list above, so its Draw() never runs.
    // The global pointer staying non-NULL is harmless.

    ShowGameSelector();
}

void LawnApp::URLOpenFailed(const std::string& theURL)
{
    SexyAppBase::URLOpenFailed(theURL);
    KillDialog(DIALOG_OPENURL_WAIT);
    CopyToClipboard(theURL);

    std::string aString =
        "Please open the following URL in your browser\n\n" +
        theURL +
        "\n\nFor your convenience, this URL has already been copied to your clipboard.";

    DoDialog(DIALOG_OPENURL_WAIT, true, "Open Browser", "[DIALOG_BUTTON_OK]", aString, Dialog::BUTTONS_FOOTER);
}

void LawnApp::URLOpenSucceeded(const std::string& theURL)
{
    SexyAppBase::URLOpenSucceeded(theURL);
    KillDialog(DIALOG_OPENURL_WAIT);
}

bool LawnApp::OpenURL(const std::string& theURL, bool shutdownOnOpen)
{
    DoDialog(
        DIALOG_OPENURL_WAIT,
        true,
        "Opening Browser",
        "Opening Browser",
        "",
        Dialog::BUTTONS_NONE
    );

    DrawDirtyStuff();

    return SexyAppBase::OpenURL(theURL, shutdownOnOpen);
}

// GOTY @Patoke: 0x4564F0
void LawnApp::ConfirmQuit()
{
    std::string aBody = TodStringTranslate("[QUIT_MESSAGE]");
    std::string aHeader = TodStringTranslate("[QUIT_HEADER]");
    LawnDialog* aDialog = (LawnDialog*)DoDialog(DIALOG_QUIT, true, aHeader, aBody, "", Dialog::BUTTONS_OK_CANCEL);
    aDialog->mLawnYesButton->mLabel = TodStringTranslate("[QUIT_BUTTON]");
    CenterDialog(aDialog, aDialog->mWidth, aDialog->mHeight);
}

void LawnApp::ButtonPress(int) {}
void LawnApp::ButtonDownTick(int) {}
void LawnApp::ButtonMouseEnter(int) {}
void LawnApp::ButtonMouseLeave(int) {}
void LawnApp::ButtonMouseMove(int, int, int) {}

// GOTY @Patoke: 0x456690
void LawnApp::ButtonDepress(int theId)
{
    if (theId % 10000 >= 2000 && theId % 10000 < 3000)  // 按钮编号 theId ∈ [2000, 3000) 时，表示按下 theId - 2000 编号的对话中的"是"按钮
    {
        switch (theId - 2000)
        {
        case DIALOG_NEW_GAME:
            KillDialog(DIALOG_NEW_GAME);
            ShowGameSelector();
            return;

        case DIALOG_NEWOPTIONS:
            KillNewOptionsDialog();
            return;

        case DIALOG_PREGAME_NAG:
            DoRegister();
            return;

        case DIALOG_LOAD_GAME:
            return;

        case DIALOG_CONFIRM_UPDATE_CHECK:
            KillDialog(DIALOG_CONFIRM_UPDATE_CHECK);
            CheckForUpdates();
            return;

        case DIALOG_QUIT:
            KillDialog(DIALOG_QUIT);
            CloseRequestAsync();
            return;

        case DIALOG_NAG:
            KillDialog(DIALOG_NAG);
            DoRegister();
            return;

        case DIALOG_INFO:
            KillDialog(DIALOG_INFO);
            return;

        case DIALOG_PAUSED:
            KillDialog(DIALOG_PAUSED);
            return;

        case DIALOG_NO_MORE_MONEY:
            KillDialog(DIALOG_NO_MORE_MONEY);
            mBoard->AddSunMoney(100);
            return;

        case DIALOG_BONUS:
            KillDialog(DIALOG_BONUS);
            return;

        case DIALOG_CONFIRM_BACK_TO_MAIN:
            KillDialog(DIALOG_CONFIRM_BACK_TO_MAIN);
            mBoardResult = BOARDRESULT_QUIT;
            mBoard->TryToSaveGame();
            DoBackToMain();
            return;

        case DIALOG_USERDIALOG:
            FinishUserDialog(true);
            return;

        case DIALOG_CREATEUSER:
            FinishCreateUserDialog(true);
            return;

        case DIALOG_CONFIRMDELETEUSER:
            FinishConfirmDeleteUserDialog(true);
            return;

        case DIALOG_RENAMEUSER:
            FinishRenameUserDialog(true);
            return;

        case DIALOG_CREATEUSERERROR:
        case DIALOG_RENAMEUSERERROR:
            FinishNameError(theId - 2000);
            return;

        case DIALOG_CHEAT:
            FinishCheatDialog(true);
            return;

        case DIALOG_RESTARTCONFIRM:
            FinishRestartConfirmDialog();
            return;

        case DIALOG_TIMESUP:
            FinishTimesUpDialog();
            return;

        case 20008:
            KillDialog(20008);
            KillDialog(DIALOG_CHECKING_UPDATES);
            return;

        default:
            KillDialog(theId - 2000);
            return;
        }
    }

    if (theId % 10000 >= 3000 && theId < 4000)  // 按钮编号 theId ∈ [3000, 4000) 时，表示按下 theId - 3000 编号的对话中的"否"按钮
    {
        switch (theId - 3000)
        {
        case DIALOG_PREGAME_NAG:
            KillDialog(DIALOG_PREGAME_NAG);
            Shutdown();
            return;

        case DIALOG_LOAD_GAME:
            KillDialog(DIALOG_LOAD_GAME);
            return;

        case DIALOG_USERDIALOG:
            FinishUserDialog(false);
            return;

        case DIALOG_CREATEUSER:
            FinishCreateUserDialog(false);
            return;

        case DIALOG_CONFIRMDELETEUSER:
            FinishConfirmDeleteUserDialog(false);
            return;

        case DIALOG_RENAMEUSER:
            FinishRenameUserDialog(false);
            return;

        case DIALOG_CHEAT:
            FinishCheatDialog(false);
            return;

        case DIALOG_TIMESUP:
            FinishTimesUpDialog();
            return;

        case 10008:
            KillDialog(10008);
            KillDialog(DIALOG_CHECKING_UPDATES);
            return;

        default:
            KillDialog(theId - 3000);
            return;
        }
    }
}

// GOTY @Patoke: 0x4535CD
void LawnApp::CenterDialog(Dialog* theDialog, int theWidth, int theHeight)
{
    theDialog->Resize((BOARD_WIDTH - theWidth) / 2, (BOARD_HEIGHT - theHeight) / 2, theWidth, theHeight);
}

// GOTY @Patoke: 0x456B00
void LawnApp::PlayFoley(int theFoleyType)
{
    if (!mMuteSoundsForCutscene)
    {
        mSoundSystem->PlayFoley(theFoleyType);
    }
}

void LawnApp::PlayFoleyPitch(int theFoleyType, float thePitch)
{
    if (!mMuteSoundsForCutscene)
    {
        mSoundSystem->PlayFoleyPitch(theFoleyType, thePitch);
    }
}

std::string LawnApp::GetStageString(int theLevel)
{
    int aArea = ClampInt((theLevel - 1) / LEVELS_PER_AREA + 1, 1, ADVENTURE_AREAS + 1);
    int aSub = theLevel - (aArea - 1) * LEVELS_PER_AREA;
    return StrFormat("%d-%d", aArea, aSub);
}

bool LawnApp::IsAdventureMode()
{
    return mGameMode == GAMEMODE_ADVENTURE;
}

bool LawnApp::IsSurvivalMode()
{
    return mGameMode >= GAMEMODE_SURVIVAL_NORMAL_STAGE_1 && mGameMode <= GAMEMODE_SURVIVAL_ENDLESS_STAGE_5;
}

bool LawnApp::IsPuzzleMode()
{
    return
        (mGameMode >= GAMEMODE_SCARY_POTTER_1 && mGameMode <= GAMEMODE_SCARY_POTTER_ENDLESS) ||
        (mGameMode >= GAMEMODE_PUZZLE_I_ZOMBIE_1 && mGameMode <= GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS);
}

bool LawnApp::IsChallengeMode()
{
    return !IsAdventureMode() && !IsPuzzleMode() && !IsSurvivalMode();
}

bool LawnApp::IsSurvivalNormal(GameMode theGameMode)
{
    int aLevel = theGameMode - GAMEMODE_SURVIVAL_NORMAL_STAGE_1;
    return aLevel >= 0 && aLevel <= 4;
}

bool LawnApp::IsSurvivalHard(GameMode theGameMode)
{
    int aLevel = theGameMode - GAMEMODE_SURVIVAL_HARD_STAGE_1;
    return aLevel >= 0 && aLevel <= 4;
}

bool LawnApp::IsSurvivalEndless(GameMode theGameMode)
{
    int aLevel = theGameMode - GAMEMODE_SURVIVAL_ENDLESS_STAGE_1;
    return aLevel >= 0 && aLevel <= 4;
}

bool LawnApp::IsEndlessScaryPotter(GameMode theGameMode)
{
    return theGameMode == GAMEMODE_SCARY_POTTER_ENDLESS;
}

bool LawnApp::IsEndlessIZombie(GameMode theGameMode)
{
    return theGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS;
}

bool LawnApp::IsContinuousChallenge()
{
    return
        IsArtChallenge() ||
        IsSlotMachineLevel() ||
        IsFinalBossLevel() ||
        mGameMode == GAMEMODE_CHALLENGE_BEGHOULED ||
        mGameMode == GAMEMODE_UPSELL ||
        mGameMode == GAMEMODE_INTRO ||
        mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST;
}

bool LawnApp::IsArtChallenge()
{
    if (mBoard == NULL)
        return false;

    return
        mGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT ||
        mGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER ||
        mGameMode == GAMEMODE_CHALLENGE_SEEING_STARS;
}

bool LawnApp::IsSquirrelLevel()
{
    return mBoard && mGameMode == GAMEMODE_CHALLENGE_SQUIRREL;
}

bool LawnApp::IsIZombieLevel()
{
    if (mBoard == NULL)
        return false;

    return
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_1 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_2 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_3 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_4 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_5 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_6 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_7 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_8 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_9 ||
        mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS;
}

bool LawnApp::IsShovelLevel()
{
    return mBoard && mGameMode == GAMEMODE_CHALLENGE_SHOVEL;
}

// GOTY @Patoke: 0x456D10
bool LawnApp::IsWallnutBowlingLevel()
{
    if (mBoard == NULL)
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING || mGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2)
        return true;

    return IsAdventureMode() && mBoard->mLevel == 5;
}

bool LawnApp::IsSlotMachineLevel()
{
    return (mBoard && mGameMode == GAMEMODE_CHALLENGE_SLOT_MACHINE);
}

bool LawnApp::IsWhackAZombieLevel()
{
    if (mBoard == NULL)
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE)
        return true;

    return IsAdventureMode() && mBoard->mLevel == 15;
}

bool LawnApp::IsLittleTroubleLevel()
{
    return (mBoard && (mGameMode == GAMEMODE_CHALLENGE_LITTLE_TROUBLE || (mGameMode == GAMEMODE_ADVENTURE && mBoard->mLevel == 25)));
}

bool LawnApp::IsScaryPotterLevel()
{
    if (mGameMode >= GAMEMODE_SCARY_POTTER_1 && mGameMode <= GAMEMODE_SCARY_POTTER_ENDLESS)
        return true;

    return IsAdventureMode() && mBoard && mBoard->mLevel == 35;
}

bool LawnApp::IsStormyNightLevel()
{
    if (mBoard == NULL)
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_STORMY_NIGHT)
        return true;

    return IsAdventureMode() && mBoard->mLevel == 40;
}

bool LawnApp::IsBungeeBlitzLevel()
{
    if (mBoard == NULL)
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_BUNGEE_BLITZ)
        return true;

    return IsAdventureMode() && mBoard->mLevel == 45;
}

bool LawnApp::IsMiniBossLevel()
{
    if (mBoard == NULL)
        return false;

    return IsAdventureMode() && (mBoard->mLevel == 10 || mBoard->mLevel == 20 || mBoard->mLevel == 30);
}

bool LawnApp::IsFinalBossLevel()
{
    if (mBoard == NULL)
        return false;

    if (mGameMode == GAMEMODE_CHALLENGE_FINAL_BOSS)
        return true;

    return IsAdventureMode() && mBoard->mLevel == 50;
}

bool LawnApp::IsChallengeWithoutSeedBank()
{
    return
        mGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS ||
        mGameMode == GAMEMODE_UPSELL ||
        mGameMode == GAMEMODE_INTRO ||
        IsWhackAZombieLevel() ||
        IsSquirrelLevel() ||
        IsScaryPotterLevel() ||
        mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN ||
        mGameMode == GAMEMODE_TREE_OF_WISDOM;
}

bool LawnApp::IsNight()
{
    if (IsIceDemo() || mPlayerInfo == NULL)
        return false;

    return (mPlayerInfo->mLevel >= 11 && mPlayerInfo->mLevel <= 20) || (mPlayerInfo->mLevel >= 31 && mPlayerInfo->mLevel <= 40) || mPlayerInfo->mLevel == 50;
}

int LawnApp::GetCurrentChallengeIndex()
{
    return static_cast<int>(mGameMode) - static_cast<int>(GAMEMODE_SURVIVAL_NORMAL_STAGE_1);
}

ChallengeDefinition& LawnApp::GetCurrentChallengeDef()
{
    return GetChallengeDefinition(GetCurrentChallengeIndex());
}

PottedPlant* LawnApp::GetPottedPlantByIndex(int thePottedPlantIndex)
{
    TOD_ASSERT(thePottedPlantIndex >= 0 && thePottedPlantIndex < mPlayerInfo->mNumPottedPlants);
    return &mPlayerInfo->mPottedPlant[thePottedPlantIndex];
}

bool LawnApp::UpdateAppStep(bool* updated)
{
    if (mCloseRequest)
    {
        Shutdown();
        return false;
    }
    return SexyAppBase::UpdateAppStep(updated);
}

bool LawnApp::UpdateApp()
{
    if (mCloseRequest)
    {
        Shutdown();
        return false;
    }

    //if (mLoadingThreadCompleted)
    //{
    //  LoadingThreadCompleted();
    //}

    bool updated = SexyAppBase::UpdateApp();

    //if (mLoadingThreadCompleted && !mExitToTop)
    //{
    //  CheckForUpdates();
    //}

    return updated;
}

void LawnApp::CloseRequestAsync()
{
    //mDeferredMessages.clear();
    mExitToTop = true;
    mCloseRequest = true;
}

SeedType LawnApp::GetAwardSeedForLevel(int theLevel)
{
    int aArea = (theLevel - 1) / LEVELS_PER_AREA + 1;
    int aSub = (theLevel - 1) % LEVELS_PER_AREA + 1;
    int aSeedsHasGot = (aArea - 1) * 8 + aSub;  // 一般来说，每大关可以获得 8 种植物，每小关可以获得 1 种植物
    if (aSub >= 10)
    {
        aSeedsHasGot -= 2;  // 到达第 10 小关时，本大关中有 2 小关的奖励不是新植物
    }
    else if (aSub >= 5)
    {
        aSeedsHasGot -= 1;  // 到达第 5 小关时，本大关中有 1 小关的奖励不是新植物
    }
    if (aSeedsHasGot > 40)
    {
        aSeedsHasGot = 40;
    }

    return (SeedType)aSeedsHasGot;
}

int LawnApp::GetSeedsAvailable()
{
    int aLevel = mPlayerInfo->GetLevel();
    if (HasFinishedAdventure() || aLevel > 50)
    {
        return 49;
    }

    SeedType aSeedTypeMax = GetAwardSeedForLevel(aLevel);
    return MIN(NUM_SEEDS_IN_CHOOSER, aSeedTypeMax);
}

// GOTY @Patoke: 0x456FE0
bool LawnApp::HasSeedType(SeedType theSeedType)
{
    if (IsTrialStageLocked() && theSeedType >= SEED_JALAPENO)
        return false;

    /* 优化
    if (theSeedType >= SEED_GATLINGPEA && theSeedType <= SEED_IMITATER)
        return mPlayerInfo->mPurchases[theSeedType - SEED_GATLINGPEA];
    */

    switch (theSeedType)
    {
    case SEED_GATLINGPEA:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_GATLINGPEA] > 0;
    case SEED_TWINSUNFLOWER:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_TWINSUNFLOWER] > 0;
    case SEED_GLOOMSHROOM:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_GLOOMSHROOM] > 0;
    case SEED_CATTAIL:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_CATTAIL] > 0;
    case SEED_WINTERMELON:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_WINTERMELON] > 0;
    case SEED_GOLD_MAGNET:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_GOLD_MAGNET] > 0;
    case SEED_SPIKEROCK:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_SPIKEROCK] > 0;
    case SEED_COBCANNON:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_COBCANNON] > 0;
    case SEED_IMITATER:
        return mPlayerInfo->mPurchases[STORE_ITEM_PLANT_IMITATER] > 0;
    default:
        return theSeedType < GetSeedsAvailable();
    }
}

Reanimation* LawnApp::AddReanimation(float theX, float theY, int theRenderOrder, int theReanimationType)
{
    return mEffectSystem->mReanimationHolder->AllocReanimation(theX, theY, theRenderOrder, theReanimationType);
}

TodParticleSystem* LawnApp::AddTodParticle(float theX, float theY, int theRenderOrder, int theEffect)
{
    return mEffectSystem->mParticleHolder->AllocParticleSystem(theX, theY, theRenderOrder, theEffect);
}

ParticleSystemID LawnApp::ParticleGetID(TodParticleSystem* theParticle)
{
    return (ParticleSystemID)mEffectSystem->mParticleHolder->mParticleSystems.DataArrayGetID(theParticle);
}

ReanimationID LawnApp::ReanimationGetID(Reanimation* theReanimation)
{
    return static_cast<ReanimationID>(mEffectSystem->mReanimationHolder->mReanimations.DataArrayGetID(theReanimation));
}

TodParticleSystem* LawnApp::ParticleGet(ParticleSystemID theParticleID)
{
    return mEffectSystem->mParticleHolder->mParticleSystems.DataArrayGet(static_cast<unsigned int>(theParticleID));
}

TodParticleSystem* LawnApp::ParticleTryToGet(ParticleSystemID theParticleID)
{
    return mEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(static_cast<unsigned int>(theParticleID));
}

// GOTY @Patoke: 0x464B0F
Reanimation* LawnApp::ReanimationGet(ReanimationID theReanimationID)
{
    return mEffectSystem->mReanimationHolder->mReanimations.DataArrayGet(static_cast<unsigned int>(theReanimationID));
}

Reanimation* LawnApp::ReanimationTryToGet(ReanimationID theReanimationID)
{
    return mEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(static_cast<unsigned int>(theReanimationID));
}

void LawnApp::RemoveReanimation(ReanimationID theReanimationID)
{
    Reanimation* aReanim = ReanimationTryToGet(theReanimationID);
    if (aReanim)
    {
        aReanim->ReanimationDie();
    }
}

void LawnApp::RemoveParticle(ParticleSystemID theParticleID)
{
    TodParticleSystem* aParticle = ParticleTryToGet(theParticleID);
    if (aParticle)
    {
        aParticle->ParticleSystemDie();
    }
}

bool LawnApp::AdvanceCrazyDaveText()
{
    std::string aMessageName = StrFormat("[CRAZY_DAVE_%d]", mCrazyDaveMessageIndex + 1);
    if (!TodStringListExists(aMessageName))
    {
        return false;
    }

    CrazyDaveTalkIndex(mCrazyDaveMessageIndex + 1);
    return true;
}

std::string LawnApp::GetCrazyDaveText(int theMessageIndex)
{
    std::string aMessage = StrFormat("[CRAZY_DAVE_%d]", theMessageIndex);
    aMessage = TodReplaceString(aMessage, "{PLAYER_NAME}", mPlayerInfo->mName);
    aMessage = TodReplaceString(aMessage, "{MONEY}", GetMoneyString(mPlayerInfo->mCoins));
    int aCost = StoreScreen::GetItemCost(STORE_ITEM_PACKET_UPGRADE);
    aMessage = TodReplaceString(aMessage, "{UPGRADE_COST}", GetMoneyString(aCost));
    return aMessage;
}

bool LawnApp::CanShowAlmanac()
{
    if (IsIceDemo())
        return false;

    if (mPlayerInfo == NULL)
        return false;

    return HasFinishedAdventure() || mPlayerInfo->mLevel >= 15;
}

bool LawnApp::CanShowStore()
{
    if (IsIceDemo())
        return false;

    if (mPlayerInfo == NULL)
        return false;

    return HasFinishedAdventure() || mPlayerInfo->mHasSeenUpsell || mPlayerInfo->mLevel >= 25;
}

bool LawnApp::CanShowZenGarden()
{
    if (mPlayerInfo == NULL)
        return false;

    if (IsTrialStageLocked())
        return false;

    return HasFinishedAdventure() || mPlayerInfo->mLevel >= 45;
}

bool LawnApp::CanSpawnYetis()
{
    const ZombieDefinition& aZombieDef = GetZombieDefinition(ZOMBIE_YETI);
    return HasFinishedAdventure() && (mPlayerInfo->mFinishedAdventure >= 2 || mPlayerInfo->mLevel >= aZombieDef.mStartingLevel);
}

bool LawnApp::HasBeatenChallenge(GameMode theGameMode)
{
    if (mPlayerInfo == NULL)
        return false;

    int aChallengeIndex = theGameMode - GAMEMODE_SURVIVAL_NORMAL_STAGE_1;
    TOD_ASSERT(aChallengeIndex >= 0 && aChallengeIndex < NUM_CHALLENGE_MODES);
    if (IsSurvivalNormal(theGameMode))
    {
        return mPlayerInfo->mChallengeRecords[aChallengeIndex] >= SURVIVAL_NORMAL_FLAGS;
    }
    if (IsSurvivalHard(theGameMode))
    {
        return mPlayerInfo->mChallengeRecords[aChallengeIndex] >= SURVIVAL_HARD_FLAGS;
    }
    if (IsSurvivalEndless(theGameMode) || IsEndlessScaryPotter(theGameMode) || IsEndlessIZombie(theGameMode))
    {
        return false;
    }
    return mPlayerInfo->mChallengeRecords[aChallengeIndex] > 0;
}

bool LawnApp::HasFinishedAdventure()
{
    return mPlayerInfo && mPlayerInfo->mFinishedAdventure > 0;
}

bool LawnApp::IsFirstTimeAdventureMode()
{
    return IsAdventureMode() && !HasFinishedAdventure();
}

void LawnApp::CrazyDaveEnter()
{
    TOD_ASSERT(mCrazyDaveState == CRAZY_DAVE_OFF);
    TOD_ASSERT(!ReanimationTryToGet(mCrazyDaveReanimID));

    Reanimation* aCrazyDaveReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_CRAZY_DAVE);
    aCrazyDaveReanim->mIsAttachment = true;
    aCrazyDaveReanim->SetBasePoseFromAnim("anim_idle_handing");
    mCrazyDaveReanimID = ReanimationGetID(aCrazyDaveReanim);
    aCrazyDaveReanim->PlayReanim("anim_enter", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);

    mCrazyDaveState = CRAZY_DAVE_ENTERING;
    mCrazyDaveMessageIndex = -1;
    mCrazyDaveMessageText.clear();
    mCrazyDaveBlinkCounter = RandRangeInt(400, 800);

    if (mGameScene == SCENE_LEVEL_INTRO && IsStormyNightLevel())
    {
        aCrazyDaveReanim->mColorOverride = Color(64, 64, 64);
    }
}

void LawnApp::CrazyDaveDie()
{
    Reanimation* aCrazyDaveReanim = ReanimationTryToGet(mCrazyDaveReanimID);
    if (aCrazyDaveReanim)
    {
        aCrazyDaveReanim->ReanimationDie();

        mCrazyDaveState = CRAZY_DAVE_OFF;
        mCrazyDaveReanimID = REANIMATIONID_NULL;
        mCrazyDaveMessageIndex = -1;
        mCrazyDaveMessageText.clear();

        CrazyDaveStopSound();
    }
}

void LawnApp::CrazyDaveLeave()
{
    Reanimation* aCrazyDaveReanim = ReanimationTryToGet(mCrazyDaveReanimID);
    if (aCrazyDaveReanim)
    {
        if (mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING || mCrazyDaveState == CRAZY_DAVE_HANDING_IDLING)
        {
            CrazyDaveDoneHanding();
        }

        aCrazyDaveReanim->PlayReanim("anim_leave", REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
        aCrazyDaveReanim->SetImageOverride("Dave_mouths", NULL);

        mCrazyDaveState = CRAZY_DAVE_LEAVING;
        mCrazyDaveMessageIndex = -1;
        mCrazyDaveMessageText.clear();

        CrazyDaveStopSound();
    }
}

void LawnApp::CrazyDaveTalkIndex(int theMessageIndex)
{
    mCrazyDaveMessageIndex = theMessageIndex;
    std::string aMessageText = GetCrazyDaveText(theMessageIndex);
    CrazyDaveTalkMessage(aMessageText);
}

void LawnApp::CrazyDaveDoneHanding()
{
    Reanimation* aCrazyDaveReanim = ReanimationGet(mCrazyDaveReanimID);
    ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
    AttachmentDie(aHandTrackInstance->mAttachmentID);

    TodTrace("DoneHanding");
}

void LawnApp::CrazyDaveStopSound()
{
    mSoundSystem->StopFoley(FOLEY_CRAZY_DAVE_SHORT);
    mSoundSystem->StopFoley(FOLEY_CRAZY_DAVE_LONG);
    mSoundSystem->StopFoley(FOLEY_CRAZY_DAVE_EXTRA_LONG);
    mSoundSystem->StopFoley(FOLEY_CRAZY_DAVE_CRAZY);
}

void LawnApp::CrazyDaveTalkMessage(const std::string& theMessage)
{
    Reanimation* aCrazyDaveReanim = ReanimationGet(mCrazyDaveReanimID);

    bool doHanding = false;
    if (strstr(theMessage.c_str(), "{HANDING}") != NULL)
    {
        doHanding = true;
    }
    if ((mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING || mCrazyDaveState == CRAZY_DAVE_HANDING_IDLING) && !doHanding)
    {
        CrazyDaveDoneHanding();
    }

    bool doSound = true;
    if (strstr(theMessage.c_str(), "{NO_SOUND}") != NULL)
    {
        doSound = false;
    }
    else
    {
        CrazyDaveStopSound();
    }

    int aWordsCount = 0;
    bool isControlWord = false;
    for (size_t i = 0; i < theMessage.size(); i++)
    {
        if (theMessage[i] == '{')
        {
            isControlWord = true;
        }
        else if (theMessage[i] == '}')
        {
            isControlWord = false;
        }
        else if (!isControlWord)
        {
            aWordsCount++;
        }
    }

    aCrazyDaveReanim->SetImageOverride("Dave_mouths", NULL);

    if (mCrazyDaveState != CRAZY_DAVE_TALKING || doSound)
    {
        if (doHanding)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            if (doSound)
            {
                if (strstr(theMessage.c_str(), "{SHORT_SOUND}") != NULL)
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_SHORT);
                }
                else if (strstr(theMessage.c_str(), "{SCREAM}") != NULL)
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_SCREAM);
                }
                else
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_LONG);
                }
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHAKE}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_crazy", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_CRAZY);
            }

            mCrazyDaveState = CRAZY_DAVE_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SCREAM}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_smalltalk", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_SCREAM);
            }

            mCrazyDaveState = CRAZY_DAVE_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SCREAM2}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_mediumtalk", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_SCREAM_2);
            }

            mCrazyDaveState = CRAZY_DAVE_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHOW_WALLNUT}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            Reanimation* aWallnutReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_WALLNUT);
            aWallnutReanim->PlayReanim("anim_idle", REANIM_LOOP, 0, 12.0f);
            TodTrace("Handed");

            ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
            AttachEffect* aAttachEffect = AttachReanim(aHandTrackInstance->mAttachmentID, aWallnutReanim, 100.0f, 393.0f);
            aAttachEffect->mOffset.m00 = 1.2f;
            aAttachEffect->mOffset.m11 = 1.2f;

            aCrazyDaveReanim->Update();

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_SCREAM_2);
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHOW_HAMMER}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            Reanimation* aHammerReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_HAMMER);
            aHammerReanim->PlayReanim("anim_whack_zombie", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
            aHammerReanim->mAnimTime = 1.0f;

            ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
            AttachEffect* aAttachEffect = AttachReanim(aHandTrackInstance->mAttachmentID, aHammerReanim, 62.0f, 445.0f);
            aAttachEffect->mOffset.m00 = 1.5f;
            aAttachEffect->mOffset.m11 = 1.5f;

            aCrazyDaveReanim->Update();

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_LONG);
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHOW_FERTILIZER}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            Reanimation* aFertilizerReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_ZENGARDEN_FERTILIZER);
            aFertilizerReanim->PlayReanim("bag", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
            aFertilizerReanim->mAnimRate = 0.0f;

            ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
            AttachReanim(aHandTrackInstance->mAttachmentID, aFertilizerReanim, 102.0f, 412.0f);
            aCrazyDaveReanim->Update();

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_LONG);
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHOW_TREE_FOOD}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            Reanimation* aTreeFoodReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_TREEOFWISDOM_TREEFOOD);
            aTreeFoodReanim->PlayReanim("bag", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
            aTreeFoodReanim->mAnimRate = 0.0f;

            ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
            AttachReanim(aHandTrackInstance->mAttachmentID, aTreeFoodReanim, 102.0f, 412.0f);
            aCrazyDaveReanim->Update();

            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_LONG);
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else if (strstr(theMessage.c_str(), "{SHOW_MONEYBAG}") != NULL)
        {
            aCrazyDaveReanim->PlayReanim("anim_talk_handing", REANIM_LOOP, 50, 12.0f);

            Reanimation* aMoneyBagReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_ZENGARDEN_FERTILIZER);
            aMoneyBagReanim->PlayReanim("bag", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
            aMoneyBagReanim->mAnimRate = 0.0f;
            aMoneyBagReanim->SetImageOverride("bag", IMAGE_MONEYBAG);

            ReanimatorTrackInstance* aHandTrackInstance = aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand");
            AttachReanim(aHandTrackInstance->mAttachmentID, aMoneyBagReanim, 90.0f, 405.0f);
            aCrazyDaveReanim->Update();
            /*
            v16 = Reanimation::GetTrackInstanceByName(v3, "Dave_handinghand");
            theAnimRate = 405.0;
            v17 = 90.0;
            */
            if (doSound)
            {
                PlayFoley(FOLEY_CRAZY_DAVE_LONG);
            }

            mCrazyDaveState = CRAZY_DAVE_HANDING_TALKING;
        }
        else
        {
            if (aWordsCount < 23)
            {
                aCrazyDaveReanim->PlayReanim("anim_smalltalk", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

                if (doSound)
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_SHORT);
                }

                mCrazyDaveState = CRAZY_DAVE_TALKING;
            }
            else if (aWordsCount < 52)
            {
                aCrazyDaveReanim->PlayReanim("anim_mediumtalk", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

                if (doSound)
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_LONG);
                }

                mCrazyDaveState = CRAZY_DAVE_TALKING;
            }
            else
            {
                aCrazyDaveReanim->PlayReanim("anim_blahblah", REANIM_PLAY_ONCE_AND_HOLD, 50, 12.0f);

                if (doSound)
                {
                    PlayFoley(FOLEY_CRAZY_DAVE_EXTRA_LONG);
                }

                mCrazyDaveState = CRAZY_DAVE_TALKING;
            }
        }
    }

    mCrazyDaveMessageText = theMessage;
}

void LawnApp::CrazyDaveStopTalking()
{
    bool aDoneHanding = true;
    if (mGameMode == GAMEMODE_UPSELL)
    {
        aDoneHanding = false;
    }
    if (aDoneHanding && mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING)
    {
        CrazyDaveDoneHanding();
    }

    Reanimation* aCrazyDaveReanim = ReanimationGet(mCrazyDaveReanimID);
    aCrazyDaveReanim->SetImageOverride("Dave_mouths", NULL);
    if (mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING && !aDoneHanding)
    {
        aCrazyDaveReanim->PlayReanim("anim_idle_handing", REANIM_LOOP, 20, 12.0f);
        mCrazyDaveState = CRAZY_DAVE_HANDING_IDLING;
    }
    else if (mCrazyDaveState == CRAZY_DAVE_TALKING || mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING)
    {
        aCrazyDaveReanim->PlayReanim("anim_idle", REANIM_LOOP, 20, 12.0f);
        mCrazyDaveState = CRAZY_DAVE_IDLING;
    }

    mCrazyDaveMessageIndex = -1;
    mCrazyDaveMessageText.clear();
    CrazyDaveStopSound();
}

void LawnApp::UpdateCrazyDave()
{
    Reanimation* aCrazyDaveReanim = ReanimationTryToGet(mCrazyDaveReanimID);
    if (aCrazyDaveReanim == NULL)
        return;

    if (mCrazyDaveState == CRAZY_DAVE_ENTERING || mCrazyDaveState == CRAZY_DAVE_TALKING)
    {
        if (aCrazyDaveReanim->mLoopCount > 0)
        {
            aCrazyDaveReanim->PlayReanim("anim_idle", REANIM_LOOP, 20, 12.0f);
            mCrazyDaveState = CRAZY_DAVE_IDLING;
        }
    }
    else if (mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING)
    {
        if (aCrazyDaveReanim->mLoopCount > 0)
        {
            aCrazyDaveReanim->PlayReanim("anim_idle_handing", REANIM_LOOP, 20, 12.0f);
            mCrazyDaveState = CRAZY_DAVE_HANDING_IDLING;
        }
    }
    else if (mCrazyDaveState == CRAZY_DAVE_LEAVING && aCrazyDaveReanim->mLoopCount > 0)
    {
        CrazyDaveDie();
    }

    if (mCrazyDaveState == CRAZY_DAVE_IDLING || mCrazyDaveState == CRAZY_DAVE_HANDING_IDLING)
    {
        if (strstr(mCrazyDaveMessageText.c_str(), "{MOUTH_BIG_SMILE}") != NULL)
        {
            aCrazyDaveReanim->SetImageOverride("Dave_mouths", IMAGE_REANIM_CRAZYDAVE_MOUTH1);
        }
        else if (strstr(mCrazyDaveMessageText.c_str(), "{MOUTH_SMALL_SMILE}") != NULL)
        {
            aCrazyDaveReanim->SetImageOverride("Dave_mouths", IMAGE_REANIM_CRAZYDAVE_MOUTH5);
        }
        else if (strstr(mCrazyDaveMessageText.c_str(), "{MOUTH_BIG_OH}") != NULL)
        {
            aCrazyDaveReanim->SetImageOverride("Dave_mouths", IMAGE_REANIM_CRAZYDAVE_MOUTH4);
        }
        else if (strstr(mCrazyDaveMessageText.c_str(), "{MOUTH_SMALL_OH}") != NULL)
        {
            aCrazyDaveReanim->SetImageOverride("Dave_mouths", IMAGE_REANIM_CRAZYDAVE_MOUTH6);
        }
    }

    if (mCrazyDaveState == CRAZY_DAVE_IDLING || mCrazyDaveState == CRAZY_DAVE_TALKING ||
        mCrazyDaveState == CRAZY_DAVE_HANDING_TALKING || mCrazyDaveState == CRAZY_DAVE_HANDING_IDLING)
    {
        mCrazyDaveBlinkCounter--;
        if (mCrazyDaveBlinkCounter <= 0)
        {
            mCrazyDaveBlinkCounter = RandRangeInt(400, 800);
            Reanimation* aBlinkReanim = AddReanimation(0.0f, 0.0f, 0, REANIM_CRAZY_DAVE);
            aBlinkReanim->SetFramesForLayer("anim_blink");
            aBlinkReanim->mLoopType = REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD;
            aBlinkReanim->mAnimRate = 15.0f;
            aBlinkReanim->AttachToAnotherReanimation(aCrazyDaveReanim, "Dave_head");
            aBlinkReanim->mColorOverride = aCrazyDaveReanim->mColorOverride;
            aCrazyDaveReanim->AssignRenderGroupToTrack("Dave_eye", RENDER_GROUP_HIDDEN);
            mCrazyDaveBlinkReanimID = ReanimationGetID(aBlinkReanim);
        }
    }

    Reanimation* aBlinkReanim = ReanimationTryToGet(mCrazyDaveBlinkReanimID);
    if (aBlinkReanim && aBlinkReanim->mLoopCount > 0)
    {
        aCrazyDaveReanim->AssignRenderGroupToTrack("Dave_eye", RENDER_GROUP_NORMAL);
        RemoveReanimation(mCrazyDaveBlinkReanimID);
        mCrazyDaveBlinkReanimID = REANIMATIONID_NULL;
    }

    aCrazyDaveReanim->Update();
}

void LawnApp::DrawCrazyDave(Graphics* g)
{
    Reanimation* aCrazyDaveReanim = ReanimationTryToGet(mCrazyDaveReanimID);
    if (aCrazyDaveReanim == NULL)
        return;

    if (mCrazyDaveMessageText.size())
    {
        Image* aBubbleImage = IMAGE_STORE_SPEECHBUBBLE2;
        int aPosX = 285;
        int aPosY = 20;
        if (GetDialog(DIALOG_STORE))
        {
            aBubbleImage = IMAGE_STORE_SPEECHBUBBLE;
            aPosX -= 180;
            aPosY -= 78;
        }
        else if (mGameMode == GAMEMODE_UPSELL)
        {
            aPosX += 130;
            aPosY += 70;
        }
        g->DrawImage(aBubbleImage, aPosX, aPosY);

        std::string aBubbleText = mCrazyDaveMessageText;
        Rect aRect(aPosX + 25, aPosY + 6, 233, 144);
        if (strstr(aBubbleText.c_str(), "{SHAKE}") != NULL)
        {
            aBubbleText = TodReplaceString(aBubbleText, "{SHAKE}", "");
            aRect.mX += rand() % 2;
            aRect.mY += rand() % 2;
        }

        bool clickToContinue = true;
        if (mGameMode == GAMEMODE_UPSELL)
        {
            clickToContinue = false;
        }
        else if (strstr(aBubbleText.c_str(), "{NO_CLICK}") != NULL)
        {
            aBubbleText = TodReplaceString(aBubbleText, "{NO_CLICK}", "");
            clickToContinue = false;
        }

        DrawStringJustification aWrapEnum = static_cast<DrawStringJustification>(GetInteger("CRAZY_DAVE_MESSAGE_TEXT_WRAP_ENUM", DS_ALIGN_CENTER_VERTICAL_MIDDLE));
        TodDrawStringWrapped(g, aBubbleText, aRect, FONT_BRIANNETOD16, Color::Black, aWrapEnum);
        if (clickToContinue)
        {
            TodDrawString(g, "click to continue", aPosX + 139, aPosY + 140, FONT_PICO129, Color::Black, DS_ALIGN_CENTER);
        }
    }

    aCrazyDaveReanim->Draw(g);
}

int LawnApp::GetNumPreloadingTasks()
{
#ifdef LOW_MEMORY
    return 0;
#endif

    int aTaskCount = 10;
    if (mPlayerInfo)
    {
        for (SeedType i = SEED_PEASHOOTER; i < NUM_SEED_TYPES; i = static_cast<SeedType>(static_cast<int>(i) + 1))
        {
            if (HasSeedType(i) || HasFinishedAdventure())
            {
                aTaskCount++;
            }
        }

        for (ZombieType i = ZOMBIE_NORMAL; i < NUM_ZOMBIE_TYPES; i = static_cast<ZombieType>(static_cast<int>(i) + 1))
        {
            if (HasFinishedAdventure() || mPlayerInfo->mLevel >= GetZombieDefinition(i).mStartingLevel)
            {
                if (i != ZOMBIE_BOSS &&
                    i != ZOMBIE_CATAPULT &&
                    i != ZOMBIE_GARGANTUAR &&
                    i != ZOMBIE_DIGGER &&
                    i != ZOMBIE_ZAMBONI)
                {
                    aTaskCount++;
                }
            }
        }
    }
    return aTaskCount * 68;
}

void LawnApp::PreloadForUser()
{
    int aNumTasks = mNumLoadingThreadTasks + GetNumPreloadingTasks();
    if (mTitleScreen && mTitleScreen->mQuickLoadKey != KEYCODE_UNKNOWN)
    {
        TodTrace("preload canceled\n");
        mNumLoadingThreadTasks = aNumTasks;
        return;
    }

#ifndef LOW_MEMORY
    ReanimatorEnsureDefinitionLoaded(REANIM_PUFF, true);
    ReanimatorEnsureDefinitionLoaded(REANIM_LAWN_MOWERED_ZOMBIE, true);
    ReanimatorEnsureDefinitionLoaded(REANIM_READYSETPLANT, true);
    mCompletedLoadingThreadTasks += 68;
    ReanimatorEnsureDefinitionLoaded(REANIM_FINAL_WAVE, true);
    ReanimatorEnsureDefinitionLoaded(REANIM_SUN, true);
    ReanimatorEnsureDefinitionLoaded(REANIM_TEXT_FADE_ON, true);
    mCompletedLoadingThreadTasks += 68;
    ReanimatorEnsureDefinitionLoaded(REANIM_ZOMBIE, true);
    mCompletedLoadingThreadTasks += 68;
    ReanimatorEnsureDefinitionLoaded(REANIM_ZOMBIE_NEWSPAPER, true);
    mCompletedLoadingThreadTasks += 68;
    ReanimatorEnsureDefinitionLoaded(REANIM_SELECTOR_SCREEN, true);
    mCompletedLoadingThreadTasks += 340;
    ReanimatorEnsureDefinitionLoaded(REANIM_ZOMBIE_HAND, true);
    mCompletedLoadingThreadTasks += 68;

    if (mPlayerInfo)
    {
        for (SeedType i = SEED_PEASHOOTER; i < NUM_SEED_TYPES; i = static_cast<SeedType>(static_cast<int>(i) + 1))
        {
            if (HasSeedType(i) || HasFinishedAdventure())
            {
                Plant::PreloadPlantResources(i);
                if (mCompletedLoadingThreadTasks < aNumTasks)
                {
                    mCompletedLoadingThreadTasks += 68;
                }

                if (mTitleScreen && mTitleScreen->mQuickLoadKey != KEYCODE_UNKNOWN)
                {
                    TodTrace("preload canceled\n");
                    mNumLoadingThreadTasks = aNumTasks;
                    return;
                }

                if (mShutdown || mCloseRequest)
                {
                    return;
                }
            }
        }

        for (ZombieType i = ZOMBIE_NORMAL; i < NUM_ZOMBIE_TYPES; i = static_cast<ZombieType>(static_cast<int>(i) + 1))
        {
            if (HasFinishedAdventure() || mPlayerInfo->mLevel >= GetZombieDefinition(i).mStartingLevel)
            {
                continue;
            }
            if (i == ZOMBIE_BOSS || i == ZOMBIE_CATAPULT || i == ZOMBIE_GARGANTUAR ||
                i == ZOMBIE_DIGGER || i == ZOMBIE_ZAMBONI)
            {
                continue;
            }

            Zombie::PreloadZombieResources(i);
            if (mCompletedLoadingThreadTasks < aNumTasks)
            {
                mCompletedLoadingThreadTasks += 68;
            }

            if (mTitleScreen && mTitleScreen->mQuickLoadKey != KEYCODE_UNKNOWN)
            {
                TodTrace("preload canceled\n");
                mNumLoadingThreadTasks = aNumTasks;
                return;
            }

            if (mShutdown || mCloseRequest)
            {
                return;
            }
        }
    }
#endif

    if (mCompletedLoadingThreadTasks != aNumTasks)
    {
        TodTrace("num preload tasks wasn't calculated correctly");
        mCompletedLoadingThreadTasks = aNumTasks;
    }
}

std::string LawnApp::Pluralize(int theCount, const char* theSingular, const char* thePlural)
{
    if (theCount == 1)
    {
        return TodReplaceNumberString(theSingular, "{COUNT}", theCount);
    }

    return TodReplaceNumberString(thePlural, "{COUNT}", theCount);
}

int LawnApp::GetNumTrophies(ChallengePage thePage)
{
    int aNumTrophies = 0;

    for (int i = 0; i < NUM_CHALLENGE_MODES; i++)
    {
        const ChallengeDefinition& aDef = GetChallengeDefinition(i);
        if (aDef.mPage == thePage && HasBeatenChallenge(aDef.mChallengeMode))
        {
            aNumTrophies++;
        }
    }

    return aNumTrophies;
}

int LawnApp::TrophiesNeedForGoldSunflower()
{
    return 48 - GetNumTrophies(CHALLENGE_PAGE_SURVIVAL) - GetNumTrophies(CHALLENGE_PAGE_CHALLENGE) - GetNumTrophies(CHALLENGE_PAGE_PUZZLE);
}

// GOTY @Patoke: 0x459190
bool LawnApp::EarnedGoldTrophy()
{
    return HasFinishedAdventure() && TrophiesNeedForGoldSunflower() <= 0;
}

void LawnApp::FinishZenGardenToturial()
{
    mBoardResult = BOARDRESULT_WON;
    KillBoard();
    PreNewGame(GAMEMODE_ADVENTURE, false);
}

bool LawnApp::IsTrialStageLocked()
{
    if (mDebugTrialLocked)
        return true;

    return mTrialType == TRIALTYPE_STAGELOCKED;
}

void LawnApp::InitHook()
{
    mTrialType = TRIALTYPE_NONE;
}

std::string LawnApp::GetMoneyString(int theAmount)
{
    int aValue = theAmount * 10;
    if (aValue > 999999)
    {
        return StrFormat("$%d,%03d,%03d", aValue / 1000000, (aValue - aValue / 1000000 * 1000000) / 1000, aValue - aValue / 1000 * 1000);
    }
    else if (aValue > 9999)
    {
        return StrFormat("$%d,%03d", aValue / 1000, aValue - aValue / 1000 * 1000);
    }
    else
    {
        return StrFormat("$%d", aValue);
    }
}

std::string LawnGetCurrentLevelName()
{
    if (gLawnApp == NULL)
    {
        return "Before App";
    }
    if (gLawnApp->mGameScene == SCENE_LOADING)
    {
        return "Game Loading";
    }
    if (gLawnApp->mGameScene == SCENE_MENU)
    {
        return "Game Selector";
    }
    if (gLawnApp->mGameScene == SCENE_AWARD)
    {
        return "Award Screen";
    }
    if (gLawnApp->mGameScene == SCENE_CHALLENGE)
    {
        return "Challenge Screen";
    }
    if (gLawnApp->mGameScene == SCENE_CREDIT)
    {
        return "Credits";
    }
    if (gLawnApp->mBoard == NULL)
    {
        return "Not Playing";
    }

    if (gLawnApp->IsFirstTimeAdventureMode())
    {
        return gLawnApp->GetStageString(gLawnApp->mBoard->mLevel);
    }
    if (gLawnApp->IsAdventureMode())
    {
        return StrFormat("F%d", gLawnApp->GetStageString(gLawnApp->mBoard->mLevel).c_str());
    }

    return gLawnApp->GetCurrentChallengeDef().mChallengeName;
}

bool LawnApp::CanDoPinataMode()
{
    if (mPlayerInfo == NULL)
        return false;

    return mPlayerInfo->mChallengeRecords[static_cast<int>(GAMEMODE_TREE_OF_WISDOM) - static_cast<int>(GAMEMODE_SURVIVAL_NORMAL_STAGE_1)] >= 1000;
}

bool LawnApp::CanDoDanceMode()
{
    if (mPlayerInfo == NULL)
        return false;

    return mPlayerInfo->mChallengeRecords[static_cast<int>(GAMEMODE_TREE_OF_WISDOM) - static_cast<int>(GAMEMODE_SURVIVAL_NORMAL_STAGE_1)] >= 500;
}

bool LawnApp::CanDoDaisyMode()
{
    if (mPlayerInfo == NULL)
        return false;

    return mPlayerInfo->mChallengeRecords[static_cast<int>(GAMEMODE_TREE_OF_WISDOM) - static_cast<int>(GAMEMODE_SURVIVAL_NORMAL_STAGE_1)] >= 100;
}

void LawnApp::PlaySample(intptr_t theSoundNum)
{
    if (!mMuteSoundsForCutscene)
    {
        SexyAppBase::PlaySample(theSoundNum);
    }
}

void LawnApp::SwitchScreenMode(bool wantWindowed, bool is3d, bool force)
{
    SexyAppBase::SwitchScreenMode(wantWindowed, is3d, force);

    NewOptionsDialog* aNewOptionsDialog = (NewOptionsDialog*)GetDialog(DIALOG_NEWOPTIONS);
    if (aNewOptionsDialog)
    {
        aNewOptionsDialog->mFullscreenCheckbox->SetChecked(!mIsWindowed);
    }
}

/* #################################################################################################### */
/*
void LawnApp::BetaSubmit(bool theAskForComments)
{

}

void LawnApp::BetaRecordLevelStats()
{

}

void LawnApp::BetaAddFile(std::list<std::string>& theUploadFileList, std::string theFileName, std::string theShortName)
{

}

void LawnApp::TraceLoadGroup(const char* theGroupName, int theGroupTime, int theTotalGroupWeigth, int theTaskWeight)
{

}
*/

/* #################################################################################################### */

void LawnApp::DoHighScoreDialog()
{

}

void LawnApp::DoRegister()
{

}

void LawnApp::DoRegisterError()
{

}

bool LawnApp::CanDoRegisterDialog()
{
    return false;
}

void LawnApp::DoNeedRegisterDialog()
{

}

void LawnApp::FinishModelessDialogs()
{

}

bool LawnApp::NeedRegister()
{
    return false;
}

void LawnApp::UpdateRegisterInfo()
{

}