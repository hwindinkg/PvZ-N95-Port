#ifndef __CHECKBOX_H__
#define __CHECKBOX_H__

#include "Widget.h"

namespace Sexy {

class Checkbox : public Widget
{
public:
    Checkbox() : Widget(), mChecked(false) {}
    virtual ~Checkbox() {}
    bool IsChecked() const { return mChecked; }
    void SetChecked(bool checked) { mChecked = checked; }
    bool mChecked;
};

} // namespace Sexy

#endif // __CHECKBOX_H__
