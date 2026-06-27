/*
 * ReanimLoader.cpp -- minimal .reanim.compiled file loader for Symbian port.
 *
 * Parses PopCap compiled reanimation binary format.
 * Based on upstream Definition.cpp DefMapReadFromCache logic.
 */
#include "ReanimLoader.h"
#include "../engine/PvZVfs.h"
#include "../engine/PakInterface.h"
#include "../engine/Image.h"
#include "../engine/MemoryImage.h"
#include "../engine/ResourceManager.h"

// miniz.h includes stdio.h (for FILE*) which conflicts with Common.h's
// C++ linkage declarations of sprintf/vsprintf. Wrap in extern "C" to
// force C linkage on all miniz declarations.
extern "C" {
#include "miniz.h"
}

#include <string.h>
#include <stdlib.h>

// ===========================================================================
// SMemR — read raw bytes from buffer, advance pointer (matches upstream)
// ===========================================================================
static void SMemR(void*& aSrc, void* aDst, int aSize)
{
    memcpy(aDst, aSrc, aSize);
    aSrc = (void*)((unsigned char*)aSrc + aSize);
}

static char* SMemRStr(void*& aSrc)
{
    int len = 0;
    SMemR(aSrc, &len, sizeof(int));
    if (len <= 0)
        return (char*)"";
    char* str = new char[len + 1];
    SMemR(aSrc, str, len);
    str[len] = '\0';
    return str;
}

static Sexy::Image* SMemRImage(void*& aSrc)
{
    int len = 0;
    SMemR(aSrc, &len, sizeof(int));
    if (len <= 0)
        return NULL;
    char* name = new char[len + 1];
    SMemR(aSrc, name, len);
    name[len] = '\0';

    // Load image via ResourceManager
    Sexy::Image* img = NULL;
    if (gResourceManager)
        img = gResourceManager->GetImage(name);

    delete[] name;
    return img;
}

// ===========================================================================
// ReanimDefinition destructor — free allocated memory
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
    // Step 1: Read file from PAK
    if (!gPak)
        return EFalse;

    TInt fileSize = gPak->GetFileSize(aPakPath);
    if (fileSize <= 0)
        return EFalse;

    // Read + XOR decrypt
    unsigned char* compressed = new unsigned char[fileSize];
    TPtr8 buf(compressed, fileSize);
    if (!gPak->ReadFile(aPakPath, buf))
    {
        delete[] compressed;
        return EFalse;
    }

    // Step 2: Check header (cookie 0xDEADFED4 + uncompressedSize)
    if (fileSize < 8)
    {
        delete[] compressed;
        return EFalse;
    }
    TUint32 cookie = compressed[0] | (compressed[1] << 8) |
                     (compressed[2] << 16) | (compressed[3] << 24);
    if (cookie != 0xDEADFED4)
    {
        delete[] compressed;
        return EFalse;
    }
    TUint32 uncSize = compressed[4] | (compressed[5] << 8) |
                      (compressed[6] << 16) | (compressed[7] << 24);

    // Step 3: zlib decompress
    unsigned char* decompressed = new unsigned char[uncSize];
    mz_ulong uncLen = uncSize;
    int result = mz_uncompress(decompressed, &uncLen,
                               compressed + 8, fileSize - 8);
    delete[] compressed;
    if (result != MZ_OK)
    {
        delete[] decompressed;
        return EFalse;
    }

    // Step 4: Parse binary format
    // ReanimatorDefinition binary layout (from upstream DefMap):
    //   - DT_ARRAY "track": int defSize + int count + raw data + fixup
    //   - DT_FLOAT "fps": float
    //   - DT_UNKNOWN (mReanimAtlas): skip (pointer, 4 bytes in raw data)
    //
    // But the ACTUAL binary layout from the compiled file is:
    //   [8 bytes: header — 2 ints, probably schema hash + version]
    //   [4 bytes: track count]
    //   [4 bytes: mFPS (float)]
    //   [4 bytes: mReanimAtlas (pointer = 0)]
    //   For each track:
    //     [4 bytes: transform defSize (sizeof(ReanimatorTransform))]
    //     [4 bytes: transform count]
    //     [transformCount * rawTransformSize: raw transform data]
    //     For each transform (fixup phase):
    //       DT_IMAGE: read string (image name), load image
    //       DT_FONT: read string (font name)
    //       DT_STRING: read string (text)
    //     DT_STRING "name": read string (track name)

    void* readPtr = decompressed;

    // Skip 8-byte header
    readPtr = (unsigned char*)readPtr + 8;

    // Read track count
    int trackCount = 0;
    SMemR(readPtr, &trackCount, sizeof(int));
    outDefinition.mTrackCount = trackCount;

    // Read FPS
    SMemR(readPtr, &outDefinition.mFPS, sizeof(float));

    // Skip mReanimAtlas pointer (4 bytes)
    readPtr = (unsigned char*)readPtr + 4;

    // Allocate tracks
    outDefinition.mTracks = new ReanimTrack[trackCount];

    // Parse each track
    for (int i = 0; i < trackCount; i++)
    {
        // DT_ARRAY: read defSize
        int defSize = 0;
        SMemR(readPtr, &defSize, sizeof(int));

        // Read transform count
        int transformCount = 0;
        SMemR(readPtr, &transformCount, sizeof(int));

        outDefinition.mTracks[i].mTransformCount = transformCount;

        if (transformCount > 0)
        {
            // Read raw transform data (floats only: x,y,kx,ky,sx,sy,f,a = 8 floats = 32 bytes)
            // BUT the actual struct size is larger (includes Image*, Font*, text*).
            // The raw data has only the float fields. The fixup phase reads
            // image/font/text strings separately.
            // Raw float data per transform = 8 * sizeof(float) = 32 bytes
            const int kRawTransformSize = 8 * sizeof(float);

            outDefinition.mTracks[i].mTransforms = new ReanimTransform[transformCount];

            for (int j = 0; j < transformCount; j++)
            {
                ReanimTransform& t = outDefinition.mTracks[i].mTransforms[j];

                // Read 8 floats: x, y, kx, ky, sx, sy, f, a
                SMemR(readPtr, &t.mTransX, sizeof(float));
                SMemR(readPtr, &t.mTransY, sizeof(float));
                SMemR(readPtr, &t.mSkewX, sizeof(float));
                SMemR(readPtr, &t.mSkewY, sizeof(float));
                SMemR(readPtr, &t.mScaleX, sizeof(float));
                SMemR(readPtr, &t.mScaleY, sizeof(float));
                SMemR(readPtr, &t.mFrame, sizeof(float));
                SMemR(readPtr, &t.mAlpha, sizeof(float));

                // Fixup: DT_IMAGE — read image name string, load image
                t.mImage = SMemRImage(readPtr);

                // Fixup: DT_FONT — read font name string
                t.mFontName = SMemRStr(readPtr);

                // Fixup: DT_STRING — read text string
                t.mText = SMemRStr(readPtr);
            }
        }

        // DT_STRING "name": read track name
        outDefinition.mTracks[i].mName = SMemRStr(readPtr);
    }

    delete[] decompressed;
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
            // Case-insensitive compare
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
