// platform/symbian/PvZGameView.h
// GLES 1.1 rendering view for PvZ on Symbian S60 3rd FP1

#ifndef PVZGAMEVIEW_H
#define PVZGAMEVIEW_H

#include <coecntrl.h>
#include <e32std.h>
#include <w32std.h>
#include <gles/gl.h>
#include <gles/egl.h>

namespace Sexy { class LawnApp; }
namespace Sexy { class GLInterface; }

class CPvZGameView : public CCoeControl
{
public:
    static CPvZGameView* NewL();
    ~CPvZGameView();

    void Draw() const;
    void InitGLES();
    void ShutdownGLES();

    // Called from game loop with the current application
    void RenderFrame(Sexy::LawnApp* theApp);

    // Expose GL interface for game drawing
    Sexy::GLInterface* GetGL() const { return iGL; }

private:
    CPvZGameView();
    void ConstructL();

private:
    Sexy::GLInterface*      iGL;

    EGLDisplay  iEglDisplay;
    EGLSurface  iEglSurface;
    EGLContext  iEglContext;
};

#endif // PVZGAMEVIEW_H
