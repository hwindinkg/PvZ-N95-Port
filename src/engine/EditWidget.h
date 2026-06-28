#ifndef __EDITWIDGET_H__
#define __EDITWIDGET_H__

#include "Widget.h"

namespace Sexy {

class EditWidget : public Widget
{
public:
    EditWidget() : Widget() {}
    virtual ~EditWidget() {}
    std::string GetText() const { return mText; }
    void SetText(const std::string& text) { mText = text; }
    std::string mText;
};

} // namespace Sexy

#endif // __EDITWIDGET_H__
