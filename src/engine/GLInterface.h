// GLInterface.h
// Main GLES 1.1 fixed-pipeline renderer interface for Symbian S60 3rd FP1.
// Adapts the PvZ-Portable GLInterface for GLES 1.1 with no VBOs and
// immediate-mode-style vertex submission through interleaved arrays.
//
// C++03 compatible, no exceptions, no STL.
// Uses Symbian integer types (TInt, TBool, TUint32).

#ifndef GLINTERFACE_H
#define GLINTERFACE_H

#include <e32base.h>
#include <gles/gl.h>
#include <gles/egl.h>
#include "SexyMatrix.h"

namespace Sexy {

class MemoryImage;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// Maximum vertices for a single batched draw call.
static const TInt MAX_VERTICES = 4096;

// Interleaved vertex structure for 2D rendering.
// x,y = position; u,v = texture coords; r,g,b,a = colour.
struct TVertex
{
    float x, y;
    float u, v;
    float r, g, b, a;
};

enum DrawMode
{
    DRAWMODE_NORMAL   = 0,  // alpha blend: src * srcA + dst * (1 - srcA)
    DRAWMODE_ADDITIVE = 1   // additive:     src * srcA + dst * 1
};

// ---------------------------------------------------------------------------
// GLInterface -- GLES 1.1 fixed-pipeline 2D renderer
// ---------------------------------------------------------------------------
class GLInterface
{
public:
    GLInterface();
    ~GLInterface();

    // Initialisation ---------------------------------------------------------
    TBool Init();
    void  Redraw();

    // Texture management ----------------------------------------------------
    GLuint CreateTexture(TInt width, TInt height, const void* pixels, TInt format);
    void   DeleteTexture(GLuint texID);
    void   UpdateTexture(GLuint texID, TInt x, TInt y, TInt w, TInt h,
                         const void* pixels);

    // Immediate-mode-style drawing ------------------------------------------
    void Clear();
    void SetDrawMode(DrawMode mode);
    void SetColor(float r, float g, float b, float a);
    void SetTexture(GLuint texID);
    void SetTextureEnabled(TBool enabled);

    // Matrix operations -----------------------------------------------------
    void PushTransform(const SexyMatrix3& m);
    void PopTransform();
    void LoadIdentity();
    void Translate(float x, float y);
    void SetProjection(float left, float top, float right, float bottom);

    // Vertex submission -----------------------------------------------------
    void Begin(GLenum mode);
    void AddVertex(float x, float y, float u, float v);
    void AddVertex(float x, float y);
    void FlushIfOverBudget();
    void End();

    // Viewport --------------------------------------------------------------
    void SetViewport(TInt x, TInt y, TInt w, TInt h);
    void UpdateViewport(TInt drawW, TInt drawH);

    // Image<->Texture conversion --------------------------------------------
    TBool CopyImageToTexture(MemoryImage* img, GLuint texID);
    TBool CopyImageToTextureSub(MemoryImage* img, GLuint texID,
                                TInt x, TInt y);

    // Pixel format conversion -----------------------------------------------
    static void ArgbToRgba(const TUint32* src, TUint32* dst, TInt count);
    static void RgbaToArgb(const TUint32* src, TUint32* dst, TInt count);

    // Accessors -------------------------------------------------------------
    MemoryImage* GetScreenImage() { return mScreenImage; }

private:
    // Internal helpers
    void SetupGLState();
    void ApplyTransform();
    void Flush();

    MemoryImage* mScreenImage;

    // Vertex batch
    TVertex mVertices[MAX_VERTICES];
    TInt    mVertexCount;
    GLenum  mCurrentMode;

    // Texture state
    GLuint   mCurrentTexture;
    TBool    mTextureEnabled;
    DrawMode mDrawMode;

    // Transform stack
    static const TInt MAX_TRANSFORMS = 32;
    SexyMatrix3 mTransformStack[MAX_TRANSFORMS];
    TInt        mTransformDepth;
    TBool       mTransformDirty;
    float       mGLMatrix[16]; // cached 4x4 for glLoadMatrixf

    // Viewport
    TInt mViewX, mViewY, mViewW, mViewH;

    // Colour
    float mColorR, mColorG, mColorB, mColorA;

    // Flags
    TBool mInitialized;
};

} // namespace Sexy

#endif // GLINTERFACE_H
