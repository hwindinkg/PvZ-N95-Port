#include "PvZGameView.h"
#include "../../engine/GLInterface.h"
#include "../../engine/SymbianFixes.h"
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../LawnApp.h"
#include "../Lawn/Board.h"

// Simple log function that ALWAYS flushes
static void Log(const TDesC& aMsg) {
    RFs fs; RFile f;
    if (fs.Connect() != KErrNone) return;
    fs.MkDirAll(_L("C:\\Data\\PvZ"));
    if (f.Open(fs, _L("C:\\Data\\PvZ\\log.txt"), EFileWrite|EFileShareAny) != KErrNone)
        f.Create(fs, _L("C:\\Data\\PvZ\\log.txt"), EFileWrite);
    else { TInt p=0; f.Seek(ESeekEnd,p); }
    TBuf8<128> _b; _b.Copy(aMsg); f.Write(_b); f.Write(_L8("\n")); f.Close(); fs.Close();
}

EGLDisplay gEglDisplay = EGL_NO_DISPLAY;
EGLSurface gEglSurface = EGL_NO_SURFACE;

CPvZGameView* CPvZGameView::NewL()
{
    CPvZGameView* self = new (ELeave) CPvZGameView();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CPvZGameView::CPvZGameView()
    : iGL(NULL), iEglDisplay(EGL_NO_DISPLAY),
      iEglSurface(EGL_NO_SURFACE), iEglContext(EGL_NO_CONTEXT)
{
}

void CPvZGameView::ConstructL()
{
    Log(_L("GV:ConstructL start"));
    CreateWindowL();
    Log(_L("GV:CreateWindowL done"));
    ::TSize screen(240, 320);
    SetRect(::TRect(::TPoint(0,0), screen));
    Window().SetRequiredDisplayMode(EColor64K);
    Log(_L("GV:SetDisplay done"));
    ActivateL();
    Log(_L("GV:ActivateL done"));
}

void CPvZGameView::InitGLES()
{
    Log(_L("GL:InitGLES start"));
    iEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (iEglDisplay == EGL_NO_DISPLAY) { Log(_L("GL:eglGetDisplay FAIL")); return; }

    EGLint major, minor;
    if (!eglInitialize(iEglDisplay, &major, &minor)) { Log(_L("GL:eglInit FAIL")); return; }
    Log(_L("GL:eglInit OK"));

    EGLConfig config;
    // Get configs with WINDOW_BIT
    EGLint numConfigs;
    EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    if (!eglChooseConfig(iEglDisplay, attribs, NULL, 0, &numConfigs) || numConfigs == 0)
    { Log(_L("GL:no cfgs")); eglTerminate(iEglDisplay); return; }

    EGLConfig* configs = new EGLConfig[numConfigs];
    eglChooseConfig(iEglDisplay, attribs, configs, numConfigs, &numConfigs);

    // Log all configs for debugging
    config = NULL;
    for (EGLint i = 0; i < numConfigs && i < 20; i++) {
        EGLint rb, gb, bb, depth, stencil, rtype;
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_RED_SIZE, &rb);
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_GREEN_SIZE, &gb);
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_BLUE_SIZE, &bb);
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_STENCIL_SIZE, &stencil);
        eglGetConfigAttrib(iEglDisplay, configs[i], EGL_RENDERABLE_TYPE, &rtype);
        TBuf<128> b;
        b.Format(_L("CFG%d: %d%d%d d=%d st=%d rtype=0x%x"), i, rb, gb, bb, depth, stencil, rtype);
        Log(b);
    }
    // Take first config (don't filter by rtype - N95 EGL may not set it)
    config = configs[0];
    delete[] configs;

    if (!config)
    { Log(_L("GL:no ES cfg")); eglTerminate(iEglDisplay); return; }
    Log(_L("GL:chooseCfg OK"));

    {
        RDrawableWindow* drawWnd = DrawableWindow();
        if (!drawWnd) { Log(_L("GL:DrawableWindow NULL!")); return; }
        Log(_L("GL:DrawableWindow OK"));
        RWindow* wnd = reinterpret_cast<RWindow*>(drawWnd);
        iEglSurface = eglCreateWindowSurface(iEglDisplay, config, wnd, NULL);
    }
    if (iEglSurface == EGL_NO_SURFACE) { Log(_L("GL:createSurface FAIL")); return; }
    Log(_L("GL:createSurface OK"));

    // Try context with ES 1.1, fallback to default
    EGLint ctx[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};
    iEglContext = eglCreateContext(iEglDisplay, config, EGL_NO_CONTEXT, ctx);
    if (iEglContext == EGL_NO_CONTEXT)
    {
        // Fallback: try without version attribute (N95 PowerVR MBX compatibility)
        EGLint ctxFallback[] = {EGL_NONE};
        iEglContext = eglCreateContext(iEglDisplay, config, EGL_NO_CONTEXT, ctxFallback);
        if (iEglContext == EGL_NO_CONTEXT)
        {
            Log(_L("GL:createCtx FAIL"));
            eglDestroySurface(iEglDisplay, iEglSurface);
            iEglSurface = EGL_NO_SURFACE;
            eglTerminate(iEglDisplay);
            iEglDisplay = EGL_NO_DISPLAY;
            return;
        }
    }
    Log(_L("GL:createCtx OK"));

    if (!eglMakeCurrent(iEglDisplay, iEglSurface, iEglSurface, iEglContext))
    { Log(_L("GL:makeCurrent FAIL")); return; }
    Log(_L("GL:makeCurrent OK"));

    gEglDisplay = iEglDisplay;
    gEglSurface = iEglSurface;

    iGL = new Sexy::GLInterface();
    if (iGL) {
        iGL->Init();
        Log(_L("GL:iGL->Init done"));
        iGL->SetViewport(0, 0, 240, 320);
        Log(_L("GL:SetViewport done"));
    }
    Log(_L("GL:InitGLES done"));
}

void CPvZGameView::ShutdownGLES()
{
    if (iEglContext != EGL_NO_CONTEXT) {
        eglMakeCurrent(iEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(iEglDisplay, iEglContext);
        iEglContext = EGL_NO_CONTEXT;
    }
    if (iEglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(iEglDisplay, iEglSurface);
        iEglSurface = EGL_NO_SURFACE;
    }
    if (iEglDisplay != EGL_NO_DISPLAY) {
        eglTerminate(iEglDisplay);
        iEglDisplay = EGL_NO_DISPLAY;
    }
}

void CPvZGameView::Draw() const {}

void CPvZGameView::RenderFrame(LawnApp* theApp)
{
    if (!theApp || !iGL || iEglContext == EGL_NO_CONTEXT)
    { Log(_L("RF:skip")); return; }

    static int fc = 0;
    fc++;

    if (fc <= 3 || fc % 100 == 0) {
        TBuf<64> b; b.Format(_L("RF%d:start"), fc); Log(b);
    }

    if (!eglMakeCurrent(iEglDisplay, iEglSurface, iEglSurface, iEglContext))
    { Log(_L("RF:makeCurrent FAIL")); return; }

    glClearColor(0.15f, 0.05f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (fc <= 3) Log(_L("RF:Calling UpdateFrames"));
    theApp->UpdateFrames();
    if (fc <= 3) Log(_L("RF:UpdateFrames done"));

    if (fc <= 3) Log(_L("RF:swap"));
    eglSwapBuffers(iEglDisplay, iEglSurface);
    if (fc <= 3 || fc % 100 == 0) {
        TBuf<64> b; b.Format(_L("RF%d:done"), fc); Log(b);
    }
}

CPvZGameView::~CPvZGameView()
{
    ShutdownGLES();
}
