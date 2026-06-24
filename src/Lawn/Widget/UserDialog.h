#ifndef __USERDIALOG_H__
#define __USERDIALOG_H__
#include "../../engine/Dialog.h"
#include <string>

namespace Sexy { class LawnApp; }

class UserDialog : public Sexy::Dialog
{
public:
    UserDialog() : Sexy::Dialog() {}
    UserDialog(LawnApp* theApp) : Sexy::Dialog(), mApp(theApp) {}
    virtual ~UserDialog() {}
    virtual void Update() {}
    virtual void Draw(Sexy::Graphics* g) { (void)g; }
    std::string GetSelName() { return std::string(); }
    void FinishDeleteUser() {}
    void FinishRenameUser(const std::string&) {}
    LawnApp* mApp;
};
#endif
