#ifndef __NEWOPTIONSDIALOG_H__
#define __NEWOPTIONSDIALOG_H__

#include "../../engine/Dialog.h"
#include "../../engine/Checkbox.h"

namespace Sexy { class LawnApp; }

class NewOptionsDialog : public Sexy::Dialog
{
public:
    NewOptionsDialog(LawnApp* theApp, bool theFromGameSelector) : Sexy::Dialog() { (void)theApp; (void)theFromGameSelector; }
    virtual ~NewOptionsDialog() {}
    Sexy::Checkbox* mFullscreenCheckbox;
    Sexy::Checkbox* mHardwareAccelerationCheckbox;
};

#endif // __NEWOPTIONSDIALOG_H__
