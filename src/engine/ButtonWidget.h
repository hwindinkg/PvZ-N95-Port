#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include "Widget.h"
#include "ButtonListener.h"
#include <string>

namespace Sexy {

class ButtonWidget : public Widget
{
public:
    ButtonWidget(int theId, ButtonListener* theListener)
        : Widget(), mId(theId), mListener(theListener), mHasDrawn(false) {}

    virtual void Draw(Graphics* g) { (void)g; }

    int mId;
    ButtonListener* mListener;
    bool mHasDrawn;
    std::string mLabel;
};

} // namespace Sexy

#endif // BUTTONWIDGET_H