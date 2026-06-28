/*
 * GameSelector.cpp -- M4 #1 main-menu implementation.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/GameSelector.cpp (1509 lines),
 * heavily adapted to this port's infrastructure. The upstream version uses
 * Reanimation / TodParticleSystem / NewLawnButton / TodStringTranslate --
 * none of which are ported yet (M5 territory). This implementation:
 *
 *   - Creates 10 GameButton children in the constructor and adds them to
 *     mApp->mWidgetManager directly (NOT as container children -- port's
 *     WidgetContainer::AddWidget does NOT call AddedToManager, and port's
 *     Widget::Draw does NOT recurse into a child container. Top-level
 *     widgets in the manager get drawn & hit-tested automatically.)
 *   - Lays them out in a 4-row grid sized to the 400x300 logical canvas.
 *   - Draws IMAGE_TITLESCREEN as the background (the only IMAGE_* global
 *     reliably loaded at boot per M3 work). Once IMAGE_BACKGROUND1 tiles
 *     properly (M4 #3) that can replace it.
 *   - Routes button clicks via ButtonDepress -> LawnApp flow methods
 *     (PreNewGame / ShowChallengeScreen / ShowStoreScreen / DoNewOptions /
 *     ConfirmQuit / DoAlmanacDialog / etc.).
 *   - Logs each button press to C:\Data\PvZ\gs_log.txt for on-device
 *     debugging (same pattern as the M3 handoff code).
 *
 * What's NOT here (left as M5+ TODOs):
 *   - Reanimation cloud/flower/leaf/hand decoration (Reanimator not ported)
 *   - ParticleSystem trophy sparkle (TodParticle not ported)
 *   - Profile dropdown / user switching (ProfileMgr is a no-op stub here)
 *   - Slide-in animation (mSlideCounter wired but no animation thread)
 *   - Zombatar / Achievements / QuickPlay buttons (sub-screens not ported)
 */
#include "GameSelector.h"

#include "GameButton.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../ConstEnums.h"
#include "../../engine/ResourceManager.h"
#include "../../engine/Graphics.h"
#include "../../engine/GLInterface.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"
#include "../../engine/WidgetManager.h"
#include "../../engine/SexyAppBase.h"
#include "../../engine/Color.h"
#include "../../engine/Rect.h"
#include "../../engine/Font.h"   // for Font::StringWidth/GetAscent (forward decl in Graphics.h is not enough)
#include "../../engine/SystemFont.h"  // for title + button label text
#include "ToolTipWidget.h"

#include <e32std.h>
#include <f32file.h>
#include <string.h>

namespace Sexy {

// -------------------------------------------------------------------------
// GSLog -- on-device diagnostic logger (same pattern as M3 handoff).
// -------------------------------------------------------------------------
static void GSLog(const TDesC8& aMsg)
{
    RFs fs; RFile f;
    if (fs.Connect() == KErrNone)
    {
        fs.MkDirAll(_L("C:\\Data\\PvZ"));
        // APPEND instead of Replace — so we can see ALL log lines,
        // not just the last one (which was overwriting previous clicks).
        TInt err = f.Open(fs, _L("C:\\Data\\PvZ\\gs_log.txt"),
                          EFileWrite | EFileShareAny);
        if (err == KErrNotFound)
            err = f.Create(fs, _L("C:\\Data\\PvZ\\gs_log.txt"),
                           EFileWrite | EFileShareAny);
        if (err == KErrNone)
        {
            TInt pos = 0; f.Seek(ESeekEnd, pos);
            f.Write(aMsg);
            f.Flush();
            f.Close();
        }
        fs.Close();
    }
}

// -------------------------------------------------------------------------
// Construction -- creates all buttons and adds them to mApp->mWidgetManager.
// LawnApp::ShowGameSelector must be called AFTER mWidgetManager exists
// (it is -- PvZAppUi::ConstructL creates mWidgetManager before Init()).
// -------------------------------------------------------------------------
GameSelector::GameSelector(LawnApp* theApp)
    : Widget()
    , mAdventureButton(NULL)
    , mMinigameButton(NULL)
    , mPuzzleButton(NULL)
    , mSurvivalButton(NULL)
    , mOptionsButton(NULL)
    , mQuitButton(NULL)
    , mHelpButton(NULL)
    , mStoreButton(NULL)
    , mAlmanacButton(NULL)
    , mZenGardenButton(NULL)
    , mChangeUserButton(NULL)
    , mToolTip(NULL)
    , mApp(theApp)
    , mStartingGame(false)
    , mStartingGameCounter(0)
    , mMinigamesLocked(true)
    , mPuzzleLocked(true)
    , mSurvivalLocked(true)
    , mShowStartButton(false)
    , mLoading(false)
    , mHasTrophy(false)
    , mUnlockSelectorCheat(false)
    , mLevel(0)
    , mSlideCounter(0)
    , mStartX(0), mStartY(0)
    , mDestX(0),  mDestY(0)
{
    mVisible = true;
    mMouseVisible = false;   // [M4 #1 fix] don't intercept clicks -- let them
                             // reach our child buttons (top-level in WidgetManager).
                             // FindWidget now skips widgets with mMouseVisible=false.
    mHasTransparencies = true;
    mDoFinger = false;
    mWantsFocus = true;
    mReanimLoaded = false;

    WidgetManager* wm = mApp ? mApp->mWidgetManager : NULL;
    if (!wm)
    {
        GSLog(_L8("GS:ctor mApp or mWidgetManager NULL -- buttons not created\n"));
        return;
    }

    // Helper macro: allocate, configure, add to manager.
    #define MAKE_BUTTON(field, id, label) \
        do { \
            field = new GameButton(id, this); \
            field->mApp = mApp; \
            field->SetLabel(label); \
            field->mDrawStoneButton = true; \
            field->Resize(0, 0, 120, 32); \
            wm->AddWidget(field); \
        } while (0)

    MAKE_BUTTON(mAdventureButton,  GameSelector_Adventure, "Adventure");
    MAKE_BUTTON(mSurvivalButton,   GameSelector_Survival,  "Survival");
    MAKE_BUTTON(mMinigameButton,   GameSelector_Minigame,  "Minigames");
    MAKE_BUTTON(mPuzzleButton,     GameSelector_Puzzle,    "Puzzles");
    MAKE_BUTTON(mStoreButton,      GameSelector_Store,     "Store");
    MAKE_BUTTON(mAlmanacButton,    GameSelector_Almanac,   "Almanac");
    MAKE_BUTTON(mZenGardenButton,  GameSelector_ZenGarden, "Zen Garden");
    MAKE_BUTTON(mOptionsButton,    GameSelector_Options,   "Options");
    MAKE_BUTTON(mHelpButton,       GameSelector_Help,      "Help");
    MAKE_BUTTON(mQuitButton,       GameSelector_Quit,      "Quit");
    #undef MAKE_BUTTON

    mToolTip = new ToolTipWidget();
    mToolTip->Resize(0, 270, 400, 24);
    wm->AddWidget(mToolTip);

    GSLog(_L8("GS:ctor buttons created and added to WidgetManager\n"));

    // [M4 reanim] Load SelectorScreen.reanim (XML version) for 1:1 menu.
    // Uses XML instead of .compiled because the compiled binary format has
    // architecture-dependent struct sizes (64-bit pointers from PC build
    // are incompatible with N95's 32-bit ARM). The XML is 328KB but
    // architecture-independent.
    mReanimLoaded = ReanimLoadCompiled(
        "reanim/SelectorScreen.reanim", mReanimDef);
    if (mReanimLoaded)
    {
        TBuf8<80> b;
        b.Format(_L8("GS:reanim loaded: %d tracks, FPS=%.1f\n"),
                 mReanimDef.mTrackCount, mReanimDef.mFPS);
        GSLog(b);
        // [Stage-1 diagnostic] Dump EVERY track name + transform count so the
        // next session can map reanim tracks to menu buttons (Adventure /
        // Survival / Options / Quit / ...). The upstream GameSelector positions
        // buttons by reading each track's transform via FindTrackIndex, so
        // knowing the exact names is the prerequisite for 1:1 button hit-zones.
        // (Previously only the first 5 were logged.)
        for (int i = 0; i < mReanimDef.mTrackCount; i++)
        {
            TBuf8<160> tb;
            const char* nm = mReanimDef.mTracks[i].mName
                             ? mReanimDef.mTracks[i].mName : "(null)";
            tb.Format(_L8("  track[%d]: '%s' xforms=%d\n"), i,
                      (const TUint8*)nm,
                      mReanimDef.mTracks[i].mTransformCount);
            GSLog(tb);
        }

        // [Stage-1] Wire the ReanimPlayer runtime so the menu animates from
        // the parsed tracks instead of rendering a static frame-0 snapshot.
        // The SelectorScreen reanim loops (clouds drift, flowers sway), so
        // play it on loop at native rate. mCoordScale defaults to 0.5
        // (800x600 reanim space -> 400x300 canvas).
        mReanimPlayer.SetDefinition(&mReanimDef);
        mReanimPlayer.mLoopType = ReanimPlayer::LOOP_ON;
        mReanimPlayer.mAnimRate = 1.0f;
        TBuf8<96> pb;
        pb.Format(_L8("GS:ReanimPlayer bound, duration=%.2fs\n"),
                  mReanimPlayer.GetDuration());
        GSLog(pb);
    }
    else
    {
        GSLog(_L8("GS:reanim FAILED — falling back to IMAGE_BACKGROUND1 menu\n"));
    }
}

GameSelector::~GameSelector()
{
    WidgetManager* wm = mApp ? mApp->mWidgetManager : NULL;
    // Remove from manager first (so it doesn't dangling-ref), then delete.
    #define TEARDOWN(field) \
        do { \
            if (field) { \
                if (wm) wm->RemoveWidget(field); \
                delete field; \
                field = NULL; \
            } \
        } while (0)
    TEARDOWN(mAdventureButton);
    TEARDOWN(mSurvivalButton);
    TEARDOWN(mMinigameButton);
    TEARDOWN(mPuzzleButton);
    TEARDOWN(mStoreButton);
    TEARDOWN(mAlmanacButton);
    TEARDOWN(mZenGardenButton);
    TEARDOWN(mOptionsButton);
    TEARDOWN(mHelpButton);
    TEARDOWN(mQuitButton);
    TEARDOWN(mChangeUserButton);
    TEARDOWN(mToolTip);
    #undef TEARDOWN
}

// -------------------------------------------------------------------------
// LayoutButtons -- position the 10 buttons in a 4-row grid on the 400x300
// canvas. Sizes are picked to fit the N95's 320x240 landscape viewport
// (the GL ortho is 400x300, letterboxed to 320x240). All buttons are
// 120x32; rows are 40px tall with 8px gaps.
//
//   Row 0 (y=70):   [  Adventure (full width 280)  ]   -- centered
//   Row 1 (y=120):  [Survival] [Minigames] [Puzzles]
//   Row 2 (y=160):  [Store]    [Almanac]   [Zen Garden]
//   Row 3 (y=200):  [Options]  [Help]      [Quit]
// -------------------------------------------------------------------------
void GameSelector::LayoutButtons()
{
    if (mAdventureButton)
        mAdventureButton->Resize((mWidth - 280) / 2, 70, 280, 32);

    const int colX[3] = { 12, 12 + 120 + 8, 12 + 2 * (120 + 8) };
    const int rowY[3] = { 120, 160, 200 };

    struct Btn { GameButton* btn; int col; int row; };
    Btn btns[] = {
        { mSurvivalButton,  0, 0 },
        { mMinigameButton,  1, 0 },
        { mPuzzleButton,    2, 0 },
        { mStoreButton,     0, 1 },
        { mAlmanacButton,   1, 1 },
        { mZenGardenButton, 2, 1 },
        { mOptionsButton,   0, 2 },
        { mHelpButton,      1, 2 },
        { mQuitButton,      2, 2 },
    };
    for (unsigned i = 0; i < sizeof(btns)/sizeof(btns[0]); i++)
    {
        if (btns[i].btn)
            btns[i].btn->Resize(colX[btns[i].col], rowY[btns[i].row], 120, 32);
    }

    if (mToolTip)
        mToolTip->Resize(0, mHeight - 30, mWidth, 24);
}

// -------------------------------------------------------------------------
// SyncProfile -- called by LawnApp after profile load. In this MVP we
// just unlock the sub-modes based on whether the player has finished
// adventure (LawnApp::HasFinishedAdventure).
// -------------------------------------------------------------------------
void GameSelector::SyncProfile(bool /*theShowLoading*/)
{
    if (!mApp) return;
    bool hasFinished = mApp->HasFinishedAdventure();
    mMinigamesLocked = !hasFinished && !mUnlockSelectorCheat;
    mPuzzleLocked    = !hasFinished && !mUnlockSelectorCheat;
    mSurvivalLocked  = !hasFinished && !mUnlockSelectorCheat;
    if (mMinigameButton) mMinigameButton->SetDisabled(mMinigamesLocked);
    if (mPuzzleButton)   mPuzzleButton->SetDisabled(mPuzzleLocked);
    if (mSurvivalButton) mSurvivalButton->SetDisabled(mSurvivalLocked);
    if (mStoreButton)     mStoreButton->SetDisabled(!mApp->CanShowStore());
    if (mAlmanacButton)   mAlmanacButton->SetDisabled(!mApp->CanShowAlmanac());
    if (mZenGardenButton) mZenGardenButton->SetDisabled(!mApp->CanShowZenGarden());
}

// -------------------------------------------------------------------------
// UpdateTooltip -- set the tooltip text based on which button is hovered.
// -------------------------------------------------------------------------
void GameSelector::UpdateTooltip()
{
    if (!mToolTip) return;
    const char* txt = "";
    if      (mAdventureButton  && mAdventureButton->IsMouseOver())  txt = "Start the main adventure";
    else if (mSurvivalButton   && mSurvivalButton->IsMouseOver())   txt = "Endless survival levels";
    else if (mMinigameButton   && mMinigameButton->IsMouseOver())   txt = "Fun mini-games";
    else if (mPuzzleButton     && mPuzzleButton->IsMouseOver())     txt = "Vasebreaker / I, Zombie";
    else if (mStoreButton      && mStoreButton->IsMouseOver())      txt = "Buy upgrades for your garden";
    else if (mAlmanacButton    && mAlmanacButton->IsMouseOver())    txt = "View plant and zombie info";
    else if (mZenGardenButton  && mZenGardenButton->IsMouseOver())  txt = "Tend to your Zen Garden";
    else if (mOptionsButton    && mOptionsButton->IsMouseOver())    txt = "Game options";
    else if (mHelpButton       && mHelpButton->IsMouseOver())       txt = "How to play";
    else if (mQuitButton       && mQuitButton->IsMouseOver())       txt = "Exit the game";
    mToolTip->SetText(txt);
}

// -------------------------------------------------------------------------
// Update
// -------------------------------------------------------------------------
void GameSelector::Update()
{
    Widget::Update();
    UpdateTooltip();
    // Advance the reanim player. The port runs at 30 FPS, so each Update
    // call represents ~1/30 s of wall-clock time. This drives cloud drift,
    // flower sway, and any other animated SelectorScreen tracks.
    if (mReanimLoaded)
        mReanimPlayer.Update(1.0f / 30.0f);
}

// -------------------------------------------------------------------------
// Draw -- background image + title. (Buttons draw themselves via
// WidgetManager::DrawAll iterating top-level widgets.)
// -------------------------------------------------------------------------
void GameSelector::Draw(Graphics* g)
{
    if (g == NULL) return;

    g->SetColor(Color(255, 255, 255, 255));

    // [Stage-1] If SelectorScreen.reanim loaded, render the animated menu via
    // ReanimPlayer. This draws every track (background, gravestone, wooden
    // signs, button sprites, clouds, flowers, logo, etc.) at the interpolated
    // transform for the current mAnimTime, scaled from the reanim's 800x600
    // space to the 400x300 canvas (ReanimPlayer::mCoordScale = 0.5).
    if (mReanimLoaded && mReanimDef.mTrackCount > 0)
    {
        mReanimPlayer.Draw(g);
        return;  // reanim rendered, skip fallback
    }

    // -- Fallback: IMAGE_BACKGROUND1 (lawn) or IMAGE_TITLESCREEN --
    Image* bg = IMAGE_BACKGROUND1;
    if (bg == NULL)
        bg = IMAGE_TITLESCREEN;
    if (bg == NULL && mApp && mApp->mResourceManager)
    {
        bg = mApp->mResourceManager->GetImage("IMAGE_BACKGROUND1");
        if (!bg) bg = mApp->mResourceManager->GetImage("IMAGE_TITLESCREEN");
    }

    if (bg && bg->GetWidth() > 0 && bg->GetHeight() > 0)
    {
        MemoryImage* mem = static_cast<MemoryImage*>(bg);
        g->DrawImage(mem, 0, 0, mWidth, mHeight);
    }
    else
    {
        g->SetColor(Color(20, 60, 30, 255));
        g->FillRect(0, 0, mWidth, mHeight);
    }

    // Title text -- use SystemFont (8x8 bitmap fallback) since PvZ font assets
    // are not in the PAK (M4 #4). FONT_DWARVEN is NULL in this port.
    SystemFont* titleFont = SystemFont::Get();
    if (titleFont)
    {
        g->SetFont(titleFont);
        g->SetColor(Color(255, 240, 200, 255));
        const char* title = "Plants vs. Zombies";
        int sw = titleFont->StringWidth(title);
        g->DrawString(title, (mWidth - sw) / 2, 30);
    }
}

// -------------------------------------------------------------------------
// ButtonDepressed -- click routing. Called by GameButton::MouseUp when the
// user clicks a button. We dispatch to the appropriate LawnApp method.
// -------------------------------------------------------------------------
void GameSelector::ButtonDepressed(int theId)
{
    if (!mApp) return;
    TBuf8<80> b;
    b.Format(_L8("GS:ButtonDepress id=%d\n"), theId);
    GSLog(b);

    switch (theId)
    {
    case GameSelector_Adventure:
        // [M4 #1] Adventure -> PreNewGame -> NewGame -> MakeNewBoard ->
        // Board::InitLevel -> ShowSeedChooserScreen -> CutScene::StartLevelIntro.
        // This whole chain depends on unported subsystems (Board game logic,
        // SeedChooser UI, CutScene animation, plant/zombie reanimations).
        // Calling PreNewGame crashes deep in Board::InitLevel because of NULL
        // IMAGE_* / REANIM_* resources. Disable until M5 ports the gameplay.
        GSLog(_L8("GS:Adventure (gameplay not yet ported -- M5)\n"));
        break;

    case GameSelector_Survival:
        // ShowChallengeScreen -> ChallengeScreen constructor needs IMAGE_*
        // resources that are NULL. Crashes. Disable until ChallengeScreen
        // is properly ported.
        GSLog(_L8("GS:Survival (ChallengeScreen not yet ported -- M5)\n"));
        break;

    case GameSelector_Minigame:
        GSLog(_L8("GS:Minigame (ChallengeScreen not yet ported -- M5)\n"));
        break;

    case GameSelector_Puzzle:
        GSLog(_L8("GS:Puzzle (ChallengeScreen not yet ported -- M5)\n"));
        break;

    case GameSelector_Store:
        // ShowStoreScreen -> StoreScreen constructor (1187 lines upstream)
        // needs many IMAGE_* resources that are NULL. Crashes. Disable.
        GSLog(_L8("GS:Store (StoreScreen not yet ported -- M5)\n"));
        break;

    case GameSelector_Almanac:
        // DoAlmanacDialog -> AlmanacDialog constructor (718 lines upstream)
        // needs IMAGE_ALMANAC_* resources that are NULL. Crashes. Disable.
        GSLog(_L8("GS:Almanac (AlmanacDialog not yet ported -- M5)\n"));
        break;

    case GameSelector_ZenGarden:
        GSLog(_L8("GS:ZenGarden (not yet ported -- M5)\n"));
        break;

    case GameSelector_Options:
        // DoNewOptions has a NULL guard on IMAGE_OPTIONS_MENUBACK now, but
        // NewOptionsDialog itself may still crash on other NULL resources.
        // Try it -- if it crashes, we'll add more guards.
        GSLog(_L8("GS:Options (calling DoNewOptions)\n"));
        mApp->DoNewOptions(true /* fromGameSelector */);
        break;

    case GameSelector_Help:
        GSLog(_L8("GS:Help (not yet implemented)\n"));
        break;

    case GameSelector_Quit:
        // ConfirmQuit -> shows a yes/no dialog. Should be safe (Dialog base
        // class is ported). If it crashes, the dialog's IMAGE_* refs need
        // NULL guards.
        GSLog(_L8("GS:Quit (calling ConfirmQuit)\n"));
        mApp->ConfirmQuit();
        break;

    default:
        break;
    }
}

void GameSelector::ButtonMouseEnter(int /*theId*/)
{
    // Could play a hover sound here once Music is wired.
}

// -------------------------------------------------------------------------
// KeyDown -- ESC -> Quit confirmation, ENTER -> same as Adventure click.
// Both are disabled for now (gameplay not ported -- see ButtonDepressed note).
// -------------------------------------------------------------------------
void GameSelector::KeyDown(KeyCode theKey)
{
    if (!mApp) return;
    if (theKey == KEYCODE_ESCAPE)
    {
        // ConfirmQuit may crash on dialog IMAGE_* refs -- log and try.
        GSLog(_L8("GS:KeyDown ESC (calling ConfirmQuit)\n"));
        mApp->ConfirmQuit();
    }
    else if (theKey == KEYCODE_RETURN)
    {
        // Adventure -> PreNewGame crashes deep in Board::InitLevel.
        // Log only -- see ButtonDepressed(GameSelector_Adventure) comment.
        GSLog(_L8("GS:KeyDown ENTER (Adventure not yet ported -- M5)\n"));
    }
}

void GameSelector::MouseDown(int /*x*/, int /*y*/, int /*theClickCount*/)
{
    // Clicks route through our child buttons (top-level in WidgetManager),
    // not through the selector itself.
}

} // namespace Sexy
