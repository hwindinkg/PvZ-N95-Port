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

} // namespace Sexy
