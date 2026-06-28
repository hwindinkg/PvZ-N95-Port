/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable SexyAppFramework/graphics/Graphics.h.
 * GLES 1.1 rendering context for Symbian S60 3rd FP1.
 *
 * Translates high-level drawing calls (FillRect, DrawImage, etc.)
 * into GLInterface vertex submissions with texture caching.
 *
 * C++03, no STL, no exceptions.
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <e32base.h>
#include <gles/gl.h>
#include "Common.h"
#include "Color.h"
#include "Rect.h"
#include "Point.h"
#include "Image.h"
#include "MemoryImage.h"
#include "SexyMatrix.h"

namespace Sexy
{

// Forward declarations
class GLInterface;
class Font;

// ---------------------------------------------------------------------------
// DrawStringJustification  --  text alignment flags (from ConstEnums.h)
// ---------------------------------------------------------------------------
enum DrawStringJustification
{
    DS_ALIGN_LEFT                  = 0,
    DS_ALIGN_RIGHT                 = 1,
    DS_ALIGN_CENTER                = 2,
    DS_ALIGN_LEFT_VERTICAL_MIDDLE  = 3,
    DS_ALIGN_RIGHT_VERTICAL_MIDDLE = 4,
    DS_ALIGN_CENTER_VERTICAL_MIDDLE = 5
};

// ---------------------------------------------------------------------------
// Graphics  --  GLES 1.1 drawing context
// ---------------------------------------------------------------------------
class Graphics
{
public:
    enum
    {
        DRAWMODE_NORMAL   = 0,
        DRAWMODE_ADDITIVE = 1
    };

public:
    Graphics();
    virtual ~Graphics();

    // -- State management ------------------------------------------------
    void SetDrawMode(int theDrawMode);
    int  GetDrawMode() const { return mDrawMode; }
    void SetColor(const Color& theColor);
    const Color& GetColor() const { return mColor; }
    void SetFont(Font* font);
    Font* GetFont() const { return mFont; }

    // -- Clip rect -------------------------------------------------------
    void SetClipRect(const Rect& r);
    const Rect& GetClipRect() const { return mClipRect; }
    void ClearClipRect();

    // -- Transform -------------------------------------------------------
    void PushState();
    void PopState();
    void PushTransform(const SexyMatrix3& m);
    void PopTransform();
    void ClearTransform();
    void Translate(float dx, float dy);

    float mTransX;
    float mTransY;
    float mScaleX;
    float mScaleY;

    // -- Drawing primitives ----------------------------------------------
    void FillRect(int x, int y, int w, int h);
    void DrawRect(int x, int y, int w, int h);
    void ClearRect(int x, int y, int w, int h);

    // -- Image drawing ---------------------------------------------------
    void DrawImage(MemoryImage* img, int x, int y);
    void DrawImage(MemoryImage* img, int x, int y, int dstW, int dstH);
    void DrawImage(MemoryImage* img, int x, int y, const Rect& srcRect);
    // [Session-12] Scaled src-rect draw: draws a sub-rect of the source image
    // scaled to (dstW, dstH). Used by the loading bar to "unroll" the grass.
    void DrawImageScaledSrcRect(MemoryImage* img, int dstX, int dstY,
                                int dstW, int dstH, const Rect& srcRect);
    void DrawImageF(Image* img, float x, float y);
    void DrawImageRotated(Image* img, float x, float y,
                          const Rect& srcRect, float rot,
                          float centerX, float centerY);

    // -- Text drawing (placeholder) --------------------------------------
    void DrawString(const char* text, int x, int y);
    void DrawString(const char* text, int x, int y, int justify);
    void SetColorizeImages(bool val) { (void)val; }
    void SetScale(float theScaleX, float theScaleY) { mScaleX = theScaleX; mScaleY = theScaleY; }
    // Transform-style SetScale (used by plant.cpp)
    void SetScale(float theScaleX, float theScaleY, float, float) { mScaleX = theScaleX; mScaleY = theScaleY; }
    void DrawImageCel(Image* img, int x, int y, int cel, int cel2) { (void)img; (void)x; (void)y; (void)cel; (void)cel2; }
    void DrawImageMirror(Image* img, const Rect& destRect, const Rect& srcRect, bool mirror) { (void)img; (void)destRect; (void)srcRect; (void)mirror; }
    void SetLinearBlend(bool val) { (void)val; }
    void DrawImage(Image* img, int x, int y) { DrawImageF(img, (float)x, (float)y); }
    void DrawImage(Image* img, int x, int y, const Rect& srcRect) { (void)img; (void)x; (void)y; (void)srcRect; }

    // -- Accessors -------------------------------------------------------
    int  GetWidth()  const { return mWidth;  }
    int  GetHeight() const { return mHeight; }
    void SetWidth(int w)   { mWidth = w;     }
    void SetHeight(int h)  { mHeight = h;    }
    void SetGLInterface(GLInterface* gl) { mGL = gl; }
    GLInterface* GetGLInterface() const { return mGL; }

private:
    // -- Internal helpers ------------------------------------------------
    void ApplyClipRect();
    void DisableClipRect();
    GLuint GetOrCreateTexture(MemoryImage* img);
    void   DrawTextPlaceholder(const char* text, int x, int y);

public:
    // -- State -----------------------------------------------------------
    Color       mColor;
    int         mDrawMode;
    Rect        mClipRect;
    TBool       mClipRectSet;
    Font*       mFont;

    // -- Rendering -------------------------------------------------------
    GLInterface* mGL;

    // -- Logical surface dimensions --------------------------------------
    int mWidth;
    int mHeight;
};

} // namespace Sexy

#endif // GRAPHICS_H
