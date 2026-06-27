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
        if (f.Replace(fs, _L("C:\\Data\\PvZ\\gs_log.txt"), EFileWrite) == KErrNone)
        {
            f.Write(aMsg);
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
    mMouseVisible = true;
    mHasTransparencies = true;
    mDoFinger = false;
    mWantsFocus = true;

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

    MAKE_BUTTON(mAdventureButton,  GameSelector_Adventure, "[Adventure]");
    MAKE_BUTTON(mSurvivalButton,   GameSelector_Survival,  "[Survival]");
    MAKE_BUTTON(mMinigameButton,   GameSelector_Minigame,  "[Minigames]");
    MAKE_BUTTON(mPuzzleButton,     GameSelector_Puzzle,    "[Puzzles]");
    MAKE_BUTTON(mStoreButton,      GameSelector_Store,     "[Store]");
    MAKE_BUTTON(mAlmanacButton,    GameSelector_Almanac,   "[Almanac]");
    MAKE_BUTTON(mZenGardenButton,  GameSelector_ZenGarden, "[Zen Garden]");
    MAKE_BUTTON(mOptionsButton,    GameSelector_Options,   "[Options]");
    MAKE_BUTTON(mHelpButton,       GameSelector_Help,      "[Help]");
    MAKE_BUTTON(mQuitButton,       GameSelector_Quit,      "[Quit]");
    #undef MAKE_BUTTON

    mToolTip = new ToolTipWidget();
    mToolTip->Resize(0, 270, 400, 24);
    wm->AddWidget(mToolTip);

    GSLog(_L8("GS:ctor buttons created and added to WidgetManager\n"));
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
    // mStartingGame / mStartingGameCounter are kept for future use (e.g.
    // transition animations before launching a game) but currently unused --
    // ButtonDepressed calls LawnApp flow methods directly.
}

// -------------------------------------------------------------------------
// Draw -- background image + title. (Buttons draw themselves via
// WidgetManager::DrawAll iterating top-level widgets.)
// -------------------------------------------------------------------------
void GameSelector::Draw(Graphics* g)
{
    if (g == NULL) return;

    Image* bg = IMAGE_TITLESCREEN;
    if (bg == NULL && mApp && mApp->mResourceManager)
        bg = mApp->mResourceManager->GetImage("IMAGE_TITLESCREEN");

    g->SetColor(Color(255, 255, 255, 255));
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

    // Title text (best-effort; font may be NULL until M4 #4 fonts done).
    //
    // NOTE: in this port, FONT_DWARVENTODCRAFT18BRIGHTGREENINSET etc. are
    // _Font* (opaque), NOT Sexy::Font*. The only Sexy::Font* globals are
    // FONT_DWARVEN and FONT_COUNTER (declared in engine/Font.h).
    Font* titleFont = Sexy::FONT_DWARVEN;
    if (titleFont)
    {
        g->SetFont(titleFont);
        g->SetColor(Color(255, 240, 200, 255));
        const char* title = "Plants vs. Zombies";
        int sw = titleFont->StringWidth(title);
        g->DrawString(title, (mWidth - sw) / 2, 40);
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
        // Direct call -- matches upstream pattern. The WidgetManager is NOT
        // iterating widgets when ButtonDepressed fires (we're called from
        // GameButton::MouseUp <- WidgetManager::MouseUp, which dispatches
        // only to mLastDownWidget, no array iteration). PreNewGame may
        // synchronously kill this GameSelector (via KillGameSelector ->
        // RemoveWidget + delete), but the event dispatch unwinds cleanly
        // because the caller (WidgetManager::MouseUp) doesn't touch the
        // widget array after the dispatch returns.
        mApp->PreNewGame(GAMEMODE_ADVENTURE, true);
        break;

    case GameSelector_Survival:
        if (!mSurvivalLocked)
            mApp->ShowChallengeScreen(CHALLENGE_PAGE_SURVIVAL);
        break;

    case GameSelector_Minigame:
        // CHALLENGE_PAGE_MINIGAMES doesn't exist in this port's ConstEnums.h
        // -- use CHALLENGE_PAGE_CHALLENGE (the "mini-games" page in PvZ).
        if (!mMinigamesLocked)
            mApp->ShowChallengeScreen(CHALLENGE_PAGE_CHALLENGE);
        break;

    case GameSelector_Puzzle:
        if (!mPuzzleLocked)
            mApp->ShowChallengeScreen(CHALLENGE_PAGE_PUZZLE);
        break;

    case GameSelector_Store:
        if (mApp->CanShowStore())
            mApp->ShowStoreScreen();
        break;

    case GameSelector_Almanac:
        if (mApp->CanShowAlmanac())
            mApp->DoAlmanacDialog(SEED_NONE, ZOMBIE_INVALID);
        break;

    case GameSelector_ZenGarden:
        GSLog(_L8("GS:ZenGarden (not yet implemented)\n"));
        break;

    case GameSelector_Options:
        mApp->DoNewOptions(true /* fromGameSelector */);
        break;

    case GameSelector_Help:
        GSLog(_L8("GS:Help (not yet implemented)\n"));
        break;

    case GameSelector_Quit:
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
// KeyDown -- ESC -> Quit confirmation, ENTER -> Adventure.
// -------------------------------------------------------------------------
void GameSelector::KeyDown(KeyCode theKey)
{
    if (!mApp) return;
    if (theKey == KEYCODE_ESCAPE)
    {
        mApp->ConfirmQuit();
    }
    else if (theKey == KEYCODE_RETURN)
    {
        // Direct call (see ButtonDepressed note on the dispatch safety).
        mApp->PreNewGame(GAMEMODE_ADVENTURE, true);
    }
    // Arrow-key navigation between buttons can be added here once the
    // focus-widget chain is wired (port's WidgetManager::FocusNextWidget
    // exists but isn't currently called from input).
}

void GameSelector::MouseDown(int /*x*/, int /*y*/, int /*theClickCount*/)
{
    // Clicks route through our child buttons (top-level in WidgetManager),
    // not through the selector itself.
}

} // namespace Sexy
