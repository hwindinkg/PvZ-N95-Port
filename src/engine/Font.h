#ifndef FONT_H
#define FONT_H

#include "Common.h"
#include "Color.h"

namespace Sexy {

class Graphics;
class Image;
class MemoryImage;

class Font
{
public:
    Font();
    virtual ~Font();

    virtual void DrawString(Graphics* g, int x, int y,
                            const char* text, int length = -1,
                            const Color* theColor = NULL);
    virtual int StringWidth(const char* text);
    virtual int CharWidth(char c) const;
    virtual int GetHeight() const { return mFontHeight; }
    virtual int GetAscent() const { return mAscent; }

    int mFontHeight;
    int mAscent;
    int mMaxCharWidth;
};

class ImageFont : public Font
{
public:
    ImageFont(Image* theImage, int theCellWidth, int theCellHeight,
              const char* theCharMap, int theCharCount);
    virtual ~ImageFont();

    virtual void DrawString(Graphics* g, int x, int y,
                            const char* text, int length = -1,
                            const Color* theColor = NULL);
    virtual int StringWidth(const char* text);
    virtual int CharWidth(char c) const;

    Image* mImage;
    int mCellWidth;
    int mCellHeight;
    int mCharCount;
    char* mCharMap;
    int mCharOffsets[256]; // x offsets for each char in the sheet
};

} // namespace Sexy

// Font resource constants (used in Sexy namespace)
// Board.cpp calls graphics->SetFont(Sexy::FONT_DWARVEN) etc.
namespace Sexy {
    extern Font* FONT_DWARVEN;
    extern Font* FONT_COUNTER;
}

#endif // FONT_H
