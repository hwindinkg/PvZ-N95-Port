#include "Common.h"
#include "MemoryImage.h"

namespace Sexy {

uint32_t* MemoryImage::gGlobals = NULL;

MemoryImage::MemoryImage()
    : Image()
    , mBits(NULL)
    , mColorTable(NULL)
    , mColorTableSize(0)
    , mDataSize(0)
    , mOwnsBits(false)
{
}

MemoryImage::~MemoryImage()
{
    DeleteBits();
    delete[] mColorTable;
    mColorTable = NULL;
}

void MemoryImage::SetSize(int w, int h)
{
    Image::SetSize(w, h);

    // Free existing bits if we own them
    if (mOwnsBits && mBits)
    {
        delete[] mBits;
        mBits = NULL;
    }

    mDataSize = w * h * 4; // 4 bytes per pixel (RGBA 8888)
    mBits = new unsigned char[mDataSize];
    mOwnsBits = true;

    // Zero out the pixel buffer
    if (mBits)
    {
        TInt i;
        for (i = 0; i < mDataSize; i++)
        {
            mBits[i] = 0;
        }
    }
}

void MemoryImage::SetBits(unsigned char* bits, int size, bool own)
{
    // Free existing bits if we own them
    if (mOwnsBits && mBits)
    {
        delete[] mBits;
    }

    mBits = bits;
    mDataSize = size;
    mOwnsBits = own;
}

void MemoryImage::Palletedize()
{
    // Stub: paletted image conversion not yet implemented
    // In the future this will convert 32-bit RGBA to a paletted format
    // with a color table for memory-efficient storage.
}

void MemoryImage::DeleteBits()
{
    if (mOwnsBits && mBits)
    {
        delete[] mBits;
    }
    mBits = NULL;
    mDataSize = 0;
    mOwnsBits = false;
}

void MemoryImage::ApplyColorKey(unsigned int keyColor, int tolerance)
{
    if (!mBits || mWidth <= 0 || mHeight <= 0)
        return;

    // bits format: 0xAARRGGBB, one uint32 per pixel (native endian).
    unsigned int* pixels = (unsigned int*)mBits;
    int pixelCount = mWidth * mHeight;

    // Extract channels from keyColor (0xAARRGGBB).
    int keyR = (int)((keyColor >> 16) & 0xFF);
    int keyG = (int)((keyColor >> 8)  & 0xFF);
    int keyB = (int)((keyColor)       & 0xFF);

    for (int i = 0; i < pixelCount; i++)
    {
        unsigned int px = pixels[i];
        int r = (int)((px >> 16) & 0xFF);
        int g = (int)((px >> 8)  & 0xFF);
        int b = (int)((px)       & 0xFF);

        // Check if pixel matches keyColor within tolerance.
        int dr = r - keyR; if (dr < 0) dr = -dr;
        int dg = g - keyG; if (dg < 0) dg = -dg;
        int db = b - keyB; if (db < 0) db = -db;
        if (dr <= tolerance && dg <= tolerance && db <= tolerance)
        {
            // Make fully transparent, preserve RGB (irrelevant when alpha=0).
            pixels[i] = 0x00000000;
        }
        else
        {
            // Ensure opaque (JPEG decodes with alpha=0xFF already, but be safe).
            pixels[i] = px | 0xFF000000;
        }
    }
}

} // namespace Sexy
