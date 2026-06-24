// engine/GLESUtils.h
// Compatibility header for old GLES_Utils.h functions.
// Now delegates to GLInterface.
#ifndef GLES_UTILS_H
#define GLES_UTILS_H

#include <e32base.h>
#include "GLInterface.h"

// Draw a filled rectangle using the current GLInterface color
inline void DrawRectGLES(Sexy::GLInterface* g, TInt x, TInt y, TInt w, TInt h)
{
    if (!g) return;
    g->Begin(GL_TRIANGLES);
    float fx = (float)x;
    float fy = (float)y;
    float fw = (float)(x + w);
    float fh = (float)(y + h);
    g->AddVertex(fx, fy);
    g->AddVertex(fw, fy);
    g->AddVertex(fw, fh);
    g->AddVertex(fx, fy);
    g->AddVertex(fw, fh);
    g->AddVertex(fx, fh);
    g->End();
}

#endif // GLES_UTILS_H
