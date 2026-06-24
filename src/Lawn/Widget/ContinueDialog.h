#ifndef __CONTINUEDIALOG_H__
#define __CONTINUEDIALOG_H__

#include "../../engine/Dialog.h"

namespace Sexy { class LawnApp; }

class ContinueDialog : public Sexy::Dialog
{
public:
    ContinueDialog(LawnApp* theApp) : Sexy::Dialog() { (void)theApp; }
    virtual ~ContinueDialog() {}
};

#endif // __CONTINUEDIALOG_H__
