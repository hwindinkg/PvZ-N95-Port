#include "SexyAppBase.h"
#include "WidgetManager.h"
#include "GLInterface.h"
#include "Graphics.h"

namespace Sexy {

SexyAppBase::SexyAppBase()
    : mWidth(320)
    , mHeight(240)
    , mIsWindowed(true)
    , mActive(true)
    , mRunning(false)
    , mMinimized(false)
    , mExitToTop(false)
    , mPaused(false)
    , mHasFocus(true)
    , mWidgetManager(NULL)
    , mGL(NULL)
    , mGraphics(NULL)
    , mWindow(NULL)
    , mContext(NULL)
    , mSurface(NULL)
    , mSessionID(0)
    , mPlayTimeInactiveSession(0)
    , mLastTimerTime(0)
    , mLastUserInputTick(0)
    , mOnlyAllowOneCopyToRun(false)
    , mInitialized(false)
{
}

SexyAppBase::~SexyAppBase()
{
    Shutdown();
}

void SexyAppBase::Init()
{
    if (mInitialized)
        return;

    mWidgetManager = new WidgetManager();
    mGL = new GLInterface();
    mGraphics = new Graphics();

    if (mGL)
        mGraphics->SetGLInterface(mGL);

    if (mWidgetManager)
    {
        mWidgetManager->mMouseVisible = true;
    }

    mInitialized = true;
}

void SexyAppBase::Start()
{
    mRunning = true;
}

void SexyAppBase::Shutdown()
{
    mRunning = false;

    delete mGraphics;
    mGraphics = NULL;
    delete mWidgetManager;
    mWidgetManager = NULL;
    // GLInterface is owned by the platform layer, not us
    mGL = NULL;

    mInitialized = false;
}

void SexyAppBase::UpdateFrames()
{
    if (mPaused || mMinimized || !mActive)
        return;

    if (mWidgetManager)
    {
        mWidgetManager->UpdateFrame();
        mWidgetManager->DrawScreen(mGraphics);
    }
}

void SexyAppBase::DrawScreen()
{
    if (mWidgetManager && mGraphics)
    {
        mWidgetManager->DrawScreen(mGraphics);
    }

    if (mGL)
    {
        mGL->Redraw();
    }
}

void SexyAppBase::HandleKeyDown(KeyCode key)
{
    if (mWidgetManager)
        mWidgetManager->KeyDown(key);
}

void SexyAppBase::HandleKeyUp(KeyCode key)
{
    if (mWidgetManager)
        mWidgetManager->KeyUp(key);
}

void SexyAppBase::HandleMouseDown(int x, int y, int btn)
{
    if (mWidgetManager)
        mWidgetManager->MouseDown(x, y, btn);
}

void SexyAppBase::HandleMouseUp(int x, int y, int btn)
{
    if (mWidgetManager)
        mWidgetManager->MouseUp(x, y, btn);
}

void SexyAppBase::HandleMouseMove(int x, int y)
{
    if (mWidgetManager)
        mWidgetManager->MouseMove(x, y);
}

void SexyAppBase::CloseRequestAsync()
{
    mRunning = false;
    mExitToTop = true;
}

// ---- Stub implementations for PopCap-like methods ----

void SexyAppBase::WriteToRegistry() {}
void SexyAppBase::ReadFromRegistry() {}
bool SexyAppBase::DebugKeyDown(int) { return false; }
void SexyAppBase::ShowResourceError(bool) {}
void SexyAppBase::SwitchScreenMode(bool, bool, bool) {}
bool SexyAppBase::OpenURL(const std::string&, bool) { return false; }
void SexyAppBase::URLOpenFailed(const std::string&) {}
void SexyAppBase::URLOpenSucceeded(const std::string&) {}
Dialog* SexyAppBase::DoDialog(int, bool, const std::string&, const std::string&, const std::string&, int) { return NULL; }
bool SexyAppBase::KillDialog(int) { return false; }
bool SexyAppBase::UpdateAppStep(bool*) { return false; }
bool SexyAppBase::UpdateApp() { return false; }

void SexyAppBase::PlaySample(intptr_t) {}

void SexyAppBase::RegistryWriteString(const char*, const std::string&) {}
std::string SexyAppBase::RegistryReadString(const char*) { return std::string(); }
int SexyAppBase::GetInteger(const char*, int defaultValue) { return defaultValue; }

} // namespace Sexy
