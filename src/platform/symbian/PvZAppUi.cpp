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
    TInt err = f.Open(fs, _L("C:\\Data\\PvZ\\log.txt"), EFileWrite|EFileShareAny);
    if (err == KErrNotFound) err = f.Create(fs, _L("C:\\Data\\PvZ\\log.txt"), EFileWrite|EFileShareAny);
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
    // Register with the Avkon UI framework. Without BaseConstructL the window
    // server never wires up key/command/exit delivery for this app.
    BaseConstructL(EAknEnableSkin);

    Log(_L("Loading main.pak..."));
    gPak = CPvZVfs::NewL();
    gPak->LoadPakL(_L("C:\\Data\\PvZ\\main.pak"));
    Log(_L("main.pak loaded OK"));

    iLawnApp = new (ELeave) LawnApp();
    gLawnApp = iLawnApp;
    iLawnApp->mWidth = 400; iLawnApp->mHeight = 300;
    iLawnApp->mShutdown = false;
    if (!iLawnApp->mWidgetManager) iLawnApp->mWidgetManager = new Sexy::WidgetManager();
    if (!iLawnApp->mGraphics) iLawnApp->mGraphics = new Sexy::Graphics();
    if (!iLawnApp->mResourceManager) iLawnApp->mResourceManager = new ResourceManager();
    gResourceManager = iLawnApp->mResourceManager;

    Log(_L("Calling LawnApp::Init..."));
    iLawnApp->Init();
    Log(_L("LawnApp::Init done"));

    Log(_L("STEP: Creating game view..."));
    iGameView = CPvZGameView::NewL();
    Log(_L("STEP: InitGLES..."));
    iGameView->InitGLES();
    Log(_L("STEP: InitGLES done"));

    // CRITICAL: GL context is now live -- wire it everywhere BEFORE any draw.
    gSexyAppBase = iLawnApp;
    iLawnApp->mGL = iGameView->GetGL();
    // The Graphics object keeps its OWN GL pointer; it was never set, so every
    // WidgetManager::DrawAll() logged "DrawAll: mGL NULL" and drew nothing.
    if (iLawnApp->mGraphics)
        iLawnApp->mGraphics->SetGLInterface(iGameView->GetGL());

    // Drive the game from a heartbeat timer (active object) instead of a tight
    // loop. A blocking loop in ConstructL starved the window server: the app
    // could not be closed, system dialogs were overdrawn by the GL clear colour
    // every frame, and the half-built AppUi got relaunched (E32Main x2).
    Log(_L("Starting heartbeat timer..."));
    iTimer = CPeriodic::NewL(CActive::EPriorityStandard);
    iTimer->Start(KFrameInterval, KFrameInterval, TCallBack(Tick, this));
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