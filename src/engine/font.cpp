#include "Font.h"
#include "Graphics.h"
#include "Image.h"

namespace Sexy {

// ===========================================================================
// Font base
// ===========================================================================

Font::Font()
    : mFontHeight(12)
    , mAscent(10)
    , mMaxCharWidth(8)
{
}

Font::~Font()
{
}

void Font::DrawString(Graphics* g, int x, int y,
                      const char* text, int theLength,
                      const Color* theColor)
{
    if (!g || !text)
        return;

    UNUSED(theLength);

    if (theColor)
        g->SetColor(*theColor);

    g->SetFont(this);

    // Default: draw placeholder rects
    int len = 0;
    while (text[len] != '\0') len++;
    g->FillRect(x, y - mAscent, len * mMaxCharWidth, mFontHeight);
}

int Font::StringWidth(const char* text)
{
    int len = 0;
    while (text[len] != '\0') len++;
    return len * mMaxCharWidth;
}

int Font::CharWidth(char /*c*/) const
{
    return mMaxCharWidth;
}

// ===========================================================================
// ImageFont
// ===========================================================================

ImageFont::ImageFont(Image* theImage, int theCellWidth, int theCellHeight,
                     const char* theCharMap, int theCharCount)
    : mImage(theImage)
    , mCellWidth(theCellWidth)
    , mCellHeight(theCellHeight)
    , mCharCount(theCharCount)
    , mCharMap(NULL)
{
    if (theCharMap)
    {
        mCharMap = new char[theCharCount + 1];
        int i;
        for (i = 0; i < theCharCount; i++)
            mCharMap[i] = theCharMap[i];
        mCharMap[theCharCount] = '\0';
    }

    // Build offset table for ASCII chars
    int i;
    for (i = 0; i < 256; i++)
        mCharOffsets[i] = -1;

    for (i = 0; i < theCharCount; i++)
    {
        unsigned char c = (unsigned char)theCharMap[i];
        mCharOffsets[c] = i * theCellWidth;
    }

    mFontHeight = theCellHeight;
    mMaxCharWidth = theCellWidth;
}

ImageFont::~ImageFont()
{
    delete[] mCharMap;
}

void ImageFont::DrawString(Graphics* g, int x, int y,
                           const char* text, int theLength,
                           const Color* theColor)
{
    if (!g || !text)
        return;

    if (theColor)
        g->SetColor(*theColor);

    int curX = x;
    while (*text)
    {
        unsigned char c = (unsigned char)*text;
        int offset = mCharOffsets[c];
        if (offset >= 0)
        {
            Rect srcRect(offset, 0, mCellWidth, mCellHeight);
            g->DrawImage((MemoryImage*)mImage, curX, y - mCellHeight, srcRect);
        }
        curX += mCellWidth;
        text++;
    }
}

int ImageFont::StringWidth(const char* text)
{
    int len = 0;
    while (text[len] != '\0') len++;
    return len * mCellWidth;
}

int ImageFont::CharWidth(char /*c*/) const
{
    return mCellWidth;
}

} // namespace Sexy
Sexy::Font* Sexy::FONT_DWARVEN = NULL;
Sexy::Font* Sexy::FONT_COUNTER = NULL;
