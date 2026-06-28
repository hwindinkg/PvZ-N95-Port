// GLInterface.cpp
// Full GLES 1.1 fixed-pipeline implementation for Symbian S60 3rd FP1.
//
// Uses interleaved vertex arrays for batched 2D rendering through
// the GLES 1.1 fixed-function pipeline.  No VBOs, no shaders.
//
// C++03 compatible, no exceptions, no STL.
// Symbian integer types (TInt, TBool, TUint32).

#include "GLInterface.h"
#include "GLPlatform.h"
#include "SexyMatrix.h"
#include "MemoryImage.h"
#include <e32debug.h>
#include <f32file.h>

// File logging helper (writes to C:\Data\PvZ\gl_log.txt)
static void GLTrace(const char* aMsg)
{
    TBuf8<256> buf;
    TPtrC8 msg8((const TUint8*)aMsg);
    buf.Copy(msg8);
    buf.Append('\n');
    RFs fs;
    if (fs.Connect() != KErrNone) return;
    RFile f;
    TInt err = f.Open(fs, _L("C:\\Data\\PvZ\\gl_log.txt"), EFileWrite|EFileShareAny);
    if (err == KErrNotFound)
        err = f.Create(fs, _L("C:\\Data\\PvZ\\gl_log.txt"), EFileWrite|EFileShareAny);
    if (err == KErrNone) {
        TInt pos = 0;
        f.Seek(ESeekEnd, pos);
        f.Write(buf);
        f.Close();
    }
    fs.Close();
}

// ---------------------------------------------------------------------------
// Define the extension function pointer storage declared in GLPlatform.h
// ---------------------------------------------------------------------------
// glDrawTexiOES is built-in on Symbian GLES driver
    // PFNGLDRAWTEXIOES       glDrawTexiOES       = NULL;
PFNGLTEXSUBIMAGE2DOES  glTexSubImage2DOES  = NULL;
PFNGLMAPBUFFEROES      glMapBufferOES      = NULL;
PFNGLUNMAPBUFFEROES    glUnmapBufferOES    = NULL;

namespace Sexy {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

GLInterface::GLInterface()
    : mScreenImage(NULL)
    , mVertexCount(0)
    , mCurrentMode(0)
    , mCurrentTexture(0)
    , mTextureEnabled(EFalse)
    , mDrawMode(DRAWMODE_NORMAL)
    , mTransformDepth(0)
    , mTransformDirty(ETrue)
    , mViewX(0)
    , mViewY(0)
    , mViewW(400)
    , mViewH(300)
    , mColorR(1.0f)
    , mColorG(1.0f)
    , mColorB(1.0f)
    , mColorA(1.0f)
    , mInitialized(EFalse)
{
    mTransformStack[0].Identity();

    TInt i;
    for (i = 0; i < MAX_VERTICES; i++)
    {
        mVertices[i].x = 0.0f;
        mVertices[i].y = 0.0f;
        mVertices[i].u = 0.0f;
        mVertices[i].v = 0.0f;
        mVertices[i].r = 1.0f;
        mVertices[i].g = 1.0f;
        mVertices[i].b = 1.0f;
        mVertices[i].a = 1.0f;
    }
}

GLInterface::~GLInterface()
{
    // The caller is responsible for deleting mScreenImage when appropriate.
    // We do not own it -- the application layer manages the lifecycle.
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

TBool GLInterface::Init()
{
    if (mInitialized)
    {
        return ETrue;
    }

    // Load any optional GLES extensions via eglGetProcAddress
    PlatformGLInit();

    // Set up GLES 1.1 fixed-pipeline state
    SetupGLState();

    // Create a screen-sized backing image for readbacks / offscreen use.
    mScreenImage = new MemoryImage();
    if (mScreenImage)
    {
        mScreenImage->SetSize(400, 300);
    }

    mInitialized = ETrue;
    return ETrue;
}

void GLInterface::SetupGLState()
{
    // --- GLES 1.1 fixed-pipeline state setup ---

    // Start with texturing DISABLED so the cached mTextureEnabled (EFalse,
    // set in the constructor) matches the real GL state. If we enabled
    // GL_TEXTURE_2D here, the first FillRect's SetTextureEnabled(EFalse) would
    // early-return (cache already EFalse) and leave texturing ON with NO
    // texture bound -> glDrawArrays samples a null texture -> KERN-EXEC 3 on
    // PowerVR MBX (N95). Keep it off until something actually binds a texture.
    glDisable(GL_TEXTURE_2D);

    // Disable features we do not use
    glDisable(GL_DITHER);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);

    // [Session-13] Enable GL_ALPHA_TEST for 1-bit transparency (cutout).
    // PvZ sprites use alpha masks (0=transparent, 255=opaque). GL_BLEND with
    // GL_SRC_ALPHA should handle this, but on the N95 MBX driver blending
    // can be unreliable. GL_ALPHA_TEST discards fragments with alpha < 128
    // BEFORE blending, giving clean cutout edges. This is the approach used
    // by re3-symbian (GTA3 port) for the same hardware.
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);

    // Alpha blending (for smooth edges + fade effects)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Vertex + colour arrays are always used. The TEXTURE_COORD array is
    // toggled in lockstep with GL_TEXTURE_2D by SetTextureEnabled() -- on MBX,
    // having the texcoord array enabled while texturing is off (no texture
    // bound) can fault inside glDrawArrays. Start it disabled.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Texture environment: modulate texture with vertex colour
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Default clear colour (black) — [Session-12] was (0,0,0,1) which is
    // correct, but some frames showed purple because the clear colour was
    // being overridden. Keep it black so frames without content show black
    // (not purple).
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Set up orthographic projection matching PvZ logical coords (400x300)
    SetProjection(0.0f, 0.0f, 400.0f, 300.0f);
}

void GLInterface::Redraw()
{
    // Called at the end of each frame.
    // EGL buffer swapping is handled externally by the application's
    // windowing code (CPvZGameView::Draw), so nothing is needed here.
}

// ---------------------------------------------------------------------------
// Matrix operations
// ---------------------------------------------------------------------------

void GLInterface::SetProjection(float left, float top, float right, float bottom)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Map logical pixel coordinates to GL clip space.
    // (0,0) maps to top-left; y increases downward.
    // left=0, right=240, bottom=320, top=0  => identity-like mapping.
    glOrthof(left, right, bottom, top, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLInterface::PushTransform(const SexyMatrix3& m)
{
    if (mTransformDepth < MAX_TRANSFORMS - 1)
    {
        SexyMatrix3 result;
        result.Multiply(mTransformStack[mTransformDepth], m);
        mTransformDepth++;
        mTransformStack[mTransformDepth] = result;
        mTransformDirty = ETrue;
    }
}

void GLInterface::PopTransform()
{
    if (mTransformDepth > 0)
    {
        mTransformDepth--;
        mTransformDirty = ETrue;
    }
}

void GLInterface::LoadIdentity()
{
    mTransformDepth = 0;
    mTransformStack[0].Identity();
    mTransformDirty = ETrue;
}

void GLInterface::Translate(float x, float y)
{
    // Build a translation matrix and multiply into current transform
    SexyMatrix3 trans;
    trans.LoadIdentity();
    trans.m02 = x;
    trans.m12 = y;
    mTransformStack[mTransformDepth].Multiply(mTransformStack[mTransformDepth], trans);
    mTransformDirty = ETrue;
}

void GLInterface::ApplyTransform()
{
    if (!mTransformDirty)
    {
        return;
    }

    mTransformStack[mTransformDepth].ToGLMatrix(mGLMatrix);
    glLoadMatrixf(mGLMatrix);
    mTransformDirty = EFalse;
}

// ---------------------------------------------------------------------------
// Viewport
// ---------------------------------------------------------------------------

void GLInterface::SetViewport(TInt x, TInt y, TInt w, TInt h)
{
    mViewX = x;
    mViewY = y;
    mViewW = w;
    mViewH = h;
    glViewport(x, y, w, h);
}

void GLInterface::UpdateViewport(TInt drawW, TInt drawH)
{
    // Centre the viewport while maintaining a 4:3 aspect ratio.
    TInt vpW = drawW;
    TInt vpH = drawH;

    if (drawW * 3 > drawH * 4)
    {
        // Wider than 4:3 -- letterbox on the sides
        vpW = drawH * 4 / 3;
    }
    else if (drawW * 3 < drawH * 4)
    {
        // Taller than 4:3 -- letterbox on top and bottom
        vpH = drawW * 3 / 4;
    }

    TInt vpX = (drawW - vpW) / 2;
    TInt vpY = (drawH - vpH) / 2;

    SetViewport(vpX, vpY, vpW, vpH);
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

void GLInterface::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

// ---------------------------------------------------------------------------
// Draw mode (blend state)
// ---------------------------------------------------------------------------

void GLInterface::SetDrawMode(DrawMode mode)
{
    if (mDrawMode == mode)
    {
        return;
    }

    FlushIfOverBudget();
    mDrawMode = mode;

    if (mode == DRAWMODE_NORMAL)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        // DRAWMODE_ADDITIVE
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
}

// ---------------------------------------------------------------------------
// Colour state
// ---------------------------------------------------------------------------

void GLInterface::SetColor(float r, float g, float b, float a)
{
    static TBool logged = EFalse;
    if (!logged) { logged = ETrue; GLTrace("GLI::SetColor called"); }
    mColorR = r;
    mColorG = g;
    mColorB = b;
    mColorA = a;
}

// ---------------------------------------------------------------------------
// Texture state
// ---------------------------------------------------------------------------

void GLInterface::SetTexture(GLuint texID)
{
    if (mCurrentTexture != texID)
    {
        FlushIfOverBudget();
        mCurrentTexture = texID;
        glBindTexture(GL_TEXTURE_2D, texID);
    }
}

void GLInterface::SetTextureEnabled(TBool enabled)
{
    TBool en = (enabled != EFalse);
    if (mTextureEnabled == en)
    {
        return;
    }

    FlushIfOverBudget();
    mTextureEnabled = en;

    if (en)
    {
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

// ---------------------------------------------------------------------------
// Vertex submission (immediate-mode style via vertex arrays)
// ---------------------------------------------------------------------------

void GLInterface::Begin(GLenum mode)
{
    // If there is unflushed geometry from a previous Begin/End pair,
    // flush it first.
    if (mVertexCount > 0)
    {
        Flush();
    }

    mCurrentMode = mode;
    mVertexCount = 0;
}

void GLInterface::AddVertex(float x, float y, float u, float v)
{
    if (mVertexCount >= MAX_VERTICES)
    {
        // Flush current batch and start a new one.
        Flush();
    }

    TVertex* vx = &mVertices[mVertexCount];
    vx->x = x;
    vx->y = y;
    vx->u = u;
    vx->v = v;
    vx->r = mColorR;
    vx->g = mColorG;
    vx->b = mColorB;
    vx->a = mColorA;

    mVertexCount++;
}

void GLInterface::AddVertex(float x, float y)
{
    AddVertex(x, y, 0.0f, 0.0f);
}

void GLInterface::FlushIfOverBudget()
{
    if (mVertexCount > 0)
    {
        Flush();
    }
}

void GLInterface::End()
{
    if (mVertexCount > 0)
    {
        Flush();
    }
}

void GLInterface::Flush()
{
    if (mVertexCount <= 0)
    {
        return;
    }

    // Apply the current modelview transform (if dirty, loads it into GL).
    ApplyTransform();

    // Set up vertex, texcoord, and colour pointers into our interleaved
    // vertex buffer.  All three arrays live inside TVertex with a stride
    // of sizeof(TVertex).
    glVertexPointer(
        2,                      // 2 components (x,y)
        GL_FLOAT,               // type
        sizeof(TVertex),        // stride
        &mVertices[0].x);       // pointer to first x

    glTexCoordPointer(
        2,                      // 2 components (u,v)
        GL_FLOAT,
        sizeof(TVertex),
        &mVertices[0].u);       // pointer to first u

    glColorPointer(
        4,                      // 4 components (r,g,b,a)
        GL_FLOAT,
        sizeof(TVertex),
        &mVertices[0].r);       // pointer to first r

    // Issue the draw call
    glDrawArrays(mCurrentMode, 0, mVertexCount);

    // Reset counter for the next batch
    mVertexCount = 0;
}

// ---------------------------------------------------------------------------
// Texture management
// ---------------------------------------------------------------------------

GLuint GLInterface::CreateTexture(TInt width, TInt height,
                                  const void* pixels, TInt format)
{
    static TInt texCount = 0;
    texCount++;
    // [Session-12] Increased from 3 to 15 so we can see if the SelectorScreen_BG
    // texture (100x75 → POT 128x128) is created. The old cutoff of 3 only
    // showed the loading screen textures.
    if (texCount <= 15) {
        TBuf8<128> buf;
        buf.Format(_L8("GLI::CreateTexture #%d %dx%d fmt=%d"), texCount, width, height, format);
        GLTrace((const char*)buf.PtrZ());
    }
    // --- N95/MBX safety: GL_MAX_TEXTURE_SIZE is 1024 on PowerVR MBX Lite.
    // A bigger texture makes glTexImage2D fail (GL_INVALID_VALUE) and renders
    // as nothing -> caller sees only the clear colour. Query once, refuse
    // oversized requests, and log loudly so we learn the real limit.
    static GLint sMaxTex = 0;
    if (sMaxTex == 0)
    {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &sMaxTex);
        if (sMaxTex <= 0) sMaxTex = 1024;
        TBuf8<64> mb; mb.Format(_L8("GLI::GL_MAX_TEXTURE_SIZE=%d"), (int)sMaxTex);
        GLTrace((const char*)mb.PtrZ());
    }
    if (width > sMaxTex || height > sMaxTex)
    {
        TBuf8<96> ob;
        ob.Format(_L8("GLI::CreateTexture REJECT %dx%d > max %d"),
                  (int)width, (int)height, (int)sMaxTex);
        GLTrace((const char*)ob.PtrZ());
        return 0;
    }
    GLuint texID = 0;
    glGenTextures(1, &texID);

    if (texID == 0)
    {
        // [Session-12] Log when glGenTextures fails (GL out of texture memory).
        if (texCount <= 15)
        {
            GLTrace("GLI:glGenTextures returned 0 (GL out of texture memory)");
        }
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texID);

    // Default texture parameters (bilinear filtering, clamp-to-edge)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,                          // mipmap level
        (GLint)format,              // internal format
        (GLsizei)width,
        (GLsizei)height,
        0,                          // border (must be 0 in GLES)
        (GLenum)format,             // pixel data format
        GL_UNSIGNED_BYTE,           // type
        pixels);                    // data (may be NULL for empty texture)

    return texID;
}

void GLInterface::DeleteTexture(GLuint texID)
{
    if (texID != 0)
    {
        glDeleteTextures(1, &texID);
    }

    if (mCurrentTexture == texID)
    {
        mCurrentTexture = 0;
    }
}

void GLInterface::UpdateTexture(GLuint texID, TInt x, TInt y,
                                TInt w, TInt h, const void* pixels)
{
    if (texID == 0 || pixels == NULL)
    {
        return;
    }

    FlushIfOverBudget();
    glBindTexture(GL_TEXTURE_2D, texID);

    // Use glTexSubImage2DOES if the extension is available;
    // otherwise fall back to re-uploading the whole texture.
    // Since we do not track texture dimensions here, the sub-image
    // path is preferred when available.
    if (glTexSubImage2DOES != NULL)
    {
        glTexSubImage2DOES(
            GL_TEXTURE_2D,
            0,                      // level
            (GLint)x,
            (GLint)y,
            (GLsizei)w,
            (GLsizei)h,
            GL_RGBA,                // format
            GL_UNSIGNED_BYTE,
            pixels);
    }
    // If no sub-image extension, we must re-upload the entire texture.
    // This is a fallback that requires the caller to provide full-width
    // data; for partial updates we simply skip.
    // A production implementation would track texture dimensions to
    // allow correct full re-uploads here.
}

// ---------------------------------------------------------------------------
// Image <-> Texture conversion
// ---------------------------------------------------------------------------

TBool GLInterface::CopyImageToTexture(MemoryImage* img, GLuint texID)
{
    if (img == NULL || texID == 0)
    {
        return EFalse;
    }

    TUint32* bits = reinterpret_cast<TUint32*>(img->GetBits());
    TInt     w    = img->GetWidth();
    TInt     h    = img->GetHeight();

    if (bits == NULL || w <= 0 || h <= 0)
    {
        return EFalse;
    }

    // Bind and set up texture parameters
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Convert ARGB pixel data (native MemoryImage format) to GL_RGBA.
    TInt pixelCount = w * h;
    TUint32* rgbaData = new TUint32[pixelCount];
    if (rgbaData == NULL)
    {
        return EFalse;
    }

    ArgbToRgba(bits, rgbaData, pixelCount);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        (GLint)GL_RGBA,
        (GLsizei)w,
        (GLsizei)h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgbaData);

    delete[] rgbaData;
    return ETrue;
}

TBool GLInterface::CopyImageToTextureSub(MemoryImage* img, GLuint texID,
                                         TInt x, TInt y)
{
    if (img == NULL || texID == 0)
    {
        return EFalse;
    }

    TUint32* bits = reinterpret_cast<TUint32*>(img->GetBits());
    TInt     w    = img->GetWidth();
    TInt     h    = img->GetHeight();

    if (bits == NULL || w <= 0 || h <= 0)
    {
        return EFalse;
    }

    // EXPERIMENT v7: Force the caller down the full-POT glTexImage2D path.
    // glTexSubImage2DOES on the N95 MBX driver appears to load the image but
    // the texture never renders correctly (purple tint, 1/4 crop). Returning
    // EFalse here makes Graphics::GetOrCreateTexture do a full POT upload
    // with explicit zero-padding + manual byte layout, which is the only
    // path we can actually verify on-device.
    (void)bits;
    (void)w;
    (void)h;
    return EFalse;

    glBindTexture(GL_TEXTURE_2D, texID);

    if (glTexSubImage2DOES != NULL)
    {
        // Convert ARGB to RGBA for the sub-region
        TInt pixelCount = w * h;
        TUint32* rgbaData = new TUint32[pixelCount];
        if (rgbaData == NULL)
        {
            return EFalse;
        }

        ArgbToRgba(bits, rgbaData, pixelCount);

        glTexSubImage2DOES(
            GL_TEXTURE_2D,
            0,
            (GLint)x,
            (GLint)y,
            (GLsizei)w,
            (GLsizei)h,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            rgbaData);

        delete[] rgbaData;
        return ETrue;
    }

    // Extension not available -- can't sub-upload. Returning EFalse tells the
    // caller (Graphics::GetOrCreateTexture) to do a manual full-POT upload with
    // zero-padding. CopyImageToTexture would create an NPOT texture here, which
    // PowerVR MBX cannot render (renders as nothing -> purple screen).
    return EFalse;
}

// ---------------------------------------------------------------------------
// Pixel format conversion (ARGB <-> RGBA)
// ---------------------------------------------------------------------------

void GLInterface::ArgbToRgba(const TUint32* src, TUint32* dst, TInt count)
{
    TInt i;
    for (i = 0; i < count; i++)
    {
        TUint32 argb = src[i];
        TUint8 a = (TUint8)(argb >> 24);
        TUint8 r = (TUint8)(argb >> 16);
        TUint8 g = (TUint8)(argb >> 8);
        TUint8 b = (TUint8)(argb);
        // GL_RGBA layout in host-endian 32-bit word:
        // byte 0 = R, byte 1 = G, byte 2 = B, byte 3 = A
#if defined(__BIG_ENDIAN__)
        // Big-endian: store as R G B A in memory (which is what GL expects)
        dst[i] = ((TUint32)r << 24) | ((TUint32)g << 16)
               | ((TUint32)b << 8)  | (TUint32)a;
#else
        // Little-endian: GL_UNSIGNED_BYTE reads bytes in memory order.
        // We store RGBA as a 32-bit word where byte[0]=R, byte[1]=G,
        // byte[2]=B, byte[3]=A.  On little-endian this means:
        //   word = A<<24 | B<<16 | G<<8 | R
        // But the simplest cross-endian approach: keep host format and
        // let GL treat bytes sequentially.  For little-endian:
        //   word = R | (G<<8) | (B<<16) | (A<<24)
        dst[i] = (TUint32)r | ((TUint32)g << 8)
               | ((TUint32)b << 16) | ((TUint32)a << 24);
#endif
    }
}

void GLInterface::RgbaToArgb(const TUint32* src, TUint32* dst, TInt count)
{
    TInt i;
    for (i = 0; i < count; i++)
    {
        TUint32 rgba = src[i];
        TUint8 r, g, b, a;

#if defined(__BIG_ENDIAN__)
        // Big-endian: RGBA stored as R G B A in memory
        r = (TUint8)(rgba >> 24);
        g = (TUint8)(rgba >> 16);
        b = (TUint8)(rgba >> 8);
        a = (TUint8)(rgba);
#else
        // Little-endian: our custom storage is R at byte0, G at byte1,
        // B at byte2, A at byte3.  Extract as:
        r = (TUint8)(rgba);
        g = (TUint8)(rgba >> 8);
        b = (TUint8)(rgba >> 16);
        a = (TUint8)(rgba >> 24);
#endif
        // Store as ARGB (A R G B in memory for big-endian, or
        // little-endian: word = A<<24 | R<<16 | G<<8 | B)
#if defined(__BIG_ENDIAN__)
        dst[i] = ((TUint32)a << 24) | ((TUint32)r << 16)
               | ((TUint32)g << 8)  | (TUint32)b;
#else
        dst[i] = ((TUint32)a << 24) | ((TUint32)r << 16)
               | ((TUint32)g << 8)  | (TUint32)b;
#endif
    }
}

} // namespace Sexy