#ifndef _LAWNDIALOG_H_
#define _LAWNDIALOG_H_

#include "../../engine/Dialog.h"
#include "../LawnApp.h"

namespace Sexy {

class LawnDialog : public Dialog
{
public:
    LawnDialog(LawnApp* theApp, int theDialogId, bool isModal, const char* theDialogHeader,
               const char* theDialogLines, const char* theDialogFooter, int theButtonMode)
        : Dialog()
        , mApp(theApp)
        , mIsModal(isModal)
    { (void)theDialogId; (void)theDialogHeader; (void)theDialogLines; (void)theDialogFooter; (void)theButtonMode; }

    virtual void Update() {}
    virtual void Draw(Graphics* g) { (void)g; }

    LawnApp* mApp;
    bool     mIsModal;
};

} // namespace Sexy

#endif
