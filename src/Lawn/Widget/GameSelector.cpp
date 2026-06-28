/*
 * GameSelector.cpp -- main-menu widget (1:1 reanim-based, session 6).
 *
 * Renders the SelectorScreen reanim directly (background, wooden signs,
 * button sprites, clouds, flowers, leaves) and does hit-testing against
 * the reanim button-track positions. The 10 stub GameButton widgets are
 * GONE -- the menu is now the reanim, with click zones derived from the
 * track transforms.
 *
 * Reanim track -> button mapping (from on-device gs_log, 48 tracks):
 *   SelectorScreen_Adventure_button   -> Adventure (id 100)
 *   SelectorScreen_Survival_button    -> Survival  (id 110) [locked]
 *   SelectorScreen_Challenges_button  -> Minigames (id 101) [locked]
 *   SelectorScreen_StartAdventure_button -> (unused, same as Adventure)
 *   SelectorScreen_ZenGarden_button   -> Zen Garden (id 109) [locked]
 *   almanac_key_shadow / woodsign1-3  -> Almanac/Options/Help/Quit hit zones
 *
 * The Options/Help/Quit buttons are NOT separate reanim tracks (they're
 * IMAGE_SELECTORSCREEN_OPTIONS1/2 etc. in upstream). For now we use fixed
 * canvas positions for those three (bottom-right corner, matching upstream
 * GameSelector layout) so the user can still quit / open options.
 */
#include "GameSelector.h"

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
#include "../../engine/Font.h"
#include "../../engine/SystemFont.h"

#include <e32std.h>
#include <f32file.h>
#include <string.h>

namespace Sexy {

// -------------------------------------------------------------------------
// GSLog -- on-device diagnostic logger.
// -------------------------------------------------------------------------
static void GSLog(const TDesC8& aMsg)
{
    RFs fs; RFile f;
    if (fs.Connect() == KErrNone)
    {
        fs.MkDirAll(_L("C:\\Data\\PvZ"));
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
// Construction -- loads the reanim and binds the ReanimPlayer. NO stub
// GameButton widgets are created anymore -- the menu is the reanim.
// -------------------------------------------------------------------------
GameSelector::GameSelector(LawnApp* theApp)
    : Widget()
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
    mMouseVisible = true;    // [Session-6] receive clicks directly for hit-testing
    mHasTransparencies = true;
    mDoFinger = false;
    mWantsFocus = true;
    mReanimLoaded = false;

    GSLog(_L8("GS:ctor (reanim menu, no stub buttons)\n"));

    // [Stage-1] Load SelectorScreen.reanim (XML version) for 1:1 menu.
    mReanimLoaded = EFalse;
    TRAPD(reanimErr, mReanimLoaded = ReanimLoadCompiled(
        "reanim/SelectorScreen.reanim", mReanimDef));
    if (reanimErr != KErrNone)
    {
        TBuf8<96> eb;
        eb.Format(_L8("GS:reanim LEAVED err=%d\n"), reanimErr);
        GSLog(eb);
        mReanimLoaded = EFalse;
    }
    if (mReanimLoaded)
    {
        TBuf8<80> b;
        b.Format(_L8("GS:reanim loaded: %d tracks, FPS=%.1f\n"),
                 mReanimDef.mTrackCount, mReanimDef.mFPS);
        GSLog(b);

        mReanimPlayer.SetDefinition(&mReanimDef);
        mReanimPlayer.mLoopType = ReanimPlayer::LOOP_ON;
        mReanimPlayer.mAnimRate = 1.0f;
        TBuf8<96> pb;
        pb.Format(_L8("GS:ReanimPlayer bound, duration=%.2fs\n"),
                  mReanimPlayer.GetDuration());
        GSLog(pb);

        // [Session-8] Preload all reanim images NOW (in the constructor, not
        // in Draw). The lazy-load in ReanimPlayer::Draw was not firing on-
        // device (gs_log showed RP:track lines but ZERO RP:load lines).
        // Preloading here is simpler and more reliable: iterate every track,
        // find transforms with image names, call GetImage to resolve them.
        // Most images are already in the ResourceManager cache from
        // LoadingThreadProc, so GetImage returns instantly.
        int preloadedCount = 0;
        for (int i = 0; i < mReanimDef.mTrackCount; i++)
        {
            ReanimTrack& tr = mReanimDef.mTracks[i];
            for (int j = 0; j < tr.mTransformCount; j++)
            {
                ReanimTransform& xf = tr.mTransforms[j];
                if (!xf.mImage && xf.mImageName && xf.mImageName[0] != '\0')
                {
                    if (gResourceManager)
                        xf.mImage = gResourceManager->GetImage(xf.mImageName);
                    if (xf.mImage)
                        preloadedCount++;
                }
            }
        }
        TBuf8<80> pl;
        pl.Format(_L8("GS:preloaded %d reanim images\n"), preloadedCount);
        GSLog(pl);
    }
    else
    {
        GSLog(_L8("GS:reanim FAILED\n"));
    }
}

GameSelector::~GameSelector()
{
    // No stub buttons to tear down -- the reanim definition is freed by its
    // own destructor when mReanimDef goes out of scope.
}

// -------------------------------------------------------------------------
// SyncProfile -- unlock sub-modes based on adventure progress.
// -------------------------------------------------------------------------
void GameSelector::SyncProfile(bool /*theShowLoading*/)
{
    if (!mApp) return;
    bool hasFinished = mApp->HasFinishedAdventure();
    mMinigamesLocked = !hasFinished && !mUnlockSelectorCheat;
    mPuzzleLocked    = !hasFinished && !mUnlockSelectorCheat;
    mSurvivalLocked  = !hasFinished && !mUnlockSelectorCheat;
}

// -------------------------------------------------------------------------
// UpdateTooltip -- (no tooltip widget now; kept for future use).
// -------------------------------------------------------------------------
void GameSelector::UpdateTooltip()
{
}

// -------------------------------------------------------------------------
// Update
// -------------------------------------------------------------------------
void GameSelector::Update()
{
    Widget::Update();
    // Advance the reanim player. The port runs at 30 FPS.
    if (mReanimLoaded)
        mReanimPlayer.Update(1.0f / 30.0f);
}

// -------------------------------------------------------------------------
// Draw -- render the SelectorScreen reanim (graveyard scene). The lawn
// (IMAGE_BACKGROUND1) is for gameplay (Board), NOT the menu. If the reanim
// didn't load, show a dark background + error text (not a misleading lawn).
// -------------------------------------------------------------------------
void GameSelector::Draw(Graphics* g)
{
    if (g == NULL) return;

    g->SetColor(Color(255, 255, 255, 255));

    // [Session-8] The menu IS the reanim. SelectorScreen_BG track draws the
    // graveyard scene (dirt path, wooden signs, tombstones). The lawn
    // (IMAGE_BACKGROUND1) is for gameplay (Board), NOT the menu.
    // No lawn fallback — if the reanim didn't load, show a dark background
    // + error text so we know something is wrong (not a misleading lawn).
    if (mReanimLoaded && mReanimDef.mTrackCount > 0)
    {
        mReanimPlayer.Draw(g);
        return;
    }

    // Reanim failed to load — dark background + diagnostic text.
    g->SetColor(Color(30, 25, 20, 255));
    g->FillRect(0, 0, mWidth, mHeight);
    g->SetColor(Color(255, 100, 100, 255));

    SystemFont* titleFont = SystemFont::Get();
    if (titleFont)
    {
        g->SetFont(titleFont);
        const char* msg = "Reanim load failed";
        int sw = titleFont->StringWidth(msg);
        g->DrawString(msg, (mWidth - sw) / 2, mHeight / 2);
    }
}

// -------------------------------------------------------------------------
// HitTestButton -- check which reanim button track the click (x,y) landed
// on. Returns the button ID, or -1 if no hit.
//
// Each button track has a transform[0] with mTransX/mTransY (position in
// 800x600 reanim space) and an mImage whose size + mScaleX/mScaleY define
// the button bounds. We scale everything by 0.5 (reanim -> 400x300 canvas).
// -------------------------------------------------------------------------
int GameSelector::HitTestButton(int x, int y)
{
    if (!mReanimLoaded)
        return -1;

    // Map of reanim track name -> button ID. These match the track names
    // dumped in gs_log (SelectorScreen_Adventure_button, etc.).
    struct TrackButton
    {
        const char* trackName;
        int buttonId;
        bool needsUnlock; // true if locked until adventure finished
    };
    static const TrackButton buttons[] =
    {
        { "SelectorScreen_Adventure_button",   GameSelector_Adventure,  false },
        { "SelectorScreen_Survival_button",    GameSelector_Survival,   true  },
        { "SelectorScreen_Challenges_button",  GameSelector_Minigame,   true  },
        { "SelectorScreen_ZenGarden_button",   GameSelector_ZenGarden,  true  },
        { "SelectorScreen_StartAdventure_button", GameSelector_Adventure, false },
    };
    const int nButtons = sizeof(buttons) / sizeof(buttons[0]);

    for (int i = 0; i < nButtons; i++)
    {
        int trackIdx = mReanimPlayer.FindTrackIndex(buttons[i].trackName);
        if (trackIdx < 0)
            continue;
        ReanimTransform t;
        if (!mReanimPlayer.GetCurrentTransform(trackIdx, t))
            continue;
        if (!t.mImage)
            continue;

        // Button bounds in reanim space (800x600).
        float bx = t.mTransX;
        float by = t.mTransY;
        float bw = t.mImage->GetWidth()  * t.mScaleX;
        float bh = t.mImage->GetHeight() * t.mScaleY;

        // Scale to canvas space (400x300) -> multiply by 0.5.
        int cx = (int)(bx * 0.5f);
        int cy = (int)(by * 0.5f);
        int cw = (int)(bw * 0.5f);
        int ch = (int)(bh * 0.5f);
        if (cw <= 0 || ch <= 0)
            continue;

        // Hit-test (inclusive bounds).
        if (x >= cx && x < cx + cw && y >= cy && y < cy + ch)
        {
            // Check lock state.
            if (buttons[i].needsUnlock)
            {
                if (buttons[i].buttonId == GameSelector_Survival && mSurvivalLocked)
                    return -1;
                if (buttons[i].buttonId == GameSelector_Minigame && mMinigamesLocked)
                    return -1;
            }
            return buttons[i].buttonId;
        }
    }

    // Options / Help / Quit are not separate reanim tracks -- use fixed
    // canvas positions matching the upstream GameSelector layout (bottom
    // area of the menu, right side). These are small click zones so the
    // user can still quit / open options.
    // Options: bottom-left woodsign area (upstream puts it ~x=60,y=250)
    if (x >= 20 && x < 120 && y >= 250 && y < 290)
        return GameSelector_Options;
    // Help: bottom-center woodsign area
    if (x >= 140 && x < 240 && y >= 250 && y < 290)
        return GameSelector_Help;
    // Quit: bottom-right woodsign area
    if (x >= 260 && x < 380 && y >= 250 && y < 290)
        return GameSelector_Quit;

    return -1;
}

// -------------------------------------------------------------------------
// MouseDown -- hit-test the click against reanim button positions.
// -------------------------------------------------------------------------
void GameSelector::MouseDown(int x, int y, int /*theClickCount*/)
{
    int id = HitTestButton(x, y);
    if (id >= 0)
    {
        TBuf8<64> b;
        b.Format(_L8("GS:MouseDown hit id=%d at %d,%d\n"), id, x, y);
        GSLog(b);
        ButtonDepressed(id);
    }
    else
    {
        TBuf8<64> b;
        b.Format(_L8("GS:MouseDown miss at %d,%d\n"), x, y);
        GSLog(b);
    }
}

// -------------------------------------------------------------------------
// MouseMove -- (no hover highlight yet; reanim tracks are static).
// -------------------------------------------------------------------------
void GameSelector::MouseMove(int /*x*/, int /*y*/)
{
}

// -------------------------------------------------------------------------
// ButtonDepressed -- click routing. Called by MouseDown hit-test.
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
        GSLog(_L8("GS:Adventure (gameplay not yet ported -- Stage 2)\n"));
        break;

    case GameSelector_Survival:
        GSLog(_L8("GS:Survival (ChallengeScreen not yet ported -- Stage 2)\n"));
        break;

    case GameSelector_Minigame:
        GSLog(_L8("GS:Minigame (ChallengeScreen not yet ported -- Stage 2)\n"));
        break;

    case GameSelector_Puzzle:
        GSLog(_L8("GS:Puzzle (ChallengeScreen not yet ported -- Stage 2)\n"));
        break;

    case GameSelector_Store:
        GSLog(_L8("GS:Store (StoreScreen not yet ported -- Stage 3)\n"));
        break;

    case GameSelector_Almanac:
        GSLog(_L8("GS:Almanac (AlmanacDialog not yet ported -- Stage 3)\n"));
        break;

    case GameSelector_ZenGarden:
        GSLog(_L8("GS:ZenGarden (not yet ported -- Stage 3)\n"));
        break;

    case GameSelector_Options:
        GSLog(_L8("GS:Options (calling DoNewOptions)\n"));
        mApp->DoNewOptions(true);
        break;

    case GameSelector_Help:
        GSLog(_L8("GS:Help (not yet implemented)\n"));
        break;

    case GameSelector_Quit:
        GSLog(_L8("GS:Quit (calling ConfirmQuit)\n"));
        mApp->ConfirmQuit();
        break;

    default:
        break;
    }
}

// -------------------------------------------------------------------------
// KeyDown -- ESC -> Quit confirmation, ENTER -> Adventure.
// -------------------------------------------------------------------------
void GameSelector::KeyDown(KeyCode theKey)
{
    if (!mApp) return;
    if (theKey == KEYCODE_ESCAPE)
    {
        GSLog(_L8("GS:KeyDown ESC (calling ConfirmQuit)\n"));
        mApp->ConfirmQuit();
    }
    else if (theKey == KEYCODE_RETURN)
    {
        GSLog(_L8("GS:KeyDown ENTER -> Adventure\n"));
        ButtonDepressed(GameSelector_Adventure);
    }
}

} // namespace Sexy
