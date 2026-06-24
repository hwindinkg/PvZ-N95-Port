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
        f.Write(buf8); f.Close();
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

    // --- Layered EH self-test to pinpoint where unwinding dies ---
    // Test 1: raw C++ throw/catch. If this line logs but "T1 caught" does NOT,
    // pure C++ EH is broken (missing .ARM.exidx / personality) -> compile-flag
    // or runtime-lib problem, NOT typeinfo.
    Log(_L("EH T1: raw C++ throw/catch"));
    {
        volatile TInt caught = 0;
        try { throw (TInt)123; } catch (TInt e) { caught = e; } catch (...) { caught = -1; }
        TBuf<64> b; b.Format(_L("EH T1 caught=%d (expect 123)"), (TInt)caught); Log(b);
    }
    // Test 2: Symbian TRAP/Leave. If T1 passed but this dies, the leave
    // mechanism (TTrap / euser) is the culprit, not generic C++ EH.
    Log(_L("EH T2: about to TRAP User::Leave(-42)"));
    TRAPD(ehTest, User::Leave(-42));
    { TBuf<64> b; b.Format(_L("EH T2 caught err=%d (expect -42)"), ehTest); Log(b); }

    Log(_L("step: BaseConstructL (TRAP)"));
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

    // Put the GL container on the control stack so it receives
    // key/pointer events and participates in redraw -- this was the
    // missing piece vs. the working Whisk3D reference.
    AddToStackL(iGameView);
    Log(_L("step: AddToStackL done"));

    Log(_L("step: start heartbeat timer"));
    iTimer = CPeriodic::NewL(CActive::EPriorityIdle);  // below normal AOs (ICL decoder) -- reference uses sub-idle priority
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