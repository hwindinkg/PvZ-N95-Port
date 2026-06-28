/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable. Symbian S60 3rd FP1 (GCCE, C++03).
 */

#include "GameObject.h"
#include "LawnApp.h"

GameObject::GameObject()
{
    mApp = gLawnApp;
    mBoard = gLawnApp->mBoard;
    mX = 0;
    mY = 0;
    mWidth = 0;
    mHeight = 0;
    mVisible = true;
    mRow = -1;
    mRenderOrder = RENDER_LAYER_TOP;
}

bool GameObject::BeginDraw(Graphics* g)
{
    if (!mVisible)
        return false;

    g->Translate(mX, mY);
    return true;
}

void GameObject::EndDraw(Graphics* g)
{
    g->Translate(-mX, -mY);
}

void GameObject::MakeParentGraphicsFrame(Graphics* g)
{
    g->Translate(-mX, -mY);
}
