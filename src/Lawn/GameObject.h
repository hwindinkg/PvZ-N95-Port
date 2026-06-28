/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "ConstEnums.h"
#include "../engine/Graphics.h"

namespace Sexy { class LawnApp; }
class Board;

using namespace Sexy;

class GameObject
{
public:
    LawnApp*                        mApp;
    Board*                          mBoard;
    int32_t                         mX;
    int32_t                         mY;
    int32_t                         mWidth;
    int32_t                         mHeight;
    bool                            mVisible;
    int32_t                         mRow;
    int32_t                         mRenderOrder;

public:
    /*inline*/                      GameObject();
    /*inline*/ bool                 BeginDraw(Graphics* g);
    /*inline*/ void                 EndDraw(Graphics* g);
    /*inline*/ void                 MakeParentGraphicsFrame(Graphics* g);
};

#endif
