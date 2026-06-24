#ifndef __MESSAGEWIDGET_H__
#define __MESSAGEWIDGET_H__

#include <e32def.h>
#include "../engine/Widget.h"

namespace Sexy { class LawnApp; }

class MessageWidget : public Sexy::Widget
{
public:
    MessageWidget() : mDuration(0) {}
    MessageWidget(Sexy::LawnApp* theApp) : Sexy::Widget(), mDuration(0) { (void)theApp; }
    void Draw(Sexy::Graphics* g) { (void)g; }
    void SetLabel(const char* label) { (void)label; }
    void SetLabel(const std::string& str, MessageStyle style) { (void)str; (void)style; }
    void ClearLabel() {}
    bool IsBeingDisplayed() { return false; }
    int mDuration;
};

#endif
