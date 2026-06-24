#ifndef __NEWUSERDIALOG_H__
#define __NEWUSERDIALOG_H__

#include "../../engine/Dialog.h"
#include "../../engine/EditWidget.h"
#include <string>

namespace Sexy { class LawnApp; }

class NewUserDialog : public Sexy::Dialog
{
public:
    NewUserDialog(LawnApp* theApp, bool isRename) : Sexy::Dialog() { (void)theApp; (void)isRename; }
    virtual ~NewUserDialog() {}
    std::string GetName() { return std::string(); }
    void SetName(const std::string& theName) { (void)theName; }
    Sexy::EditWidget* mNameEditWidget;
};

#endif // __NEWUSERDIALOG_H__
