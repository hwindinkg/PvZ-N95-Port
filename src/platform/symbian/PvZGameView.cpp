#include "PvZGameView.h"
#include <coemain.h>
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
    // Cover the whole screen, like the GTA3 (re3-symbian) and Whisk3D
    // references. A hardcoded SetRect can leave the window not marked as
    // fully covering the display, so WSERV shows stale video memory underneath.
    SetExtentToWholeScreen();
    Window().SetRequiredDisplayMode(EColor64K);
    // [green-mess fix] Until WSERV composites our EGL surface, it paints the
    // window's own background. With no background set, that is uninitialised
    // video memory -> the "green mess" seen before the first full recomposite
    // (which is why opening the Options menu, forcing a full-screen redraw,
    // suddenly shows the correct frame). Set the window background to the same
    // colour as glClearColor(0.15,0.05,0.20) so WSERV's own fill matches our
    // GL clear -> no garbage even on the very first frame, with or without a
    // composite. The reference (re3-symbian) avoids this only incidentally; an
    // explicit background is the robust fix.
    Window().SetBackgroundColor(TRgb(38, 13, 51));
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
    // Match the EGL config to the WINDOW's native pixel format. The N95 frame
    // buffer is RGB565 (EColor64K). If we pick an RGB888 config (configs[0] on
    // this driver), the surface format mismatches the window -> the MBX driver
    // blits garbage and WSERV only shows a correct frame after a forced
    // recomposite (the "green grid until the Options menu opens" symptom).
    // This mirrors the working Whisk3D reference, which derives EGL_BUFFER_SIZE
    // from Window().DisplayMode().
    EGLint bufferSize = 16; // EColor64K / RGB565 default
    switch (Window().DisplayMode())
    {
        case EColor4K:   bufferSize = 12; break;
        case EColor64K:  bufferSize = 16; break;
        case EColor16M:  bufferSize = 24; break;
        case EColor16MU:
        case EColor16MA: bufferSize = 32; break;
        default:         bufferSize = 16; break;
    }
    {
        TBuf<64> dbg; dbg.Format(_L("GL:win DMode bufSize=%d"), bufferSize); Log(dbg);
    }

    // [green-grid fix] EGL_BUFFER_SIZE is a MINIMUM, so asking for 16 returns
    // an 888/24-bit config (boot log: "chosen cfg 888 d=24"). A 24-bit surface
    // on the RGB565 (EColor64K) window makes the MBX driver blit a
    // mismatched-format buffer -> the green "dense grid" garbage that only
    // clears on a forced full recomposite (opening the menu). Fix: request the
    // EXACT per-channel sizes for the window format and pick the exact match.
    EGLint wantR, wantG, wantB;
    if (bufferSize >= 24) { wantR = 8; wantG = 8; wantB = 8; }
    else                  { wantR = 5; wantG = 6; wantB = 5; }

    EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE,     wantR,
        EGL_GREEN_SIZE,   wantG,
        EGL_BLUE_SIZE,    wantB,
        EGL_DEPTH_SIZE,   16,
        EGL_NONE
    };
    EGLint attribs2[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE,     wantR,
        EGL_GREEN_SIZE,   wantG,
        EGL_BLUE_SIZE,    wantB,
        EGL_NONE
    };
    EGLint* chosenAttribs = attribs;
    if (!eglChooseConfig(iEglDisplay, attribs, NULL, 0, &numConfigs) || numConfigs == 0)
    {
        // Fall back to depth-less match if no depth config exists for this mode.
        Log(_L("GL:no cfg w/depth, retry no-depth"));
        if (!eglChooseConfig(iEglDisplay, attribs2, NULL, 0, &numConfigs) || numConfigs == 0)
        { Log(_L("GL:no cfgs")); eglTerminate(iEglDisplay); return; }
        chosenAttribs = attribs2;
    }

    // eglChooseConfig sorts by DESCENDING component sizes, so [0] can still be a
    // larger config than requested. Enumerate and take the EXACT R/G/B match for
    // the window; fall back to the first only if nothing matches.
    {
        EGLConfig cfgList[32];
        EGLint cfgGot = 0;
        config = NULL;
        eglChooseConfig(iEglDisplay, chosenAttribs, cfgList, 32, &cfgGot);
        for (EGLint ci2 = 0; ci2 < cfgGot; ci2++)
        {
            EGLint rr = 0, gg = 0, bb2 = 0;
            eglGetConfigAttrib(iEglDisplay, cfgList[ci2], EGL_RED_SIZE,   &rr);
            eglGetConfigAttrib(iEglDisplay, cfgList[ci2], EGL_GREEN_SIZE, &gg);
            eglGetConfigAttrib(iEglDisplay, cfgList[ci2], EGL_BLUE_SIZE,  &bb2);
            if (rr == wantR && gg == wantG && bb2 == wantB) { config = cfgList[ci2]; break; }
        }
        if (config == NULL && cfgGot > 0) config = cfgList[0];
    }

    {
        EGLint rb, gb, bb, depth;
        eglGetConfigAttrib(iEglDisplay, config, EGL_RED_SIZE, &rb);
        eglGetConfigAttrib(iEglDisplay, config, EGL_GREEN_SIZE, &gb);
        eglGetConfigAttrib(iEglDisplay, config, EGL_BLUE_SIZE, &bb);
        eglGetConfigAttrib(iEglDisplay, config, EGL_DEPTH_SIZE, &depth);
        TBuf<128> b;
        b.Format(_L("GL:chosen cfg %d%d%d d=%d"), rb, gb, bb, depth);
        Log(b);
    }

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
    // CRITICAL: push the queued composite to the window server NOW. Our render
    // is a hand-rolled loop (unlike RenderWare's RwCameraShowRaster in the GTA3
    // re3-symbian reference, which flushes internally). Without this, the swap
    // command sits in the client-side WSERV buffer and the screen only refreshes
    // when some OTHER event (a key press / opening the menu) flushes the session
    // -- exactly the "garbage until I press a key" symptom. Flushing every frame
    // makes each rendered frame appear immediately.
    CCoeEnv::Static()->WsSession().Flush();
    if (fc <= 3 || fc % 100 == 0) {
        TBuf<64> b; b.Format(_L("RF%d:done"), fc); Log(b);
    }
}

CPvZGameView::~CPvZGameView()
{
    ShutdownGLES();
}