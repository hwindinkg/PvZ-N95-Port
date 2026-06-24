#ifndef __CHEATDIALOG_H__
#define __CHEATDIALOG_H__
#include "../../engine/Dialog.h"

namespace Sexy { class LawnApp; }

class CheatDialog : public Sexy::Dialog
{
public:
    CheatDialog() : Sexy::Dialog() {}
    CheatDialog(LawnApp* theApp) : Sexy::Dialog(), mApp(theApp) {}
    virtual ~CheatDialog() {}
    virtual void Update() {}
    virtual void Draw(Sexy::Graphics* g) { (void)g; }
    bool ApplyCheat() { return false; }
    LawnApp* mApp;
};
#endif
