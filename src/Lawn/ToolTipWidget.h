/*
 * ToolTipWidget.h -- M4 #1 simple text tooltip for the main menu.
 *
 * Ported from upstream PvZ-Portable src/Lawn/ToolTipWidget.h but radically
 * simplified -- upstream's version draws a richly-formatted multi-line
 * tooltip with word-wrap, embedded icons, and shadow. This port's font
 * system is stubbed (GetFontThrow stubbed per README M4 #4), so we just
 * draw a single-line label on a semi-transparent black background.
 *
 * COMPAT: the original port stub (M3 era) was in the GLOBAL namespace
 * (not Sexy::), with mCenter/mVisible members and SetWarningText/SetLabel/
 * SetPosition methods. board.cpp accesses mToolTip->mCenter/mVisible/mX/mY
 * directly. To avoid breaking board.cpp, this class:
 *   - stays in the global namespace
 *   - keeps mCenter (unused but public for compat)
 *   - keeps SetWarningText/SetLabel/SetPosition as inline compat stubs
 *   - does NOT redeclare mVisible (Widget::mVisible is used directly now --
 *     the old stub's shadowed mVisible was a latent bug: WidgetManager
 *     checked Widget::mVisible but board.cpp set ToolTipWidget::mVisible)
 *   - adds SetText as the new API
 */
#ifndef __TOOLTIPWIDGET_H__
#define __TOOLTIPWIDGET_H__

#include "../engine/Widget.h"
#include <string>

class ToolTipWidget : public Sexy::Widget
{
public:
    ToolTipWidget();
    virtual ~ToolTipWidget() {}

    virtual void Draw(Sexy::Graphics* g);
    virtual void Update() { Widget::Update(); }

    // New API (M4 #1).
    void SetText(const char* theText);
    const std::string& GetText() const { return mText; }

    // Compat API (preserved so board.cpp keeps compiling).
    void SetWarningText(const std::string& theText) { SetText(theText.c_str()); }
    void SetLabel(const std::string& theLabel) { SetText(theLabel.c_str()); }
    void SetPosition(int theX, int theY) { mX = theX; mY = theY; }

    // Compat member (board.cpp reads/writes this directly).
    bool mCenter;

private:
    std::string mText;
};

#endif // __TOOLTIPWIDGET_H__
