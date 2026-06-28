#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#include <e32base.h>
#include "Image.h"

namespace Sexy {

class MemoryImage : public Image
{
public:
    MemoryImage();
    virtual ~MemoryImage();

    unsigned char* mBits;           // raw pixel data in native format
    unsigned char* mColorTable;     // for paletted images
    int      mColorTableSize;
    int      mDataSize;       // size of mBits in bytes
    bool     mOwnsBits;       // if true, delete[] mBits on destroy

    virtual void SetSize(int w, int h);
    void SetBits(unsigned char* bits, int size, bool own);
    unsigned char* GetBits() const { return mBits; }
    int GetDataSize() const { return mDataSize; }
    void Palletedize();
    void DeleteBits();

    // [M4 fix] Apply a colorkey: pixels matching `keyColor` (with optional
    // tolerance) become fully transparent (alpha=0). Used for JPEG assets
    // that should have transparency but don't (e.g. IMAGE_PVZ_LOGO is stored
    // as pvz_logo.jpg in the PAK -- JPEG has no alpha channel, so the logo's
    // black background renders as solid black instead of transparent).
    // bits format is 0xAARRGGBB (one uint32 per pixel, native endian).
    void ApplyColorKey(unsigned int keyColor, int tolerance = 0);

    static unsigned int* gGlobals;
};

} // namespace Sexy

#endif // MEMORY_IMAGE_H
