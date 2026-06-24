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

    static unsigned int* gGlobals;
};

} // namespace Sexy

#endif // MEMORY_IMAGE_H
