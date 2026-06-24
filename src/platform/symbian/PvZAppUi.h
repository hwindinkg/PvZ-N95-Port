#ifndef PVZAPPUI_H
#define PVZAPPUI_H

#include <aknappui.h>
#include <e32base.h>

class CPvZGameView;
namespace Sexy { class LawnApp; }

class CPvZAppUi : public CAknAppUi
{
public:
    CPvZAppUi();
    ~CPvZAppUi();
    void ConstructL();

    void HandleCommandL(TInt aCommand);
    void HandleForegroundEventL(TBool aForeground);
    TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

    enum { KFrameInterval = 33333 }; // ~30 fps

private:
    static TInt Tick(TAny* aPtr);   // CPeriodic callback (one heartbeat)
    void RenderTick();              // render a single frame

private:
    CPvZGameView*   iGameView;
    Sexy::LawnApp*  iLawnApp;
    CPeriodic*      iTimer;         // heartbeat timer driving the game loop
};

#endif
