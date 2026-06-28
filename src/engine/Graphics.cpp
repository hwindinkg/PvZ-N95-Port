/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable SexyAppFramework/graphics/Graphics.cpp.
 * GLES 1.1 fixed-pipeline rendering via GLInterface.
 *
 * Features:
 *   - Texture cache: MemoryImage -> GLuint
 *   - Clip rect via glScissor
 *   - Coloured rects, textured quads, placeholder text
 *   - Transform stack for DrawImageRotated
 *
 * C++03, no STL, no exceptions.
 */

#include "Graphics.h"
#include "Font.h"   // for Font::DrawString (forward-declared in Graphics.h)
#include "GLInterface.h"
#include "SexyMatrix.h"
#include <e32debug.h>
#include <f32file.h>
#include "Image.h"
#include <string.h>
#include <math.h>

// ===========================================================================
// Texture cache  (fixed-size array, no STL)
// ===========================================================================
namespace {

struct TexCacheEntry
{
    const Sexy::MemoryImage* mImage;
    GLuint  mTexID;
    float   mUMax;   // = imgW / potW  (POT padding ratio)
    float   mVMax;   // = imgH / potH
};

// PowerVR MBX (N95, GL ES 1.1) requires power-of-two textures. Round up.
static TInt NextPow2(TInt v)
{
    TInt p = 1;
    while (p < v) p <<= 1;
    return p;
}

static const TInt KMaxTexCache = 512;

static TexCacheEntry sTexCache[KMaxTexCache];
static TInt          sTexCount = 0;

GLuint FindCachedTexture(const Sexy::MemoryImage* img)
{
    for (TInt i = 0; i < sTexCount; ++i)
    {
        if (sTexCache[i].mImage == img)
            return sTexCache[i].mTexID;
    }
    return 0;
}

TBool FindCachedTexCoords(const Sexy::MemoryImage* img, float& uMax, float& vMax)
{
    for (TInt i = 0; i < sTexCount; ++i)
    {
        if (sTexCache[i].mImage == img)
        {
            uMax = sTexCache[i].mUMax;
            vMax = sTexCache[i].mVMax;
            return ETrue;
        }
    }
    uMax = 1.0f; vMax = 1.0f;
    return EFalse;
}

void AddCachedTexture(const Sexy::MemoryImage* img, GLuint texID, float uMax, float vMax)
{
    if (sTexCount >= KMaxTexCache)
    {
        // Cache full -- evict the oldest entry
        --sTexCount;
        // Shift remaining entries (simple FIFO eviction)
        for (TInt i = 0; i < sTexCount; ++i)
            sTexCache[i] = sTexCache[i + 1];
    }

    sTexCache[sTexCount].mImage = img;
    sTexCache[sTexCount].mTexID = texID;
    sTexCache[sTexCount].mUMax  = uMax;
    sTexCache[sTexCount].mVMax  = vMax;
    ++sTexCount;
}

} // anonymous namespace

// ===========================================================================
// Graphics implementation
// ===========================================================================

namespace Sexy {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

Graphics::Graphics()
    : mColor(255, 255, 255)
    , mDrawMode(DRAWMODE_NORMAL)
    , mClipRectSet(EFalse)
    , mFont(NULL)
    , mGL(NULL)
    , mWidth(320)
    , mHeight(240)
    , mTransX(0)
    , mTransY(0)
    , mScaleX(1.0f)
    , mScaleY(1.0f)
{
}

Graphics::~Graphics()
{
}

// ---------------------------------------------------------------------------
// State management
// ---------------------------------------------------------------------------

void Graphics::SetDrawMode(int theDrawMode)
{
    mDrawMode = theDrawMode;
    if (mGL)
        mGL->SetDrawMode(static_cast<DrawMode>(theDrawMode));
}

void Graphics::SetColor(const Color& theColor)
{
    mColor = theColor;
    // Log first call to confirm Graphics rendering
    static TInt first = 1;
    if (first) {
        first = 0;
        RFs fs; RFile f;
        if (fs.Connect() == KErrNone) {
            fs.MkDirAll(_L("C:\\Data\\PvZ"));
            if (f.Replace(fs, _L("C:\\Data\\PvZ\\gfx_log.txt"), EFileWrite) == KErrNone) {
                _LIT8(KMsg, "SetColor called\n");
                f.Write(KMsg); f.Close();
            }
            fs.Close();
        }
    }
    if (mGL)
    {
        mGL->SetColor(
            theColor.mRed   / 255.0f,
            theColor.mGreen / 255.0f,
            theColor.mBlue  / 255.0f,
            theColor.mAlpha / 255.0f);
    }
}

void Graphics::SetFont(Font* font)
{
    mFont = font;
}

// ---------------------------------------------------------------------------
// Clip rect
// ---------------------------------------------------------------------------

void Graphics::SetClipRect(const Rect& r)
{
    mClipRect    = r;
    mClipRectSet = ETrue;
}

void Graphics::ClearClipRect()
{
    mClipRectSet = EFalse;
}

void Graphics::ApplyClipRect()
{
    if (!mClipRectSet || !mGL)
        return;

    glEnable(GL_SCISSOR_TEST);

    // GL scissor uses window coordinates (origin = bottom-left).
    // Our projection has origin at top-left, so flip Y.
    TInt sx = mClipRect.mX;
    TInt sy = mHeight - mClipRect.mY - mClipRect.mHeight;
    TInt sw = mClipRect.mWidth;
    TInt sh = mClipRect.mHeight;

    if (sx < 0)      { sw += sx; sx = 0; }
    if (sy < 0)      { sh += sy; sy = 0; }
    if (sw < 0) sw = 0;
    if (sh < 0) sh = 0;

    glScissor(sx, sy, sw, sh);
}

void Graphics::DisableClipRect()
{
    if (mClipRectSet)
        glDisable(GL_SCISSOR_TEST);
}

// ---------------------------------------------------------------------------
// Transform support
// ---------------------------------------------------------------------------

void Graphics::PushTransform(const SexyMatrix3& m)
{
    if (mGL)
        mGL->PushTransform(m);
}

void Graphics::PopTransform()
{
    if (mGL)
        mGL->PopTransform();
}

void Graphics::ClearTransform()
{
    if (mGL)
        mGL->LoadIdentity();
}

void Graphics::Translate(float dx, float dy)
{
    mTransX += dx;
    mTransY += dy;
    if (mGL)
        mGL->Translate(dx, dy);
}

void Graphics::PushState()
{
    // Save current transform and clip state
    // Placeholder for now
}

void Graphics::PopState()
{
    // Restore transform and clip state
    // Placeholder for now
}

// ---------------------------------------------------------------------------
// Texture cache helper
// ---------------------------------------------------------------------------

GLuint Graphics::GetOrCreateTexture(MemoryImage* img)
{
    if (!img || !mGL)
        return 0;

    // Look up in cache
    GLuint texID = FindCachedTexture(img);
    if (texID)
        return texID;

    TInt w = img->GetWidth();
    TInt h = img->GetHeight();
    if (w <= 0 || h <= 0)
        return 0;

    // PowerVR MBX (N95, GLES 1.1) supports ONLY power-of-two textures.
    // Uploading an NPOT image (e.g. 800x600 titlescreen) and sampling it
    // reads out of bounds -> KERN-EXEC 3 on the first draw. Allocate a POT
    // texture, sub-upload the image top-left, map texcoords to w/potW, h/potH.
    TInt potW = NextPow2(w);
    TInt potH = NextPow2(h);

    texID = mGL->CreateTexture(potW, potH, NULL, GL_RGBA);
    if (!texID)
        return 0;

    if (img->GetBits())
    {
        // Attempt sub-image upload (via glTexSubImage2DOES if available).
        // If that fails (extension not present on the N95 MBX driver), fall
        // back to a manual full-POT upload with zero-padding.  DO NOT let
        // CopyImageToTextureSub reach its fallback (CopyImageToTexture),
        // which calls glTexImage2D with the image's NPOT size -> PowerVR MBX
        // doesn't support NPOT -> texture content never renders.
        if (!mGL->CopyImageToTextureSub(img, texID, 0, 0))
        {
            // glTexSubImage2DOES not available — upload full POT buffer
            // with the image placed top-left and unused pixels zero-filled.
            //
            // MemoryImage bits are stored as 0xAARRGGBB words (little-endian
            // memory bytes: B, G, R, A). GL_UNSIGNED_BYTE + GL_RGBA expects
            // memory bytes R, G, B, A. So we MUST swap R and B via
            // ArgbToRgba -- without it, red and blue channels are flipped
            // (sky becomes yellow, red becomes blue).
            TInt potPixelCount = potW * potH;
            TUint32* rgbaBuf = new TUint32[potPixelCount];
            if (rgbaBuf)
            {
                memset(rgbaBuf, 0, potPixelCount * sizeof(TUint32));
                TUint32* srcBits = reinterpret_cast<TUint32*>(img->GetBits());
                for (TInt row = 0; row < h; ++row)
                {
                    GLInterface::ArgbToRgba(
                        srcBits + row * w,
                        rgbaBuf + row * potW,
                        w);
                }
                glBindTexture(GL_TEXTURE_2D, texID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             potW, potH, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, rgbaBuf);
                delete[] rgbaBuf;
            }
        }
    }

    float uMax = (float)w / (float)potW;
    float vMax = (float)h / (float)potH;
    AddCachedTexture(img, texID, uMax, vMax);

    return texID;
}

// ---------------------------------------------------------------------------
// FillRect  --  coloured quad (GL_TRIANGLE_STRIP)
// ---------------------------------------------------------------------------

void Graphics::FillRect(int x, int y, int w, int h)
{
    if (!mGL || w <= 0 || h <= 0)
        return;

    ApplyClipRect();

    mGL->SetTextureEnabled(EFalse);

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);

    mGL->Begin(GL_TRIANGLE_STRIP);
    mGL->AddVertex(fx,      fy);
    mGL->AddVertex(fx + fw, fy);
    mGL->AddVertex(fx,      fy + fh);
    mGL->AddVertex(fx + fw, fy + fh);
    mGL->End();

    DisableClipRect();
}

// ---------------------------------------------------------------------------
// DrawRect  --  rectangle outline (GL_LINE_LOOP)
// ---------------------------------------------------------------------------

void Graphics::DrawRect(int x, int y, int w, int h)
{
    if (!mGL || w <= 0 || h <= 0)
        return;

    ApplyClipRect();

    mGL->SetTextureEnabled(EFalse);

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);

    mGL->Begin(GL_LINE_LOOP);
    mGL->AddVertex(fx,      fy);
    mGL->AddVertex(fx + fw, fy);
    mGL->AddVertex(fx + fw, fy + fh);
    mGL->AddVertex(fx,      fy + fh);
    mGL->End();

    DisableClipRect();
}

// ---------------------------------------------------------------------------
// ClearRect  --  fill with black
// ---------------------------------------------------------------------------

void Graphics::ClearRect(int x, int y, int w, int h)
{
    Color old = mColor;
    SetColor(Color(0, 0, 0));
    FillRect(x, y, w, h);
    SetColor(old);
}

// ---------------------------------------------------------------------------
// DrawImage  (MemoryImage*, int, int)  --  full image
// ---------------------------------------------------------------------------

void Graphics::DrawImage(MemoryImage* img, int x, int y)
{
    if (!mGL || !img)
        return;

    GLuint texID = GetOrCreateTexture(img);
    if (!texID)
    {
        // Fallback: draw white placeholder
        Color old = mColor;
        SetColor(Color(255, 255, 255));
        FillRect(x, y, img->GetWidth(), img->GetHeight());
        SetColor(old);
        return;
    }

    ApplyClipRect();

    mGL->SetTexture(texID);
    mGL->SetTextureEnabled(ETrue);

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fw = static_cast<float>(img->GetWidth());
    float fh = static_cast<float>(img->GetHeight());

    // Normalise texture coordinates: assume source image fills the
    // POT-padded texture: sample only the used sub-region.
    float uMax = 1.0f, vMax = 1.0f;
    FindCachedTexCoords(img, uMax, vMax);
    mGL->Begin(GL_TRIANGLE_STRIP);
    mGL->AddVertex(fx,      fy,      0.0f, 0.0f);
    mGL->AddVertex(fx + fw, fy,      uMax, 0.0f);
    mGL->AddVertex(fx,      fy + fh, 0.0f, vMax);
    mGL->AddVertex(fx + fw, fy + fh, uMax, vMax);
    mGL->End();

    DisableClipRect();
}

// ---------------------------------------------------------------------------
// DrawImage  (MemoryImage*, int, int, int, int)  --  scaled to destination
// ---------------------------------------------------------------------------

void Graphics::DrawImage(MemoryImage* img, int dstX, int dstY, int dstW, int dstH)
{
    if (!mGL || !img || dstW <= 0 || dstH <= 0)
        return;

    GLuint texID = GetOrCreateTexture(img);
    if (!texID)
    {
        Color old = mColor;
        SetColor(Color(255, 255, 255));
        FillRect(dstX, dstY, dstW, dstH);
        SetColor(old);
        return;
    }

    ApplyClipRect();

    mGL->SetTexture(texID);
    mGL->SetTextureEnabled(ETrue);

    float fx = static_cast<float>(dstX);
    float fy = static_cast<float>(dstY);
    float fw = static_cast<float>(dstW);
    float fh = static_cast<float>(dstH);

    float uMax = 1.0f, vMax = 1.0f;
    FindCachedTexCoords(img, uMax, vMax);
    mGL->Begin(GL_TRIANGLE_STRIP);
    mGL->AddVertex(fx,      fy,      0.0f, 0.0f);
    mGL->AddVertex(fx + fw, fy,      uMax, 0.0f);
    mGL->AddVertex(fx,      fy + fh, 0.0f, vMax);
    mGL->AddVertex(fx + fw, fy + fh, uMax, vMax);
    mGL->End();

    DisableClipRect();
}

// ---------------------------------------------------------------------------
// DrawImage  (MemoryImage*, int, int, const Rect&)  --  sub-rect
// ---------------------------------------------------------------------------

void Graphics::DrawImage(MemoryImage* img, int x, int y, const Rect& srcRect)
{
    if (!mGL || !img || srcRect.mWidth <= 0 || srcRect.mHeight <= 0)
        return;

    GLuint texID = GetOrCreateTexture(img);
    if (!texID)
    {
        Color old = mColor;
        SetColor(Color(255, 255, 255));
        FillRect(x, y, srcRect.mWidth, srcRect.mHeight);
        SetColor(old);
        return;
    }

    ApplyClipRect();

    mGL->SetTexture(texID);
    mGL->SetTextureEnabled(ETrue);

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fw = static_cast<float>(srcRect.mWidth);
    float fh = static_cast<float>(srcRect.mHeight);

    // Compute UV coordinates for the sub-rect
    float imgW = static_cast<float>(img->GetWidth());
    float imgH = static_cast<float>(img->GetHeight());
    float uMaxR = 1.0f, vMaxR = 1.0f;
    FindCachedTexCoords(img, uMaxR, vMaxR);
    float u0 = (static_cast<float>(srcRect.mX) / imgW) * uMaxR;
    float v0 = (static_cast<float>(srcRect.mY) / imgH) * vMaxR;
    float u1 = (static_cast<float>(srcRect.mX + srcRect.mWidth)  / imgW) * uMaxR;
    float v1 = (static_cast<float>(srcRect.mY + srcRect.mHeight) / imgH) * vMaxR;

    mGL->Begin(GL_TRIANGLE_STRIP);
    mGL->AddVertex(fx,      fy,      u0, v0);
    mGL->AddVertex(fx + fw, fy,      u1, v0);
    mGL->AddVertex(fx,      fy + fh, u0, v1);
    mGL->AddVertex(fx + fw, fy + fh, u1, v1);
    mGL->End();

    DisableClipRect();
}

// ---------------------------------------------------------------------------
// DrawImageF  (Image*, float, float)
// ---------------------------------------------------------------------------

void Graphics::DrawImageF(Image* img, float x, float y)
{
    // Cast to MemoryImage* for texture operations.
    MemoryImage* memImg = static_cast<MemoryImage*>/*RTTI off under GCCE; RM only stores MemoryImage*/(img);
    if (memImg)
    {
        DrawImage(memImg, static_cast<int>(x), static_cast<int>(y));
    }
    else if (img)
    {
        // Non-memory-backed Image: draw placeholder
        Color old = mColor;
        SetColor(Color(255, 255, 255));
        FillRect(static_cast<int>(x), static_cast<int>(y),
                 img->GetWidth(), img->GetHeight());
        SetColor(old);
    }
}

// ---------------------------------------------------------------------------
// DrawImageRotated  --  texture-mapped quad with rotation transform
// ---------------------------------------------------------------------------

void Graphics::DrawImageRotated(Image* img, float x, float y,
                                 const Rect& srcRect, float rot,
                                 float centerX, float centerY)
{
    if (!mGL || !img)
        return;

    // Build a combined transform: T(x+centerX, y+centerY) * R(rot) * T(-centerX, -centerY)
    float c = cos(rot);
    float s = sin(rot);

    SexyMatrix3 mat;
    mat.m00 = c;
    mat.m01 = -s;
    mat.m02 = x + centerX - centerX * c + centerY * s;
    mat.m10 = s;
    mat.m11 =  c;
    mat.m12 = y + centerY - centerX * s - centerY * c;
    mat.m20 = 0.0f;
    mat.m21 = 0.0f;
    mat.m22 = 1.0f;

    mGL->PushTransform(mat);

    // Draw the image at the origin offset so centre aligns
    MemoryImage* memImg = static_cast<MemoryImage*>/*RTTI off under GCCE; RM only stores MemoryImage*/(img);
    if (memImg)
        DrawImage(memImg, static_cast<int>(-centerX),
                          static_cast<int>(-centerY), srcRect);
    else
        FillRect(static_cast<int>(-centerX), static_cast<int>(-centerY),
                 srcRect.mWidth, srcRect.mHeight);

    mGL->PopTransform();
}

// ---------------------------------------------------------------------------
// DrawString  --  dispatches to mFont->DrawString (was placeholder white rect)
// ---------------------------------------------------------------------------

void Graphics::DrawString(const char* text, int x, int y)
{
    DrawString(text, x, y, DS_ALIGN_LEFT);
}

void Graphics::DrawString(const char* text, int x, int y, int justify)
{
    if (!mGL || !text || !mFont)
        return;

    // Dispatch to the font's DrawString method. The font (SystemFont)
    // renders each glyph as pixel rects using the current mColor.
    // Justify is handled by the caller (GameButton etc.) which computes
    // x/y offsets before calling DrawString.
    mFont->DrawString(this, x, y, text, -1, NULL);
}

} // namespace Sexy