/*
 * GameSelector.h -- M4 #1 main-menu widget.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/GameSelector.h, adapted
 * to this port's infrastructure:
 *   - uses port's GameButton (not upstream's NewLawnButton -- needs
 *     DialogButton infrastructure that isn't ported yet)
 *   - uses port's ButtonListener interface (ButtonDepressed instead of
 *     upstream's ButtonPress)
 *   - Reanimation / ParticleSystem fields are stubbed out (Reanimator /
 *     TodLib not ported yet -- they're M5)
 *   - ToolTipWidget is ported as a simple text-label widget (no rich
 *     formatting) -- see ToolTipWidget.h
 *
 * Layout (M4 #1 MVP): 9 buttons in a 3x3 grid + Adventure button on top.
 *   Adventure      (full-width, large)
 *   [Survival] [Minigame] [Puzzle]
 *   [Store]    [Almanac]  [Zen Garden]
 *   [Options]  [Help]     [Quit]
 *
 * All buttons are created in the constructor. Click routing goes through
 * ButtonDepress -> a switch on mId -> the appropriate LawnApp method
 * (PreNewGame / ShowChallengeScreen / ShowStoreScreen / DoNewOptions /
 * ConfirmQuit etc.).
 */
#ifndef __GAMESELECTOR_H__
#define __GAMESELECTOR_H__

#include "../../engine/Widget.h"
#include "../../engine/ButtonListener.h"
#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/ReanimLoader.h"

namespace Sexy { class LawnApp; }
class ToolTipWidget;
class GameButton;

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

    // -- Owned sub-widgets ------------------------------------------------
    GameButton*     mAdventureButton;
    GameButton*     mMinigameButton;
    GameButton*     mPuzzleButton;
    GameButton*     mSurvivalButton;
    GameButton*     mOptionsButton;
    GameButton*     mQuitButton;
    GameButton*     mHelpButton;
    GameButton*     mStoreButton;
    GameButton*     mAlmanacButton;
    GameButton*     mZenGardenButton;
    GameButton*     mChangeUserButton;
    ToolTipWidget*  mToolTip;
    ReanimDefinition mReanimDef;   // parsed SelectorScreen.reanim.compiled
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
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); LayoutButtons(); }
    // NOTE: port's WidgetContainer::AddWidget does NOT call AddedToManager
    // (unlike upstream's WidgetManager). We create buttons in the constructor
    // directly, so AddedToManager/RemovedFromManager are not needed here.
    virtual void KeyDown(KeyCode theKey);
    virtual void MouseDown(int x, int y, int theClickCount);

    // -- ButtonListener overrides ---------------------------------------
    // Port's ButtonListener interface uses ButtonDepressed (NOT ButtonPress
    // like upstream). See engine/ButtonListener.h.
    virtual void ButtonDepressed(int theId);
    virtual void ButtonMouseEnter(int theId);
    virtual void ButtonMouseLeave(int theId) { (void)theId; }
    virtual void ButtonMouseMove(int, int, int) {}

    // -- API -------------------------------------------------------------
    void            SyncProfile(bool theShowLoading);
    void            LayoutButtons();
    void            UpdateTooltip();
    bool            ShouldDoZenTutorialBeforeAdventure() { return false; }
};

} // namespace Sexy

#endif // __GAMESELECTOR_H__
