#ifndef SEXYAPPBASE_H
#define SEXYAPPBASE_H

#include "Common.h"
#include "Color.h"
#include "Rect.h"
#include "KeyCodes.h"
#include <string>
#include <map>

namespace Sexy {

class WidgetManager;
class GLInterface;
class Graphics;
class Dialog;

class SexyAppBase
{
public:
    SexyAppBase();
    virtual ~SexyAppBase();

    virtual void Init();
    virtual void Start();
    virtual void Shutdown();
    virtual void UpdateFrames();
    virtual void DrawScreen();
    virtual void HandleKeyDown(KeyCode key);
    virtual void HandleKeyUp(KeyCode key);
    virtual void HandleMouseDown(int x, int y, int btn);
    virtual void HandleMouseUp(int x, int y, int btn);
    virtual void HandleMouseMove(int x, int y);

    // PopCap-like app methods
    virtual void WriteToRegistry();
    virtual void ReadFromRegistry();
    virtual bool DebugKeyDown(int key);
    virtual void ShowResourceError(bool doExit);
    virtual void SwitchScreenMode(bool wantWindowed, bool want3D, bool changeVideoMode);
    virtual bool OpenURL(const std::string& theURL, bool shutdown);
    virtual void URLOpenFailed(const std::string& theURL);
    virtual void URLOpenSucceeded(const std::string& theURL);
    virtual Dialog* DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
    virtual bool KillDialog(int theDialogId);
    virtual bool UpdateAppStep(bool*);
    virtual bool UpdateApp();

    // Sound
    virtual void PlaySample(intptr_t soundNum);

    // Static helpers
    static void RegistryWriteString(const char* key, const std::string& value);
    static std::string RegistryReadString(const char* key);
    static int GetInteger(const char* key, int defaultValue);

    // Dimensions
    int mWidth;
    int mHeight;
    bool mIsWindowed;
    bool mActive;
    bool mRunning;
    bool mMinimized;
    bool mExitToTop;
    bool mPaused;
    bool mHasFocus;

    // Components
    WidgetManager* mWidgetManager;
    GLInterface*   mGL;
    Graphics*      mGraphics;

    // EGL/T platform handles
    void* mWindow;
    void* mContext;
    void* mSurface;

    // Callbacks
    void CloseRequestAsync();

    // Title
    std::string mTitle;

    // Session tracking
    int  mSessionID;
    int  mPlayTimeInactiveSession;
    int  mLastTimerTime;
    int  mLastUserInputTick;
    bool mOnlyAllowOneCopyToRun;

protected:
    bool mInitialized;

    // Dialog map for tracking dialogs by ID
    std::map<int, Dialog*> mDialogMap;
};

    // Global application pointer (defined in the platform entry point)
    extern SexyAppBase* gSexyAppBase;

} // namespace Sexy

#endif // SEXYAPPBASE_H
