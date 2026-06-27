/*
 * ToolTipWidget.cpp -- M4 #1 simple text tooltip.
 */
#include "ToolTipWidget.h"
#include "../engine/Graphics.h"
#include "../engine/Color.h"
#include "../engine/Font.h"
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

    // Text -- best-effort; if FONT_DWARVEN is NULL (fonts stubbed), the
    // tooltip just shows the rectangle. Once fonts are wired the label
    // appears.
    //
    // NOTE: in this port, FONT_DWARVENTODCRAFT18GREENINSET etc. are
    // _Font* (opaque), NOT Sexy::Font*. The only Sexy::Font* globals
    // available are FONT_DWARVEN and FONT_COUNTER (declared in
    // engine/Font.h). Use FONT_DWARVEN here.
    Sexy::Font* font = Sexy::FONT_DWARVEN;
    if (font)
    {
        g->SetFont(font);
        g->SetColor(Sexy::Color(255, 255, 255, 255));
        int tw = font->StringWidth(mText.c_str());
        int tx = mX + (mWidth - tw) / 2;
        int ty = mY + (mHeight + font->GetAscent()) / 2 - 2;
        g->DrawString(mText.c_str(), tx, ty);
    }
}
