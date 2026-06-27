/*
 * GameButton.cpp -- M4 #1 main-menu button implementation.
 *
 * Ported from upstream PvZ-Portable src/Lawn/Widget/GameButton.cpp, adapted
 * to this port's API surface:
 *   - Widget base class provides mX/mY/mWidth/mHeight (so we don't duplicate)
 *   - WidgetManager has mDownButtons bitmask + mLastMouseX/mLastMouseY now
 *     (this file relies on those additions)
 *   - Graphics::SetColorizeImages is a no-op stub in this port (TODO), so
 *     the additive over-overlay draw is currently visual-only
 *   - GetFontThrow is stubbed (README M4 #4), so mFont may be NULL -- we
 *     guard every font call so the button still draws its image even if
 *     the label is invisible. Once fonts are wired, labels will appear.
 *   - dynamic_cast is FORBIDDEN (M3 fix #1). Where upstream used it for
 *     MemoryImage, we use static_cast or the DrawImage(MemoryImage*) overload.
 *   - TodStringTranslate is stubbed in this port, so SetLabel stores the
 *     raw string. Upstream localisation will work once TodStringFile is
 *     ported.
 */
#include "GameButton.h"

#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../engine/Graphics.h"
#include "../../engine/GLInterface.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"
#include "../../engine/Font.h"
#include "../../engine/SystemFont.h"  // for button label text
#include "../../engine/WidgetManager.h"
#include "../../engine/SexyAppBase.h"

#include <string.h>

// -------------------------------------------------------------------------
// Default palette (mirrors upstream gGameButtonColors). Labels are white
// on a grey button; hilite is the same white (no highlight tint until
// fonts arrive).
// -------------------------------------------------------------------------
static Sexy::Color gGameButtonDefaultColors[GameButton::NUM_COLORS] = {
    Sexy::Color(255, 255, 255),  // COLOR_LABEL
    Sexy::Color(255, 255,  64),  // COLOR_LABEL_HILITE
    Sexy::Color(  0,   0,   0),  // COLOR_DARK_OUTLINE
    Sexy::Color(255, 255, 255),  // COLOR_LIGHT_OUTLINE
    Sexy::Color(132, 132, 132),  // COLOR_MEDIUM_OUTLINE
    Sexy::Color(212, 212, 212)   // COLOR_BKG
};

// -------------------------------------------------------------------------
// DrawStoneButton -- 9-slice stone button. If IMAGE_BUTTON_LEFT etc. are
// NULL (resource not loaded -- README M4 #5), falls back to a filled rect
// with the label colour so the button rectangle is at least visible.
// -------------------------------------------------------------------------
void DrawStoneButton(Sexy::Graphics* g, int x, int y, int theWidth, int theHeight,
                     bool isDown, bool isHighLighted, const std::string& theLabel)
{
    using namespace Sexy;
    Image* aLeftImage   = IMAGE_BUTTON_LEFT;
    Image* aMiddleImage = IMAGE_BUTTON_MIDDLE;
    Image* aRightImage  = IMAGE_BUTTON_RIGHT;
    int aFontX = x;
    int aFontY = y;
    int aImageX = x;
    if (isDown)
    {
        aLeftImage   = IMAGE_BUTTON_DOWN_LEFT;
        aMiddleImage = IMAGE_BUTTON_DOWN_MIDDLE;
        aRightImage  = IMAGE_BUTTON_DOWN_RIGHT;
        aFontX++; aFontY++; aImageX++;
    }

    // 9-slice draw only if all three pieces are loaded.
    if (aLeftImage && aMiddleImage && aRightImage &&
        aLeftImage->GetWidth() > 0 && aMiddleImage->GetWidth() > 0 &&
        aRightImage->GetWidth() > 0)
    {
        int aRepeat = (theWidth - aLeftImage->GetWidth() - aRightImage->GetWidth())
                      / aMiddleImage->GetWidth();
        g->DrawImageF(aLeftImage, aImageX, y);
        aImageX += aLeftImage->GetWidth();
        while (aRepeat > 0)
        {
            g->DrawImageF(aMiddleImage, aImageX, y);
            aImageX += aMiddleImage->GetWidth();
            --aRepeat;
        }
        g->DrawImageF(aRightImage, aImageX, y);
    }
    else
    {
        // Fallback: filled rectangle so the button is at least visible.
        // Stone buttons are beige/tan when up, slightly darker when down.
        if (isDown)        g->SetColor(Color(140, 110,  70, 255));
        else if (isHighLighted) g->SetColor(Color(200, 170, 110, 255));
        else               g->SetColor(Color(170, 140,  90, 255));
        g->FillRect(x, y, theWidth, theHeight);
        g->SetColor(Color(60, 40, 20, 255));
        g->DrawRect(x, y, theWidth, theHeight);
    }

    // Label -- use SystemFont (8x8 bitmap fallback) since PvZ font assets
    // are not in the PAK (M4 #4).
    SystemFont* aFont = SystemFont::Get();
    if (aFont)
    {
        g->SetFont(aFont);
        int sw = aFont->StringWidth(theLabel.c_str());
        aFontX += (theWidth - sw) / 2 + 1;
        aFontY += (theHeight - 8) / 2 + 8;
        g->SetColor(Color::White);
        g->DrawString(theLabel.c_str(), aFontX, aFontY);
    }
}

// -------------------------------------------------------------------------
// Constructor / destructor
// -------------------------------------------------------------------------
GameButton::GameButton(int theId, Sexy::ButtonListener* theListener)
    : Widget()
    , mApp(NULL)
    , mId(theId)
    , mListener(theListener)
    , mParentWidget(NULL)
    , mTextOffsetX(0), mTextOffsetY(0)
    , mButtonOffsetX(0), mButtonOffsetY(0)
    , mIsOver(false), mIsDown(false), mDisabled(false)
    , mInverted(false), mBtnNoDraw(false), mFrameNoDraw(false)
    , mDrawStoneButton(false), mHasAlpha(true), mHasTransparencies(true)
    , mTranslateX(1), mTranslateY(1)
    , mOverAlpha(0.0), mOverAlphaSpeed(0.0), mOverAlphaFadeInSpeed(0.0)
    , mLabel()
    , mLabelJustify(BUTTON_LABEL_CENTER)
    , mFont(NULL)
    , mButtonImage(NULL), mOverImage(NULL), mDownImage(NULL)
    , mDisabledImage(NULL), mOverOverlayImage(NULL)
    , mNormalRect(), mOverRect(), mDownRect(), mDisabledRect()
{
    mVisible = true;
    mMouseVisible = true;
    mDoFinger = true;
    mWantsFocus = false;
    mHasTransparencies = true;
    for (int i = 0; i < NUM_COLORS; i++)
        mColors[i] = gGameButtonDefaultColors[i];

    // Hook to LawnApp via global (matches upstream's mApp = (LawnApp*)gSexyAppBase)
    if (Sexy::gSexyAppBase)
        mApp = static_cast<Sexy::LawnApp*>(Sexy::gSexyAppBase);
}

GameButton::~GameButton()
{
    // mFont is owned by the resource system (Resources.cpp globals) -- do NOT delete.
    mFont = NULL;
}

// -------------------------------------------------------------------------
// Static / inline helpers
// -------------------------------------------------------------------------
bool GameButton::HaveButtonImage(Sexy::Image* theImage, const Sexy::Rect& theRect)
{
    return theImage != NULL || theRect.mWidth != 0;
}

void GameButton::DrawButtonImage(Sexy::Graphics* g, Sexy::Image* theImage,
                                 const Sexy::Rect& theRect, int theX, int theY)
{
    if (theImage == NULL) return;
    int aPosX = theX + mButtonOffsetX;
    int aPosY = theY + mButtonOffsetY;
    // Port's Graphics::DrawImage(Image*,x,y,srcRect) is currently a stub
    // (Graphics.h line 115). Use DrawImageF for now (no sub-rect support),
    // and ignore theRect. Once DrawImage sub-rect is implemented, switch.
    if (theRect.mWidth != 0 && theRect.mHeight != 0)
    {
        // Sub-rect path -- not yet functional in this port's Graphics.
        // Still call DrawImageF so SOMETHING draws (the whole image) rather
        // than nothing.
        (void)theRect;
        g->DrawImageF(theImage, (float)aPosX, (float)aPosY);
    }
    else
    {
        g->DrawImageF(theImage, (float)aPosX, (float)aPosY);
    }
}

void GameButton::SetFont(Sexy::Font* theFont)
{
    // We don't duplicate the font (would need Duplicate(), which needs the
    // real ImageFont impl). Just hold the pointer -- Resources.cpp owns it.
    mFont = theFont;
}

void GameButton::SetLabel(const std::string& theLabel)
{
    // Upstream calls TodStringTranslate(theLabel) here for localisation.
    // TodStringFile is stubbed in this port -- store the raw string.
    mLabel = theLabel;
}

// -------------------------------------------------------------------------
// Update -- hover / down tracking via WidgetManager's mouse state.
// Mirrors upstream GameButton::Update, using port's WidgetManager fields.
// -------------------------------------------------------------------------
void GameButton::Update()
{
    if (!mWidgetManager) return;

    int aMouseX = mWidgetManager->mLastMouseX;
    int aMouseY = mWidgetManager->mLastMouseY;
    if (mParentWidget)
    {
        aMouseX -= mParentWidget->mX;
        aMouseY -= mParentWidget->mY;
    }

    // Upstream gates hover on (focus widget == parent) OR (no dialogs open).
    // Port's LawnApp::GetDialogCount() always returns 0, so this is always
    // true for now -- leaving the gate in for when dialogs come online.
    bool focusOk = (mWidgetManager->mFocusWidget && mWidgetManager->mFocusWidget == mParentWidget)
                   || (mApp && mApp->GetDialogCount() <= 0);
    if (focusOk)
    {
        mIsOver = (aMouseX >= mX && aMouseX < mX + mWidth &&
                   aMouseY >= mY && aMouseY < mY + mHeight);
        // mDownButtons bitmask: bit 0 = left (1), bit 2 = middle (4). & 5
        // means "left OR middle pressed" -- matches upstream.
        mIsDown = (mWidgetManager->mDownButtons & 5) != 0;
    }
    else
    {
        mIsOver = false;
        mIsDown = false;
    }

    // Over-alpha fade (upstream behaviour). When not hovering, fades out.
    if (!mIsDown && !mIsOver && mOverAlpha > 0.0)
    {
        if (mOverAlphaSpeed < 0)
        {
            mOverAlpha = 0.0;
        }
        else
        {
            mOverAlpha -= mOverAlphaSpeed;
            if (mOverAlpha < 0.0) mOverAlpha = 0.0;
        }
    }
    else if (mIsOver && mOverAlphaFadeInSpeed > 0.0 && mOverAlpha < 1.0)
    {
        mOverAlpha += mOverAlphaFadeInSpeed;
        if (mOverAlpha > 1.0) mOverAlpha = 1.0;
    }
    else if (mIsOver && mOverAlphaFadeInSpeed <= 0.0)
    {
        mOverAlpha = 1.0;
    }

    Widget::Update();
}

// -------------------------------------------------------------------------
// Mouse event handlers -- Widget base routes these to us with coords
// already converted to widget-local space.
// -------------------------------------------------------------------------
void GameButton::MouseDown(int /*x*/, int /*y*/, int /*btn*/)
{
    // mIsDown will be set true by Update() polling mDownButtons.
}

void GameButton::MouseUp(int x, int y, int btn)
{
    // Click = MouseUp while still over the button (the user pressed down
    // inside us and released inside us). WidgetManager::MouseUp already
    // cleared mDownButtons, so we can't poll -- but mIsOver is still valid
    // because MouseMove fired before MouseUp.
    (void)x; (void)y; (void)btn;
    if (mIsOver && !mDisabled && !mBtnNoDraw && mListener)
    {
        // Port's ButtonListener interface uses ButtonDepressed (NOT
        // ButtonPress like upstream). See engine/ButtonListener.h.
        mListener->ButtonDepressed(mId);
    }
}

void GameButton::MouseEnter()
{
    mIsOver = true;
    if (mListener) mListener->ButtonMouseEnter(mId);
}

void GameButton::MouseLeave()
{
    mIsOver = false;
    if (mListener) mListener->ButtonMouseLeave(mId);
}

void GameButton::MouseMove(int /*x*/, int /*y*/)
{
    // mIsOver is kept fresh by Update(); nothing to do here.
}

// -------------------------------------------------------------------------
// Draw
// -------------------------------------------------------------------------
void GameButton::Draw(Sexy::Graphics* g)
{
    if (mBtnNoDraw) return;
    if (g == NULL) return;

    // White vertex colour so GL_MODULATE doesn't tint the texture
    // (M3 fix #7 -- this was the purple-tint bug).
    g->SetColor(Sexy::Color(255, 255, 255, 255));

    bool isDown = IsButtonDown() ^ mInverted;
    bool isHighLighted = IsMouseOver();

    // Stone button path (delegates to DrawStoneButton free function).
    if (mDrawStoneButton)
    {
        DrawStoneButton(g, mX, mY, mWidth, mHeight, isDown, isHighLighted, mLabel);
        return;
    }

    // Image-based button path. Translate the graphics context so
    // child draws are relative to (mX, mY).
    g->mTransX += mX;
    g->mTransY += mY;

    // Lazy default font: use SystemFont if no explicit font was assigned.
    // PvZ font assets are not in the PAK (M4 #4), so FONT_DWARVEN is NULL.
    if (!mFont && !mLabel.empty())
    {
        mFont = SystemFont::Get();
    }

    int aFontX = mTextOffsetX;
    int aFontY = mTextOffsetY;
    if (mFont)
    {
        if (mLabelJustify == BUTTON_LABEL_CENTER)
            aFontX += (mWidth - mFont->StringWidth(mLabel.c_str())) / 2;
        else if (mLabelJustify == BUTTON_LABEL_RIGHT)
            aFontX += mWidth - mFont->StringWidth(mLabel.c_str());
        aFontY += (mHeight - mFont->GetAscent() / 6 + mFont->GetAscent() - 1) / 2;
        g->SetFont(mFont);
    }

    if (!isDown)
    {
        if (mDisabled && HaveButtonImage(mDisabledImage, mDisabledRect))
            DrawButtonImage(g, mDisabledImage, mDisabledRect, 0, 0);
        else if (mOverAlpha > 0.0 && HaveButtonImage(mOverImage, mOverRect))
        {
            if (HaveButtonImage(mButtonImage, mNormalRect) && mOverAlpha < 1.0)
                DrawButtonImage(g, mButtonImage, mNormalRect, 0, 0);
            // Port's SetColorizeImages is a no-op stub, so the alpha-tinted
            // overlay won't actually blend. We draw it anyway so the path
            // is exercised once Graphics gets a real impl.
            g->SetColorizeImages(true);
            g->SetColor(Sexy::Color(255, 255, 255, (int)(mOverAlpha * 255)));
            DrawButtonImage(g, mOverImage, mOverRect, 0, 0);
            g->SetColorizeImages(false);
        }
        else if (isHighLighted && HaveButtonImage(mOverImage, mOverRect))
            DrawButtonImage(g, mOverImage, mOverRect, 0, 0);
        else if (HaveButtonImage(mButtonImage, mNormalRect))
            DrawButtonImage(g, mButtonImage, mNormalRect, 0, 0);

        if (mFont)
        {
            g->SetColor(mColors[isHighLighted ? COLOR_LABEL_HILITE : COLOR_LABEL]);
            g->DrawString(mLabel.c_str(), aFontX, aFontY);
        }

        if (isHighLighted && mOverOverlayImage)
        {
            g->SetDrawMode(Sexy::Graphics::DRAWMODE_ADDITIVE);
            DrawButtonImage(g, mOverOverlayImage, mNormalRect, 0, 0);
            g->SetDrawMode(Sexy::Graphics::DRAWMODE_NORMAL);
        }
    }
    else // isDown
    {
        if (HaveButtonImage(mDownImage, mDownRect))
            DrawButtonImage(g, mDownImage, mDownRect, 0, 0);
        else if (HaveButtonImage(mOverImage, mOverRect))
            DrawButtonImage(g, mOverImage, mOverRect, 1, 1);
        else
            DrawButtonImage(g, mButtonImage, mNormalRect, 1, 1);

        if (mFont)
        {
            g->SetColor(mColors[COLOR_LABEL_HILITE]);
            g->DrawString(mLabel.c_str(), aFontX + 1, aFontY + 1);
        }

        if (isHighLighted && mOverOverlayImage)
        {
            g->SetDrawMode(Sexy::Graphics::DRAWMODE_ADDITIVE);
            DrawButtonImage(g, mOverOverlayImage, mNormalRect, 0, 0);
            g->SetDrawMode(Sexy::Graphics::DRAWMODE_NORMAL);
        }
    }

    // Undo the translate (upstream calls g->Translate(-mX,-mY) but port's
    // Graphics::Translate is float; mTransX/Y is what we actually mutated).
    g->mTransX -= mX;
    g->mTransY -= mY;
}
