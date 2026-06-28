/*
 * ReanimLoader.cpp -- .reanim.compiled file loader for Symbian port.
 *
 * Parses PopCap compiled reanimation binary format.
 * Based on upstream Definition.cpp DefMapReadFromCache logic.
 *
 * Binary format (reverse-engineered from Definition.cpp):
 * 1. Raw ReanimatorDefinition struct (16 bytes on 32-bit):
 *    - [0] mTracks.tracks (ptr, 4 bytes, ignored)
 *    - [4] mTracks.count (int, 4 bytes) = track count
 *    - [8] mFPS (float, 4 bytes)
 *    - [12] mReanimAtlas (ptr, 4 bytes, NULL)
 * 2. Track array fixup:
 *    a. int defSize (sizeof(ReanimatorTrack) on desktop, ~12 bytes)
 *    b. Raw track data: defSize * trackCount bytes
 *       Per track (offsets within raw data):
 *       - [0] mName (ptr, 4 bytes, ignored)
 *       - [4] mTransforms.mTransforms (ptr, 4 bytes, ignored)
 *       - [8] mTransforms.count (int, 4 bytes) = transform count
 *    c. Per track fixup:
 *       - DT_STRING "name": int len + chars
 *       - DT_ARRAY "t" (transforms):
 *         i.  int transDefSize (sizeof(ReanimatorTransform) on desktop)
 *         ii. Raw transform data: transDefSize * transformCount bytes
 *             Per transform (first 32 bytes are floats):
 *             - [0]  mTransX (float)
 *             - [4]  mTransY (float)
 *             - [8]  mSkewX (float)
 *             - [12] mSkewY (float)
 *             - [16] mScaleX (float)
 *             - [20] mScaleY (float)
 *             - [24] mFrame (float)
 *             - [28] mAlpha (float)
 *         iii. Per transform fixup:
 *              - DT_IMAGE "i": int len + chars (image name)
 *              - DT_FONT "font": int len + chars (font name)
 *              - DT_STRING "text": int len + chars (text)
 */
#include "ReanimLoader.h"
#include "../engine/PvZVfs.h"
#include "../engine/PakInterface.h"
#include "../engine/Image.h"
#include "../engine/MemoryImage.h"
#include "../engine/ResourceManager.h"

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
extern "C" {
#include "miniz.h"
}

#include <string.h>
#include <stdlib.h>

// ===========================================================================
// SMemR — read raw bytes from buffer, advance pointer (matches upstream)
// ===========================================================================
static inline void SMemR(void*& aSrc, void* aDst, int aSize)
{
    memcpy(aDst, aSrc, aSize);
    aSrc = (void*)((unsigned char*)aSrc + aSize);
}

// Read a length-prefixed string from the fixup stream
static char* ReadFixupString(void*& aPtr, void* aEnd)
{
    if ((unsigned char*)aPtr + 4 > (unsigned char*)aEnd)
        return (char*)"";
    int len = 0;
    SMemR(aPtr, &len, 4);
    if (len <= 0)
        return (char*)"";
    if ((unsigned char*)aPtr + len > (unsigned char*)aEnd)
        return (char*)"";
    char* str = new char[len + 1];
    SMemR(aPtr, str, len);
    str[len] = '\0';
    return str;
}

// Read image name from fixup stream and load the image
static Sexy::Image* ReadFixupImage(void*& aPtr, void* aEnd)
{
    if ((unsigned char*)aPtr + 4 > (unsigned char*)aEnd)
        return NULL;
    int len = 0;
    SMemR(aPtr, &len, 4);
    if (len <= 0)
        return NULL;
    if ((unsigned char*)aPtr + len > (unsigned char*)aEnd)
        return NULL;
    char* name = new char[len + 1];
    SMemR(aPtr, name, len);
    name[len] = '\0';

    Sexy::Image* img = NULL;
    if (gResourceManager)
        img = gResourceManager->GetImage(name);

    delete[] name;
    return img;
}

// ===========================================================================
// ReanimDefinition destructor
// ===========================================================================
ReanimDefinition::~ReanimDefinition()
{
    for (int i = 0; i < mTrackCount; i++)
    {
        if (mTracks[i].mName && mTracks[i].mName[0] != '\0')
            delete[] mTracks[i].mName;
        if (mTracks[i].mTransforms)
        {
            for (int j = 0; j < mTracks[i].mTransformCount; j++)
            {
                if (mTracks[i].mTransforms[j].mFontName &&
                    mTracks[i].mTransforms[j].mFontName[0] != '\0')
                    delete[] mTracks[i].mTransforms[j].mFontName;
                if (mTracks[i].mTransforms[j].mText &&
                    mTracks[i].mTransforms[j].mText[0] != '\0')
                    delete[] mTracks[i].mTransforms[j].mText;
            }
            delete[] mTracks[i].mTransforms;
        }
    }
    if (mTracks)
        delete[] mTracks;
}

// ===========================================================================
// ReanimLoadCompiled — load and parse a .reanim.compiled file from PAK
// ===========================================================================
TBool ReanimLoadCompiled(const char* aPakPath, ReanimDefinition& outDefinition)
{
    if (!gPak)
        return EFalse;

    TInt fileSize = gPak->GetFileSize(aPakPath);
    if (fileSize <= 0)
        return EFalse;

    // Read + XOR decrypt
    unsigned char* compressed = (unsigned char*)User::Alloc(fileSize);
    if (!compressed)
        return EFalse;
    TPtr8 buf(compressed, fileSize);
    if (!gPak->ReadFile(aPakPath, buf))
    {
        User::Free(compressed);
        return EFalse;
    }

    // Check header
    if (fileSize < 8)
    {
        User::Free(compressed);
        return EFalse;
    }
    TUint32 cookie = compressed[0] | (compressed[1] << 8) |
                     (compressed[2] << 16) | (compressed[3] << 24);
    if (cookie != 0xDEADFED4)
    {
        User::Free(compressed);
        return EFalse;
    }
    TUint32 uncSize = compressed[4] | (compressed[5] << 8) |
                      (compressed[6] << 16) | (compressed[7] << 24);

    // zlib decompress
    unsigned char* decompressed = (unsigned char*)User::Alloc(uncSize);
    if (!decompressed)
    {
        User::Free(compressed);
        return EFalse;
    }
    mz_ulong uncLen = uncSize;
    int result = mz_uncompress(decompressed, &uncLen,
                               compressed + 8, fileSize - 8);
    User::Free(compressed);
    if (result != MZ_OK)
    {
        User::Free(decompressed);
        return EFalse;
    }

    void* readPtr = decompressed;
    void* endPtr = decompressed + uncSize;

    // Step 1: Read raw ReanimatorDefinition struct (16 bytes on 32-bit)
    // [0:3]  mTracks.tracks (ptr, ignored)
    // [4:7]  mTracks.count (int) = track count
    // [8:11] mFPS (float)
    // [12:15] mReanimAtlas (ptr, ignored)
    if ((unsigned char*)readPtr + 16 > (unsigned char*)endPtr)
    {
        User::Free(decompressed);
        return EFalse;
    }
    int dummyPtr;
    SMemR(readPtr, &dummyPtr, 4);       // mTracks.tracks ptr
    SMemR(readPtr, &outDefinition.mTrackCount, 4);  // mTracks.count
    SMemR(readPtr, &outDefinition.mFPS, 4);         // mFPS
    SMemR(readPtr, &dummyPtr, 4);       // mReanimAtlas ptr

    if (outDefinition.mTrackCount <= 0 || outDefinition.mTrackCount > 500)
    {
        User::Free(decompressed);
        return EFalse;
    }

    // Step 2: Read track array fixup
    // a. int defSize (sizeof(ReanimatorTrack) on desktop)
    if ((unsigned char*)readPtr + 4 > (unsigned char*)endPtr)
    {
        User::Free(decompressed);
        return EFalse;
    }
    int trackDefSize = 0;
    SMemR(readPtr, &trackDefSize, 4);
    if (trackDefSize < 12 || trackDefSize > 100)
    {
        User::Free(decompressed);
        return EFalse;
    }

    // b. Bulk read raw track data: defSize * trackCount bytes
    int trackDataSize = trackDefSize * outDefinition.mTrackCount;
    if ((unsigned char*)readPtr + trackDataSize > (unsigned char*)endPtr)
    {
        User::Free(decompressed);
        return EFalse;
    }
    unsigned char* trackRawData = (unsigned char*)readPtr;
    readPtr = (unsigned char*)readPtr + trackDataSize;

    // Allocate tracks
    outDefinition.mTracks = new ReanimTrack[outDefinition.mTrackCount];

    // c. Per-track fixup
    for (int i = 0; i < outDefinition.mTrackCount; i++)
    {
        unsigned char* trackRaw = trackRawData + i * trackDefSize;

        // Extract transform count from raw data at offset 8
        // (mName ptr [0:3], mTransforms.mTransforms ptr [4:7], mTransforms.count [8:11])
        int transformCount = *(int*)(trackRaw + 8);
        outDefinition.mTracks[i].mTransformCount = transformCount;
        outDefinition.mTracks[i].mTransforms = NULL;

        if (transformCount < 0 || transformCount > 1000)
            transformCount = 0;

        // Fixup: DT_STRING "name"
        outDefinition.mTracks[i].mName = ReadFixupString(readPtr, endPtr);

        // Fixup: DT_ARRAY "t" (transforms)
        if (transformCount > 0)
        {
            // Read transform defSize
            if ((unsigned char*)readPtr + 4 > (unsigned char*)endPtr)
                break;
            int transDefSize = 0;
            SMemR(readPtr, &transDefSize, 4);
            if (transDefSize < 32 || transDefSize > 200)
                break;

            // Bulk read raw transform data
            int transDataSize = transDefSize * transformCount;
            if ((unsigned char*)readPtr + transDataSize > (unsigned char*)endPtr)
                break;
            unsigned char* transRawData = (unsigned char*)readPtr;
            readPtr = (unsigned char*)readPtr + transDataSize;

            // Allocate transforms
            outDefinition.mTracks[i].mTransforms = new ReanimTransform[transformCount];

            // Per-transform fixup
            for (int j = 0; j < transformCount; j++)
            {
                unsigned char* transRaw = transRawData + j * transDefSize;

                // Extract 8 floats from raw data (first 32 bytes)
                ReanimTransform& t = outDefinition.mTracks[i].mTransforms[j];
                t.mTransX = *(float*)(transRaw + 0);
                t.mTransY = *(float*)(transRaw + 4);
                t.mSkewX  = *(float*)(transRaw + 8);
                t.mSkewY  = *(float*)(transRaw + 12);
                t.mScaleX = *(float*)(transRaw + 16);
                t.mScaleY = *(float*)(transRaw + 20);
                t.mFrame  = *(float*)(transRaw + 24);
                t.mAlpha  = *(float*)(transRaw + 28);

                // Fixup: DT_IMAGE "i"
                t.mImage = ReadFixupImage(readPtr, endPtr);

                // Fixup: DT_FONT "font"
                t.mFontName = ReadFixupString(readPtr, endPtr);

                // Fixup: DT_STRING "text"
                t.mText = ReadFixupString(readPtr, endPtr);
            }
        }
    }

    User::Free(decompressed);
    return ETrue;
}

// ===========================================================================
// ReanimFindTrack — find a track by name (case-insensitive)
// ===========================================================================
ReanimTrack* ReanimFindTrack(ReanimDefinition& aDef, const char* aName)
{
    if (!aName)
        return NULL;
    for (int i = 0; i < aDef.mTrackCount; i++)
    {
        if (aDef.mTracks[i].mName)
        {
            const char* s1 = aDef.mTracks[i].mName;
            const char* s2 = aName;
            while (*s1 && *s2)
            {
                char c1 = *s1; if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                char c2 = *s2; if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                if (c1 != c2) break;
                s1++; s2++;
            }
            if (*s1 == '\0' && *s2 == '\0')
                return &aDef.mTracks[i];
        }
    }
    return NULL;
}
