/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 */

#include "Common.h"
#include "PakInterface.h"
#include "PvZVfs.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
CPvZVfs* gPak    = NULL;   // must be set by application init
PFILE*   gPakPtr = NULL;

// ---------------------------------------------------------------------------
// PakOpenCur  --  look up aFileName in the global PAK, set gPakPtr
// ---------------------------------------------------------------------------
TInt PakOpenCur(const char* aFileName)
{
    // Close any previously opened file
    if (gPakPtr)
    {
        delete[] gPakPtr->iName;
        delete gPakPtr;
        gPakPtr = NULL;
    }

    if (!gPak)
        return KErrNotReady;

    // Get the file info first
    TInt aSize = gPak->GetFileSize(aFileName);
    if (aSize == KErrNotFound)
        return KErrNotFound;

    // We need to find the offset too.  We re-use ReadFile to avoid
    // exposing offset internals, but we need the offset for PFILE.
    // Since GetFileSize succeeded the entry exists; we find the offset
    // by reading a 0-size buffer (inefficient but keeps abstraction).
    //
    // Better approach: cache the last-found entry.  For simplicity here
    // we re-derive the info by reading the VFS internal entry.
    // We rely on GetFileSize returning > 0 for existing entries.

    // Allocate PFILE
    PFILE* aFile = new PFILE;
    if (!aFile)
        return KErrNoMemory;

    aFile->iName   = new char[strlen(aFileName) + 1];
    if (!aFile->iName)
    {
        delete aFile;
        return KErrNoMemory;
    }
    strcpy(const_cast<char*>(aFile->iName), aFileName);
    aFile->iSize = static_cast<TUint32>(aSize);
    aFile->iPos  = 0;

    // To get the offset, we read the entry from the VFS and copy it.
    // Actually, we don't need the offset for the public API since
    // we read through the VFS.  But we store it for completeness.
    aFile->iOffset = 0;  // not used externally; reads go through gPak

    gPakPtr = aFile;
    return KErrNone;
}

// ---------------------------------------------------------------------------
// PakGetFileSize
// ---------------------------------------------------------------------------
TInt PakGetFileSize()
{
    if (!gPakPtr)
        return 0;
    return static_cast<TInt>(gPakPtr->iSize);
}

// ---------------------------------------------------------------------------
// PakReadFile  --  read from the current file, advancing iPos
// ---------------------------------------------------------------------------
TInt PakReadFile(void* aBuffer, TInt aSize)
{
    if (!gPakPtr || !aBuffer)
        return 0;

    if (gPakPtr->iPos >= gPakPtr->iSize)
        return 0;  // already at EOF

    // Clamp read size to remaining bytes
    TUint32 aRemaining = gPakPtr->iSize - gPakPtr->iPos;
    TUint32 aReadSize = (static_cast<TUint32>(aSize) < aRemaining)
                        ? static_cast<TUint32>(aSize) : aRemaining;

    if (aReadSize == 0)
        return 0;

    // Use the VFS's ReadFile for simplicity.  We read the whole entry,
    // then copy the requested slice.  This is not optimal for large files
    // but keeps the implementation simple.  For better performance the
    // VFS should expose offset-based partial reads.
    //
    // Optimisation: read only the slice we need.
    void* aFullBuf = NULL;
    TInt aFullSize = 0;

    if (!gPak->ReadFile(gPakPtr->iName, &aFullBuf, &aFullSize))
        return 0;

    if (gPakPtr->iPos + aReadSize > aFullSize)
        aReadSize = aFullSize - gPakPtr->iPos;

    Mem::Copy(aBuffer, static_cast<const TUint8*>(aFullBuf) + gPakPtr->iPos, aReadSize);
    gPakPtr->iPos += aReadSize;

    delete[] static_cast<char*>(aFullBuf);
    return static_cast<TInt>(aReadSize);
}

// ---------------------------------------------------------------------------
// PakGetAvailable  --  remaining bytes in the current file
// ---------------------------------------------------------------------------
TInt PakGetAvailable()
{
    if (!gPakPtr)
        return 0;

    TInt aRemaining = static_cast<TInt>(gPakPtr->iSize - gPakPtr->iPos);
    return (aRemaining > 0) ? aRemaining : 0;
}
