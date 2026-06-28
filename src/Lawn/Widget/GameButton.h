/*
 * GameButton.h -- M4 #1 main-menu button widget.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/GameButton.h but adapted
 * to the Symbian port's existing infrastructure:
 *   - inherits from Sexy::Widget (port's WidgetManager uses Widget* arrays),
 *     not a standalone class with its own mX/mY fields
 *   - uses port's ButtonListener (which has ButtonDepressed/ButtonPressed/
 *     ButtonMouseEnter/Leave, NOT upstream's ButtonPress)
 *   - uses port's Graphics API (DrawImage(MemoryImage*,x,y), DrawString,
 *     SetColor, SetDrawMode, SetColorizeImages stub)
 *   - dynamic_cast is FORBIDDEN (RTTI unreliable under GCCE 3.4.3 per M3 fix #1)
 *
 * Upstream also defines LawnStoneButton and NewLawnButton (both derive from
 * DialogButton, which itself derives from ButtonWidget). Those are NOT ported
 * here yet -- they need full DialogButton infrastructure (component image
 * slicing, polygon hit-test, etc.) that this port doesn't have. GameSelector
 * uses GameButton directly for now; LawnStoneButton/NewLawnButton can be
 * added later by porting DialogButton.cpp first.
 *
 * The DrawStoneButton() free function IS ported (used by GameButton when
 * mDrawStoneButton is set) -- it draws a 9-slice stone button from
 * IMAGE_BUTTON_LEFT/MIDDLE/RIGHT (and DOWN_ variants). If those globals are
 * NULL (README M4 #5: 51 NULL symbols), it falls back to a filled rect so
 * the menu is still visible.
 */
#ifndef __GAMEBUTTON_H__
#define __GAMEBUTTON_H__

#include "../../engine/Widget.h"
#include "../../engine/ButtonListener.h"
#include "../../engine/Color.h"
#include "../../engine/Rect.h"
#include <string>

// Forward declarations -- LawnApp is in namespace Sexy (see LawnApp.h).
namespace Sexy { class LawnApp; class Image; class Font; class Graphics; class WidgetManager; }

class GameButton : public Sexy::Widget
{
public:
    enum
    {
        BUTTON_LABEL_LEFT   = -1,
        BUTTON_LABEL_CENTER = 0,
        BUTTON_LABEL_RIGHT  = 1
    };
    enum
    {
        COLOR_LABEL          = 0,
        COLOR_LABEL_HILITE   = 1,
        COLOR_DARK_OUTLINE   = 2,
        COLOR_LIGHT_OUTLINE  = 3,
        COLOR_MEDIUM_OUTLINE = 4,
        COLOR_BKG            = 5,
        NUM_COLORS           = 6
    };

    // -- Identity / listener -----------------------------------------------
    Sexy::LawnApp*              mApp;
    int                         mId;
    Sexy::ButtonListener*       mListener;

    // -- Layout -----------------------------------------------------------
    // (mX/mY/mWidth/mHeight are inherited from Widget; duplicated upstream
    //  fields are NOT added here to avoid confusion.)
    Sexy::Widget*           mParentWidget;
    int                     mTextOffsetX;
    int                     mTextOffsetY;
    int                     mButtonOffsetX;
    int                     mButtonOffsetY;

    // -- State -----------------------------------------------------------
    bool                    mIsOver;
    bool                    mIsDown;
    bool                    mDisabled;
    bool                    mInverted;
    bool                    mBtnNoDraw;
    bool                    mFrameNoDraw;
    bool                    mDrawStoneButton;
    bool                    mHasAlpha;
    bool                    mHasTransparencies;
    int                     mTranslateX;
    int                     mTranslateY;
    double                  mOverAlpha;
    double                  mOverAlphaSpeed;
    double                  mOverAlphaFadeInSpeed;

    // -- Visuals ---------------------------------------------------------
    Sexy::Color             mColors[NUM_COLORS];
    std::string             mLabel;
    int                     mLabelJustify;
    Sexy::Font*             mFont;

    Sexy::Image*            mButtonImage;
    Sexy::Image*            mOverImage;
    Sexy::Image*            mDownImage;
    Sexy::Image*            mDisabledImage;
    Sexy::Image*            mOverOverlayImage;

    Sexy::Rect              mNormalRect;
    Sexy::Rect              mOverRect;
    Sexy::Rect              mDownRect;
    Sexy::Rect              mDisabledRect;

public:
    GameButton(int theId, Sexy::ButtonListener* theListener = NULL);
    virtual ~GameButton();

    // -- Widget overrides (port's Widget base) ---------------------------
    virtual void Draw(Sexy::Graphics* g);
    virtual void Update();
    virtual void Resize(int x, int y, int w, int h) { Widget::Resize(x, y, w, h); }
    virtual void MouseDown(int x, int y, int btn);
    virtual void MouseUp(int x, int y, int btn);
    virtual void MouseEnter();
    virtual void MouseLeave();
    virtual void MouseMove(int x, int y);

    // -- GameButton API --------------------------------------------------
    void            SetLabel(const std::string& theLabel);
    const std::string& GetLabel() const { return mLabel; }
    void            SetFont(Sexy::Font* theFont);
    void            SetDisabled(bool theDisabled) { mDisabled = theDisabled; }
    bool            IsDisabled() const { return mDisabled; }
    bool            IsMouseOver() const { return mIsOver && !mDisabled && !mBtnNoDraw; }
    bool            IsButtonDown() const { return mIsDown && mIsOver && !mDisabled && !mBtnNoDraw; }
    void            SetColor(int idx, const Sexy::Color& c) { if (idx >= 0 && idx < NUM_COLORS) mColors[idx] = c; }

    // -- Compat: hit-test helpers used by board.cpp (Board::MouseDown).
    // Upstream GameButton has a real MouseHitTest that checks the polygon
    // shape; the old port stub returned false unconditionally. We replicate
    // the stub behaviour -- board.cpp only uses it to gate "did the click
    // land on the menu button", and the WidgetManager's FindWidget already
    // routes clicks to the right widget. Return false so the early-return
    // path in Board::MouseDown is NOT taken; the menu/store button gets
    // the click through the normal widget dispatch.
    bool            MouseHitTest(int x, int y) { (void)x; (void)y; return false; }
    bool            MouseHitTest(int x, int y, int theClickCount) { (void)x; (void)y; (void)theClickCount; return false; }

    // -- Helpers (used internally and by DrawStoneButton) ----------------
    static bool     HaveButtonImage(Sexy::Image* theImage, const Sexy::Rect& theRect);
    void            DrawButtonImage(Sexy::Graphics* g, Sexy::Image* theImage,
                                    const Sexy::Rect& theRect, int theX, int theY);
};

// -- Free helper: draws a 9-slice "stone" button (used by GameButton and
//    by LawnStoneButton when it's eventually ported). If the IMAGE_BUTTON_*
//    globals are NULL (resource not loaded), falls back to a filled rect
//    so the menu is still visible.
void DrawStoneButton(Sexy::Graphics* g, int x, int y, int theWidth, int theHeight,
                     bool isDown, bool isHighLighted, const std::string& theLabel);

#endif // __GAMEBUTTON_H__
