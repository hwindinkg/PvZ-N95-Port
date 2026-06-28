#ifndef __ALMANACDIALOG_H__
#define __ALMANACDIALOG_H__

#include "../../engine/Dialog.h"

namespace Sexy { class LawnApp; }

class AlmanacDialog : public Sexy::Dialog
{
public:
    AlmanacDialog() : Sexy::Dialog() {}
    AlmanacDialog(LawnApp* theApp) : Sexy::Dialog(), mApp(theApp) {}
    virtual ~AlmanacDialog() {}
    virtual void Draw(Sexy::Graphics* g) { (void)g; }
    virtual void Update() {}
    void ShowPlant(int) {}
    void ShowZombie(int) {}
    LawnApp* mApp;
};

#endif // __ALMANACDIALOG_H__
