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

    // M4 #2 -- pointer (touch) event forwarding. The N95 has a touch-enabled
    // variant (N95 8GB / 5800 share the S60 3rd FP1 platform); even on the
    // original N95, the d-pad can be synthesised into pointer events. We
    // forward raw pointer events to WidgetManager::MouseDown/MouseUp/MouseMove.
    void HandlePointerEventL(const TPointerEvent& aPointerEvent);

    enum { KFrameInterval = 33333 }; // ~30 fps

private:
    static TInt Tick(TAny* aPtr);   // CPeriodic callback (one heartbeat)
    void RenderTick();              // render a single frame

    // M4 #2 -- virtual cursor for d-pad navigation. The N95 d-pad generates
    // arrow key events; we synthesise MouseMove events by moving this cursor
    // 16px per key press, and MouseDown/MouseUp on the centre key.
    void InitVirtualCursor();
    void SynthesizeMouseMove();
    void SynthesizeMouseClick();

private:
    CPvZGameView*   iGameView;
    Sexy::LawnApp*  iLawnApp;
    CPeriodic*      iTimer;         // heartbeat timer driving the game loop

    // Virtual cursor position (logical 400x300 canvas). Moved by d-pad arrows;
    // centre key synthesises a MouseDown+MouseUp at this position.
    int             iCursorX;
    int             iCursorY;
    bool            iCursorVisible;
    bool            iCentreKeyDown;  // debounce for centre-key clicks
};

#endif
