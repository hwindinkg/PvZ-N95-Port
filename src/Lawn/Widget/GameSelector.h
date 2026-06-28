/*
 * GameSelector.h -- main-menu widget (1:1 reanim-based, session 6).
 *
 * Renders the SelectorScreen reanim directly (background, wooden signs,
 * button sprites, clouds, flowers, leaves) and does hit-testing against
 * the reanim button-track positions. No more 10 stub GameButton widgets.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/GameSelector.h.
 * The upstream version uses Reanimation + NewLawnButton (DialogButton);
 * this port uses ReanimPlayer (lightweight runtime in ReanimLoader.h) +
 * manual hit-zones derived from the reanim track transforms.
 *
 * Reanim tracks used (from SelectorScreen.reanim, 48 tracks total):
 *   SelectorScreen_BG          -- full background
 *   SelectorScreen_BG_Center/Left/Right -- background parts
 *   SelectorScreen_Adventure_button / _shadow
 *   SelectorScreen_Survival_button / _shadow
 *   SelectorScreen_Challenges_button / _shadow  (Minigames/Puzzles)
 *   SelectorScreen_ZenGarden_button / _shadow
 *   SelectorScreen_StartAdventure_button / _shadow
 *   Cloud1-7, flower1-3, leaf1-5, woodsign1-3, almanac_key_shadow
 *
 * Click routing: MouseDown hit-tests each button track's transform[0]
 * position + image size (×0.5 for 800x600 -> 400x300 canvas), and calls
 * ButtonDepressed(id) -> LawnApp flow methods.
 */
#ifndef __GAMESELECTOR_H__
#define __GAMESELECTOR_H__

#include "../../engine/Widget.h"
#include "../../engine/ButtonListener.h"
#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/ReanimLoader.h"
#include "../../Sexy.TodLib/ReanimatorRuntime.h"

namespace Sexy { class LawnApp; }

namespace Sexy {

class GameSelector : public Widget, public ButtonListener
{
public:
    // Button IDs (mirrors upstream GameSelector private enum).
    enum
    {
        GameSelector_Adventure         = 100,
        GameSelector_Minigame          = 101,
        GameSelector_Puzzle            = 102,
        GameSelector_Options           = 103,
        GameSelector_Help              = 104,
        GameSelector_Quit              = 105,
        GameSelector_ChangeUser        = 106,
        GameSelector_Store             = 107,
        GameSelector_Almanac           = 108,
        GameSelector_ZenGarden         = 109,
        GameSelector_Survival          = 110,
        GameSelector_Zombatar          = 111,
        GameSelector_AchievementsBack  = 112,
        GameSelector_Achievements      = 113,
        GameSelector_QuickPlay         = 114
    };

    // -- Reanim data ------------------------------------------------------
    ReanimDefinition mReanimDef;   // parsed SelectorScreen.reanim
    ReanimPlayer    mReanimPlayer; // legacy lightweight player (unused now)
    Reanimation*    mSelectorReanim; // [Session-13] full Reanimation runtime
    ReanimationHolder mReanimHolder;  // [Session-13] owns the Reanimation
    bool            mReanimLoaded;

    // -- State -----------------------------------------------------------
    LawnApp*        mApp;
    bool            mStartingGame;
    int             mStartingGameCounter;
    bool            mMinigamesLocked;
    bool            mPuzzleLocked;
    bool            mSurvivalLocked;
    bool            mShowStartButton;
    bool            mLoading;
    bool            mHasTrophy;
    bool            mUnlockSelectorCheat;
    int             mLevel;
    int             mSlideCounter;
    int             mStartX, mStartY;
    int             mDestX,  mDestY;

    // -- Construction ----------------------------------------------------
    GameSelector(LawnApp* theApp);
    virtual ~GameSelector();

    // -- Widget overrides ------------------------------------------------
    virtual void Draw(Graphics* g);
    virtual void Update();
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    virtual void KeyDown(KeyCode theKey);
    virtual void MouseDown(int x, int y, int theClickCount);
    virtual void MouseMove(int x, int y);

    // -- ButtonListener overrides (kept for compat, no stub buttons now) -
    virtual void ButtonDepressed(int theId);
    virtual void ButtonMouseEnter(int theId) { (void)theId; }
    virtual void ButtonMouseLeave(int theId) { (void)theId; }
    virtual void ButtonMouseMove(int, int, int) {}

    // -- API -------------------------------------------------------------
    void            SyncProfile(bool theShowLoading);
    void            UpdateTooltip();
    bool            ShouldDoZenTutorialBeforeAdventure() { return false; }

    // -- Hit-testing -----------------------------------------------------
    // Returns the button ID at canvas point (x,y), or -1 if no hit.
    // Uses the reanim button-track transform[0] position + image size.
    int             HitTestButton(int x, int y);
};

} // namespace Sexy

#endif // __GAMESELECTOR_H__
