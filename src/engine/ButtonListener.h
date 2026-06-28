/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable SexyAppFramework/widget/ButtonListener.h.
 * Button listener interface for Symbian S60 3rd FP1.
 *
 * C++03, no exceptions.
 */

#ifndef BUTTONLISTENER_H
#define BUTTONLISTENER_H

#include "Common.h"

namespace Sexy
{

class ButtonListener
{
public:
    virtual ~ButtonListener() {}
    virtual void ButtonDepressed(int id) {}
    virtual void ButtonDown(int id) {}
    virtual void ButtonUp(int id, int btn) {}
    virtual void ButtonPressed(int id, int btn) {}
    virtual void ButtonMouseEnter(int id) {}
    virtual void ButtonMouseLeave(int id) {}
    virtual void ButtonMouseMove(int id, int x, int y) {}
};

} // namespace Sexy

#endif // BUTTONLISTENER_H
