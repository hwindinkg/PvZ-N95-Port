#include "PvZAppUi.h"
#include "PvZGameView.h"
#include "LawnApp.h"
#include "PakInterface.h"
#include "PvZVfs.h"
#include "engine/ResourceManager.h"
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

CPvZAppUi::CPvZAppUi() : iGameView(NULL), iLawnApp(NULL) {}
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
        iLawnApp->LoadingCompleted();   // remove title screen -> ShowGameSelector()
        Log(_L("step: LoadingCompleted -> menu"));
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
    if (aType == EEventKeyDown) {
        Sexy::KeyCode aKey = Sexy::KEYCODE_UNKNOWN;
        switch (aKeyEvent.iScanCode) {
            case EStdKeyUpArrow:
            case EStdKeyDevice8:  aKey = Sexy::KEYCODE_UP; break;
            case EStdKeyDownArrow:
            case EStdKeyDevice9:  aKey = Sexy::KEYCODE_DOWN; break;
            case EStdKeyLeftArrow:
            case EStdKeyDevice10: aKey = Sexy::KEYCODE_LEFT; break;
            case EStdKeyRightArrow:
            case EStdKeyDevice11: aKey = Sexy::KEYCODE_RIGHT; break;
            case EStdKeyDevice3:  aKey = Sexy::KEYCODE_RETURN; break;
            case EStdKeyDevice1:  aKey = Sexy::KEYCODE_ESCAPE; break;
            default: break;
        }
        if (aKey != Sexy::KEYCODE_UNKNOWN) {
            iLawnApp->mWidgetManager->KeyDown(aKey);
            return EKeyWasConsumed;
        }
    }
    return EKeyWasNotConsumed;
}