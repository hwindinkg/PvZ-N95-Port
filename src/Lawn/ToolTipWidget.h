#ifndef __TOOLTIPWIDGET_H__
#define __TOOLTIPWIDGET_H__

#include <e32def.h>
#include "../engine/Widget.h"

class ToolTipWidget : public Sexy::Widget
{
public:
    ToolTipWidget() : mCenter(false), mVisible(false) {}
    void Draw(Sexy::Graphics* g) { (void)g; }
    void SetWarningText(const std::string& theText) { (void)theText; }
    void SetLabel(const std::string& theLabel) { (void)theLabel; }
    void SetPosition(int theX, int theY) { mX = theX; mY = theY; }

    bool mCenter;
    bool mVisible;
};

#endif
