#include "PvZAppUi.h"
#include "PvZGameView.h"
#include "LawnApp.h"
#include "PakInterface.h"
#include "PvZVfs.h"
#include "engine/ResourceManager.h"
#include "engine/WidgetManager.h"
#include "Resources.h"
#include <f32file.h>

using namespace Sexy;

static void Log(const TDesC& aMsg)
{
    RFs fs; RFile f;
    if (fs.Connect() != KErrNone) return;
    fs.MkDirAll(_L("C:\\Data\\PvZ"));
    TInt err = f.Open(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite|EFileShareAny);
    if (err == KErrNotFound) err = f.Create(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite|EFileShareAny);
    if (err == KErrNone) {
        TInt pos = 0; f.Seek(ESeekEnd, pos);
        TBuf8<512> buf8; buf8.Copy(aMsg); buf8.Append('\n');
        f.Write(buf8); f.Flush(); f.Close();
    }
    fs.Close();
}

CPvZAppUi::CPvZAppUi() : iGameView(NULL), iLawnApp(NULL), iTimer(NULL),
    iCursorX(200), iCursorY(86), iCursorVisible(true), iCentreKeyDown(false),
    iLoadingState(0), iLoadingFrames(0) {}
CPvZAppUi::~CPvZAppUi()
{
    if (iTimer) { iTimer->Cancel(); delete iTimer; iTimer = NULL; }
    if (iGameView) { delete iGameView; iGameView = NULL; }
    if (iLawnApp) { delete iLawnApp; iLawnApp = NULL; }
    gLawnApp = NULL;
    if (gPak) { delete gPak; gPak = NULL; }
}

void CPvZAppUi::ConstructL()
{
    Log(_L("AppUi::ConstructL ENTER"));
    Log(_L("BUILD MARKER v13-m4-gameselector: clickable menu, dpad->mouse, ToolTipWidget"));
    InitVirtualCursor();

    // === DECISIVE EH/Leave self-test ===
    // On EKA2, User::Leave == 'throw XLeaveException' and TRAP == try/catch.
    // If C++ exception unwinding is broken in this RVCT binary, this Leave will
    // hard-fault (KERN-EXEC 3) instead of being caught -> proving that the
    // BaseConstructL crash is really the FIRST internal Leave, not app logic.
    Log(_L("EHTEST: about to User::Leave(KErrNotFound) inside TRAP"));
    { TRAPD(ehErr, User::Leave(KErrNotFound));
      TBuf<64> b; b.Format(_L("EHTEST: SURVIVED, caught err=%d"), ehErr); Log(b); }

    // NOTE: The EH self-test (raw C++ throw/catch) was removed.
    // This port follows the Whisk3D reference design and does NOT rely on C++
    // exceptions: the engine never throws (operator new returns NULL on OOM,
    // resource errors use return codes / User::Leave). The only real C++ throw
    // in the whole codebase was this diagnostic, and raw throw/catch is
    // unreliable under RVCT 2.2 on EKA2 -- it crashed here with KERN-EXEC 3.
    // Symbian TRAP/Leave is used for error handling instead.

    // --- DIAGNOSTIC bisection: log env pointers, then call BaseConstructL the
    //     EXACT way the working Whisk3D reference does (default flags). If this
    //     still faults, the cause is the process/binary env, not the flags/resource.
    { TBuf<80> p; p.Format(_L("probe: iEikonEnv=%x iCoeEnv=%x"), (TUint)iEikonEnv, (TUint)iCoeEnv); Log(p); }
    Log(_L("step: BaseConstructL() [default flags, Whisk3D-style]"));
    TRAPD(bcErr, BaseConstructL());
    { TBuf<64> b; b.Format(_L("step: BaseConstructL returned err=%d"), bcErr); Log(b); }
    User::LeaveIfError(bcErr);
    Log(_L("step: BaseConstructL OK"));

    // [ORIENTATION] Lock to LANDSCAPE *before* the game view creates its window
    // and EGL surface. PvZ's canvas is 4:3 (GL ortho 400x300); the N95 default is
    // portrait 240x320, which squishes the frame vertically. 320x240 landscape is
    // exactly 4:3 -> full screen, no letterbox. We call SetOrientationL here (in
    // CAknAppUi, which already includes the AVKON chain) rather than in
    // PvZGameView.cpp, whose minimal engine includes clash with <aknappui.h>.
    // Same approach as the re3-symbian GTA3 reference (also a landscape game).
    {
        TRAPD(oErr, SetOrientationL(CAknAppUiBase::EAppUiOrientationLandscape));
        if (oErr != KErrNone) { TBuf<48> b; b.Format(_L("step: SetOrientation err=%d"), oErr); Log(b); }
        else Log(_L("step: SetOrientation Landscape OK"));
    }

    Log(_L("step: CPvZVfs::NewL"));
    gPak = CPvZVfs::NewL();
    Log(_L("step: LoadPakL main.pak"));
    gPak->LoadPakL(_L("C:\\Data\\PvZ\\main.pak"));
    Log(_L("step: main.pak loaded OK"));

    Log(_L("step: new LawnApp"));
    iLawnApp = new (ELeave) LawnApp();
    gLawnApp = iLawnApp;
    iLawnApp->mWidth = 400; iLawnApp->mHeight = 300;
    iLawnApp->mShutdown = false;
    Log(_L("step: LawnApp basic members set"));
    if (!iLawnApp->mWidgetManager) iLawnApp->mWidgetManager = new Sexy::WidgetManager();
    if (!iLawnApp->mGraphics) iLawnApp->mGraphics = new Sexy::Graphics();
    if (!iLawnApp->mResourceManager) iLawnApp->mResourceManager = new ResourceManager();
    gResourceManager = iLawnApp->mResourceManager;
    Log(_L("step: subsystems allocated"));

    Log(_L("step: LawnApp::Init"));
    iLawnApp->Init();
    Log(_L("step: LawnApp::Init done"));

    Log(_L("step: GameView NewL"));
    iGameView = CPvZGameView::NewL();
    iGameView->SetMopParent(this);   // MOP chain (skin/draw context) -- reference pattern
    Log(_L("step: InitGLES"));
    iGameView->InitGLES();
    Log(_L("step: InitGLES done"));

    gSexyAppBase = iLawnApp;
    iLawnApp->mGL = iGameView->GetGL();
    if (iLawnApp->mGraphics)
        iLawnApp->mGraphics->SetGLInterface(iGameView->GetGL());
    Log(_L("step: GL wired"));

    // ===== M1: prove the asset pipeline end-to-end with ONE real image =====
    // TodLoadResources() is still a stub (returns true, loads nothing), so the
    // bulk LoadingThreadProc() below populates NOTHING and every IMAGE_* stays
    // NULL -> the title screen has nothing to draw. Here we BYPASS the stub and
    // pull a single asset through the REAL path to prove each stage works:
    //   gResourceManager->GetImage("IMAGE_TITLESCREEN")
    //     -> LoadImageByResName -> "images/titlescreen.png" in main.pak
    //     -> ICL CImageDecoder -> MemoryImage(ARGB).
    // TitleScreen::Draw then does g->DrawImage(IMAGE_TITLESCREEN,0,0), and
    // Graphics uploads a GL texture lazily from the bits (sTexCache). If the PvZ
    // title screen renders on device, the pipeline is proven and M2 scales this
    // up to all resource groups (restore Resources.cpp / un-stub TodLoadResources).
    if (gResourceManager)
    {
        Log(_L("M1: GetImage IMAGE_TITLESCREEN ..."));
        IMAGE_TITLESCREEN = gResourceManager->GetImage("IMAGE_TITLESCREEN");
        if (IMAGE_TITLESCREEN)
        {
            TBuf<64> b; b.Format(_L("M1: TITLESCREEN OK %dx%d"),
                IMAGE_TITLESCREEN->GetWidth(), IMAGE_TITLESCREEN->GetHeight());
            Log(b);
        }
        else
        {
            Log(_L("M1: TITLESCREEN NULL (pak miss / decode fail)"));
        }
    }

    // --- CRITICAL: drive resource loading (no loading thread on this port) ---
    // The original game spawns a background thread running LoadingThreadProc()
    // while the TitleScreen shows a progress bar; when it finishes,
    // LoadingCompleted() removes the title screen and shows the GameSelector
    // (main menu). This Symbian port stubbed out threading and NEVER wired a
    // replacement, so LoadingThreadProc() was never called -> resources never
    // loaded -> the game sat forever on the title screen (the static purple
    // frame with the white box; wgt_log showed Widgets=1). We run the load
    // synchronously HERE -- after the GL interface is wired (textures need GL),
    // and before the heartbeat timer starts.
    if (iLawnApp)
    {
        Log(_L("step: LoadingThreadProc (sync) START"));
        iLawnApp->LoadingThreadProc();
        Log(_L("step: LoadingThreadProc DONE"));
        // [M4 #1 fix] DEFER LoadingCompleted so the loading screen is visible.
        // LoadingThreadProc is synchronous -- by the time it returns, all
        // resources are loaded. If we call LoadingCompleted here, TitleScreen
        // is removed before the heartbeat timer renders a single frame, so
        // the user never sees the progress bar animation.
        // Instead: set iLoadingState=1 and iLoadingFrames=60 (~2s at 30fps).
        // RenderTick will call LoadingCompleted after the countdown finishes.
        iLoadingState = 1;
        iLoadingFrames = 60;
        Log(_L("step: loading animation deferred to RenderTick (60 frames)"));
    }

    // Put the GL container on the control stack so it receives
    // key/pointer events and participates in redraw -- this was the
    // missing piece vs. the working Whisk3D reference.
    AddToStackL(iGameView);
    Log(_L("step: AddToStackL done"));

    Log(_L("step: start heartbeat timer"));
    iTimer = CPeriodic::NewL(CActive::EPriorityLow);  // match GTA3 re3-symbian render-loop priority (EPriorityIdle starved the loop)
    iTimer->Start(KFrameInterval, KFrameInterval, TCallBack(Tick, this));
    Log(_L("AppUi::ConstructL EXIT"));
}

TInt CPvZAppUi::Tick(TAny* aPtr)
{
    static_cast<CPvZAppUi*>(aPtr)->RenderTick();
    return ETrue; // keep the periodic timer running
}

void CPvZAppUi::RenderTick()
{
    if (!iLawnApp || !iGameView)
        return;
    if (iLawnApp->mShutdown)
    {
        if (iTimer) iTimer->Cancel();
        Exit();
        return;
    }

    // [M4 #1] Loading screen state machine. TitleScreen is in the widget
    // manager and animates its progress bar via Update()/Draw(). After
    // iLoadingFrames countdown, call LoadingCompleted to transition to menu.
    if (iLoadingState == 1)
    {
        iLoadingFrames--;
        if (iLoadingFrames <= 0)
        {
            Log(_L("step: loading animation done -> LoadingCompleted"));
            iLawnApp->LoadingCompleted();
            Log(_L("step: LoadingCompleted returned OK"));
            iLoadingState = 2;
        }
    }

    iGameView->RenderFrame(iLawnApp);
}

void CPvZAppUi::HandleCommandL(TInt aCommand)
{
    switch (aCommand) {
        case EAknSoftkeyExit:
        case EEikCmdExit:
            Exit();
            break;
        default: break;
    }
}

void CPvZAppUi::HandleForegroundEventL(TBool aForeground)
{
    CAknAppUi::HandleForegroundEventL(aForeground);
    if (iLawnApp) iLawnApp->mActive = aForeground;
}

TKeyResponse CPvZAppUi::HandleKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
    if (!iLawnApp || !iLawnApp->mWidgetManager) return EKeyWasNotConsumed;

    // M4 #2 -- d-pad arrow keys MOVE a virtual cursor on the 400x300 logical
    // canvas. The N95 has no touch on the original hardware, but the d-pad is
    // the natural navigation input. We synthesise MouseMove events so the
    // WidgetManager's hover/click path is exercised.
    //
    // Centre key (EStdKeyDevice3 / Enter) synthesises MouseDown+MouseUp at
    // the current cursor position -- this is the "click".
    //
    // Escape goes to the focused widget via KeyDown (for back / quit).
    if (aType == EEventKeyDown)
    {
        Sexy::KeyCode aKey = Sexy::KEYCODE_UNKNOWN;
        int dx = 0, dy = 0;
        bool isCentre = false;
        switch (aKeyEvent.iScanCode)
        {
            case EStdKeyUpArrow:
            case EStdKeyDevice8:  aKey = Sexy::KEYCODE_UP;    dy = -32; break;
            case EStdKeyDownArrow:
            case EStdKeyDevice9:  aKey = Sexy::KEYCODE_DOWN;  dy =  32; break;
            case EStdKeyLeftArrow:
            case EStdKeyDevice10: aKey = Sexy::KEYCODE_LEFT;  dx = -32; break;
            case EStdKeyRightArrow:
            case EStdKeyDevice11: aKey = Sexy::KEYCODE_RIGHT; dx =  32; break;
            case EStdKeyDevice3:
            case EStdKeyEnter:    isCentre = true; aKey = Sexy::KEYCODE_RETURN; break;
            case EStdKeyDevice1:  aKey = Sexy::KEYCODE_ESCAPE; break;
            default: break;
        }

        if (dx != 0 || dy != 0)
        {
            // Move the virtual cursor, clamp to the 400x300 canvas.
            iCursorX += dx; if (iCursorX < 0) iCursorX = 0; if (iCursorX > 399) iCursorX = 399;
            iCursorY += dy; if (iCursorY < 0) iCursorY = 0; if (iCursorY > 299) iCursorY = 299;
            iCursorVisible = true;
            SynthesizeMouseMove();
            return EKeyWasConsumed;
        }
        if (isCentre && !iCentreKeyDown)
        {
            iCentreKeyDown = true;
            iCursorVisible = true;  // show cursor on first click too
            SynthesizeMouseClick();
            return EKeyWasConsumed;
        }
        if (aKey != Sexy::KEYCODE_UNKNOWN)
        {
            iLawnApp->mWidgetManager->KeyDown(aKey);
            return EKeyWasConsumed;
        }
    }
    else if (aType == EEventKeyUp)
    {
        // Reset centre-key debounce on release so the next press clicks again.
        if (aKeyEvent.iScanCode == EStdKeyDevice3 || aKeyEvent.iScanCode == EStdKeyEnter)
            iCentreKeyDown = false;
    }
    return EKeyWasNotConsumed;
}

// M4 #2 -- Forward raw touch/pointer events to the WidgetManager. The N95
// original has no touch, but the S60 3rd FP1 platform was used on touch
// devices (5800, N95 8GB later firmware) and the framework still routes
// TPointerEvent through HandlePointerEventL. We map physical window
// coords (e.g. 320x240 landscape) to the logical 400x300 canvas.
void CPvZAppUi::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
    if (!iLawnApp || !iLawnApp->mWidgetManager || !iGameView) return;

    // Map window coords -> logical 400x300 canvas.
    TSize wsz = iGameView->Size();
    if (wsz.iWidth <= 0 || wsz.iHeight <= 0) return;
    int lx = (aPointerEvent.iPosition.iX * 400) / wsz.iWidth;
    int ly = (aPointerEvent.iPosition.iY * 300) / wsz.iHeight;

    switch (aPointerEvent.iType)
    {
        case TPointerEvent::EButton1Down:
            iLawnApp->mWidgetManager->MouseDown(lx, ly, 0);
            iCursorX = lx; iCursorY = ly; iCursorVisible = true;
            break;
        case TPointerEvent::EButton1Up:
            iLawnApp->mWidgetManager->MouseUp(lx, ly, 0);
            break;
        case TPointerEvent::EDrag:
            iLawnApp->mWidgetManager->MouseDrag(lx, ly);
            iCursorX = lx; iCursorY = ly;
            break;
        case TPointerEvent::EMove:
            iLawnApp->mWidgetManager->MouseMove(lx, ly);
            iCursorX = lx; iCursorY = ly;
            break;
        default:
            break;
    }
}

void CPvZAppUi::InitVirtualCursor()
{
    // Start the cursor on the Adventure button (60..340, 70..102).
    // Centre of Adventure is (200, 86). This way the first OK press clicks
    // Adventure without needing to move the d-pad first.
    iCursorX = 200;
    iCursorY = 86;
    iCursorVisible = true;   // visible from the start
    iCentreKeyDown = false;
}

void CPvZAppUi::SynthesizeMouseMove()
{
    if (!iLawnApp || !iLawnApp->mWidgetManager) return;
    // Update the static cursor globals so WidgetManager::DrawScreen can
    // render the cursor overlay.
    Sexy::g_sCursorX = iCursorX;
    Sexy::g_sCursorY = iCursorY;
    Sexy::g_sCursorVisible = iCursorVisible;
    iLawnApp->mWidgetManager->MouseMove(iCursorX, iCursorY);
}

void CPvZAppUi::SynthesizeMouseClick()
{
    if (!iLawnApp || !iLawnApp->mWidgetManager) return;
    // Move to the cursor position first so the correct widget is hit.
    Sexy::g_sCursorX = iCursorX;
    Sexy::g_sCursorY = iCursorY;
    Sexy::g_sCursorVisible = iCursorVisible;
    iLawnApp->mWidgetManager->MouseMove(iCursorX, iCursorY);
    // MouseDown then MouseUp -- this triggers WidgetManager::FindWidget ->
    // w->MouseDown -> SetFocus; then w->MouseUp which GameButton uses to
    // fire ButtonDepress(id).
    iLawnApp->mWidgetManager->MouseDown(iCursorX, iCursorY, 0);
    iLawnApp->mWidgetManager->MouseUp(iCursorX, iCursorY, 0);
}