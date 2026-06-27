/*
 * ToolTipWidget.cpp -- M4 #1 simple text tooltip.
 */
#include "ToolTipWidget.h"
#include "../engine/Graphics.h"
#include "../engine/Color.h"
#include "../engine/Font.h"
#include "../engine/SystemFont.h"  // for tooltip text
#include "../Resources.h"
#include <string.h>

ToolTipWidget::ToolTipWidget()
    : Sexy::Widget()
    , mCenter(false)
    , mText()
{
    mVisible = false;       // hidden by default; GameSelector shows it on hover
    mMouseVisible = false;  // don't capture hits -- pass through to buttons
    mHasTransparencies = true;
    mDisabled = false;
    mWantsFocus = false;
}

void ToolTipWidget::SetText(const char* theText)
{
    if (theText == NULL) theText = "";
    if (mText != theText)
        mText = theText;
}

void ToolTipWidget::Draw(Sexy::Graphics* g)
{
    if (g == NULL) return;
    if (!mVisible) return;
    if (mText.empty()) return;

    // Semi-transparent black background so the text is readable over any
    // background image.
    g->SetColor(Sexy::Color(0, 0, 0, 180));
    g->FillRect(mX, mY, mWidth, mHeight);

    // Thin border.
    g->SetColor(Sexy::Color(200, 200, 200, 200));
    g->DrawRect(mX, mY, mWidth, mHeight);

    // Text -- use SystemFont (8x8 bitmap fallback) since PvZ font assets
    // are not in the PAK (M4 #4).
    Sexy::SystemFont* font = Sexy::SystemFont::Get();
    if (font)
    {
        g->SetFont(font);
        g->SetColor(Sexy::Color(255, 255, 255, 255));
        int tw = font->StringWidth(mText.c_str());
        int tx = mX + (mWidth - tw) / 2;
        int ty = mY + (mHeight + 8) / 2 - 2;
        g->DrawString(mText.c_str(), tx, ty);
    }
}
