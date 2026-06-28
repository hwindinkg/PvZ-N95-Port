/*
 * SystemFont.h -- minimal bitmap font for Symbian port.
 *
 * Generates a 128x56 MemoryImage (16 cols x 7 rows of 8x8 glyphs) covering
 * ASCII 32-127 at startup. Used as fallback when PvZ font assets (.dat + .png)
 * are not available in the PAK (M4 #4 -- GetFontThrow was a stub).
 *
 * This is NOT a 1:1 port of upstream ImageFont (which uses font description
 * files with per-char metrics + kerning). It's a functional fallback so text
 * is visible. Once font assets are available, replace with real ImageFont.
 *
 * Glyph data: 8x8 monospace, 1 bit per pixel, hardcoded for ASCII printables.
 * Inspired by classic PC BIOS fonts.
 */
#ifndef __SYSTEMFONT_H__
#define __SYSTEMFONT_H__

#include "Font.h"
#include "MemoryImage.h"

// [M4 #4 fix] _Font is typedef'd to Sexy::Font here (and in Stubs.h).
// This header is included by ResourceManager.cpp which needs the typedef
// to convert SystemFont* -> _Font* in GetFontThrow's return. Stubs.h also
// has this typedef; the #ifndef guard prevents double-definition.
// Previously _Font was an opaque forward-declared class in ResourceManager.h,
// making it incompatible with Sexy::Font* used by Graphics::SetFont.
#ifndef _FONT_TYPEDEF_DEFINED
#define _FONT_TYPEDEF_DEFINED
typedef Sexy::Font _Font;
#endif

namespace Sexy {

class SystemFont : public Font
{
public:
    SystemFont();
    virtual ~SystemFont();

    // Generates the font sheet MemoryImage (call once at startup).
    // Returns true on success. Idempotent -- safe to call multiple times.
    static bool Initialize();

    // Font interface.
    virtual void DrawString(Graphics* g, int x, int y,
                            const char* text, int length = -1,
                            const Color* theColor = NULL);
    virtual int  StringWidth(const char* text);
    virtual int  CharWidth(char c) const { return 8; }

    // Singleton accessor -- returns the shared SystemFont instance.
    // Returns NULL if Initialize() hasn't been called or failed.
    static SystemFont* Get();

private:
    static MemoryImage* sFontSheet;   // 128x56 bitmap, shared
    static SystemFont*  sInstance;
};

} // namespace Sexy

#endif // __SYSTEMFONT_H__
