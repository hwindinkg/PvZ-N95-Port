/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable SexyAppFramework/widget/DialogListener.h.
 * Dialog listener interface for Symbian S60 3rd FP1.
 *
 * C++03, no exceptions.
 */

#ifndef DIALOGLISTENER_H
#define DIALOGLISTENER_H

#include "Common.h"

namespace Sexy
{

class DialogListener
{
public:
    virtual ~DialogListener() {}
    virtual void DialogButtonDepressed(int id) {}
    virtual void DialogButtonDown(int id) {}
    virtual void DialogButtonUp(int id) {}
    virtual void DialogButtonEnter(int id) {}
    virtual void DialogButtonLeave(int id) {}
    virtual void DialogButtonPress(int id) {}
};

} // namespace Sexy

#endif // DIALOGLISTENER_H
