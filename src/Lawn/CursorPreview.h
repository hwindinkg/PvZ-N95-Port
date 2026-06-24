#ifndef __CURSORPREVIEW_H__
#define __CURSORPREVIEW_H__

#include <e32def.h>
#include "../engine/Widget.h"

class CursorPreview : public Sexy::Widget
{
public:
    CursorPreview() {}
    void Draw(Sexy::Graphics* g) { (void)g; }
    bool BeginDraw(Sexy::Graphics* g) { (void)g; return false; }
    void EndDraw(Sexy::Graphics* g) { (void)g; }
};

#endif
