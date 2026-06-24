/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Symbian S60 3rd FP1 PAK archive reader.
 * Uses Seek + Read for maximum compatibility with Symbian OS v9.2
 * file server API.
 */

#include "PvZVfs.h"
#include "Common.h"
#include <string.h>  // strlen, strcmp, Mem::Copy

// ---------------------------------------------------------------------------
// PAK format constants  (from PvZ-Portable PakInterface.cpp)
// ---------------------------------------------------------------------------
static const TUint32 KPAKMagic    = 0xBAC04AC0;
static const TUint32 KPAKVersion  = 0;
static const TUint8  KPAKXorKey   = 0xF7;

static const TUint8  KFILEFLAGS_END = 0x80;

static const TInt    KNameLenMax    = 255;
static const TInt    KEntryGrow     = 64;

// ===========================================================================
// Two-phase construction
// ===========================================================================
CPvZVfs* CPvZVfs::NewL()
{
    CPvZVfs* self = new (ELeave) CPvZVfs();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CPvZVfs* CPvZVfs::NewLC()
{
    CPvZVfs* self = new (ELeave) CPvZVfs();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;  // left on CleanupStack
}

CPvZVfs::CPvZVfs()
    : iEntryCount(0)
    , iDataStart(0)
    , iEntries(NULL)
    , iReadBuffer(NULL)
{
}

void CPvZVfs::ConstructL()
{
    User::LeaveIfError(iFs.Connect());
    // Allocate a reusable read buffer (max entry header size).
    iReadBuffer = HBufC8::NewL(KNameLenMax + 16);
}

CPvZVfs::~CPvZVfs()
{
    iPakFile.Close();
    iFs.Close();
    if (iEntries)
    {
        for (TInt i = 0; i < iEntryCount; ++i)
            delete[] const_cast<char*>(iEntries[i].iName);
        delete[] iEntries;
        iEntries = NULL;
    }
    delete iReadBuffer;
}

// ===========================================================================
// LoadPakL  --  open PAK, validate header, parse entry table
// ===========================================================================
void CPvZVfs::LoadPakL(const TDesC& aPakPath)
{
    iPakFile.Close();

    // Free old entries (keep the read buffer)
    if (iEntries)
    {
        for (TInt i = 0; i < iEntryCount; ++i)
            delete[] const_cast<char*>(iEntries[i].iName);
        delete[] iEntries;
        iEntries      = NULL;
        iEntryCount   = 0;
        iDataStart    = 0;
    }

    User::LeaveIfError(iPakFile.Open(iFs, aPakPath,
                                     EFileRead | EFileShareReadersOnly));

    ParseTableL(aPakPath);
}

// ===========================================================================
// ParseTableL  --  walk entries until end-of-table marker
// ===========================================================================
void CPvZVfs::ParseTableL(const TDesC& /*aPakPath*/)
{
    TInt aFileSize = 0;
    User::LeaveIfError(iPakFile.Size(aFileSize));

    // We will accumulate entries in a dynamic array (realloc-style).
    TInt aCapacity = 0;
    TInt aCount    = 0;
    SPakEntry* aArr = NULL;

    TUint32 aOffset = 8;  // past magic + version

    for (;;)
    {
        // ---- Read flags byte ----
        TUint8 aFlags = 0;
        {
            TPckgBuf<TUint8> aPckg;
            TInt pos = aOffset;
            User::LeaveIfError(iPakFile.Seek(ESeekStart, pos));
            User::LeaveIfError(iPakFile.Read(aPckg, 1));
            aFlags = aPckg();
        }
        aFlags ^= KPAKXorKey;
        aOffset += 1;

        if (aFlags & KFILEFLAGS_END)
            break;

        // ---- Read name length ----
        TUint8 aNameLen = 0;
        {
            TPckgBuf<TUint8> aPckg;
            TInt pos = aOffset;
            User::LeaveIfError(iPakFile.Seek(ESeekStart, pos));
            User::LeaveIfError(iPakFile.Read(aPckg, 1));
            aNameLen = aPckg();
        }
        aNameLen ^= KPAKXorKey;
        aOffset += 1;

        if (aNameLen == 0 || aNameLen > KNameLenMax)
            User::Leave(KErrCorrupt);

        // ---- Read name + srcSize(4) + fileTime(8) ----
        TInt aRecSize = aNameLen + 12;
        TPtr8 aBuf = iReadBuffer->Des();
        aBuf.SetLength(aRecSize);
        {
            TInt pos = aOffset;
            User::LeaveIfError(iPakFile.Seek(ESeekStart, pos));
            User::LeaveIfError(iPakFile.Read(aBuf, aRecSize));
        }
        aOffset += aRecSize;

        // XOR-decrypt
        {
            TUint8* p = const_cast<TUint8*>(aBuf.Ptr());
            for (TInt i = 0; i < aRecSize; ++i)
                p[i] ^= KPAKXorKey;
        }

        // ---- Extract filename ----
        char aName[256];
        Mem::Copy(aName, aBuf.Ptr(), aNameLen);
        aName[aNameLen] = '\0';

        // Normalise path separators
        for (TInt i = 0; i < (TInt)aNameLen; ++i)
        {
            if (aName[i] == '\\')
                aName[i] = '/';
        }

        // ---- Extract srcSize (LE int32 at offset aNameLen) ----
        TUint32 aSrcSize = aBuf[aNameLen]
                   | ((TUint32)aBuf[aNameLen + 1] << 8)
                   | ((TUint32)aBuf[aNameLen + 2] << 16)
                   | ((TUint32)aBuf[aNameLen + 3] << 24);

        // ---- Grow array if needed ----
        if (aCount >= aCapacity)
        {
            TInt aNewCap = aCapacity + KEntryGrow;
            SPakEntry* aNewArr = new (ELeave) SPakEntry[aNewCap];
            for (TInt i = 0; i < aCount; ++i)
            {
                aNewArr[i].iName   = aArr[i].iName;
                aNewArr[i].iOffset = aArr[i].iOffset;
                aNewArr[i].iSize   = aArr[i].iSize;
            }
            delete[] aArr;
            aArr      = aNewArr;
            aCapacity = aNewCap;
        }

        // ---- Store entry (offset patched after the loop) ----
        SPakEntry& e = aArr[aCount];
        e.iName = new (ELeave) char[strlen(aName) + 1];
        strcpy(const_cast<char*>(e.iName), aName);
        e.iOffset = 0;
        e.iSize   = aSrcSize;

        ++aCount;
    }

    // aOffset is now the start of the file data section.
    iDataStart = aOffset;

    // Patch offsets: each entry's data starts at iDataStart + running pos.
    {
        TUint32 aRunningPos = 0;
        for (TInt i = 0; i < aCount; ++i)
        {
            aArr[i].iOffset = iDataStart + aRunningPos;
            aRunningPos += aArr[i].iSize;
        }
    }

    // Store the parsed table
    iEntries     = aArr;
    iEntryCount  = aCount;
}

// ===========================================================================
// GetEntryIndex  --  case-insensitive linear search
// ===========================================================================
TInt CPvZVfs::GetEntryIndex(const char* aFileName) const
{
    if (!aFileName)
        return KErrArgument;

    for (TInt i = 0; i < iEntryCount; ++i)
    {
        // Case-insensitive comparison
        const char* s1 = iEntries[i].iName;
        const char* s2 = aFileName;
        while (*s1 && *s2)
        {
            char c1 = *s1;
            char c2 = *s2;
            if (c1 >= 'A' && c1 <= 'Z') c1 += ('a' - 'A');
            if (c2 >= 'A' && c2 <= 'Z') c2 += ('a' - 'A');
            if (c1 != c2)
                break;
            ++s1;
            ++s2;
        }
        if (*s1 == '\0' && *s2 == '\0')
            return i;
    }
    return KErrNotFound;
}

// ===========================================================================
// DumpEntries  --  write all entry names to a log file
// ===========================================================================
void CPvZVfs::DumpEntries(const TDesC& aLogPath)
{
    RFs fs;
    if (fs.Connect() != KErrNone) return;
    RFile f;
    TInt err = f.Replace(fs, aLogPath, EFileWrite);
    if (err == KErrNone)
    {
        TBuf8<512> line;
        for (TInt i = 0; i < iEntryCount; ++i)
        {
            const char* name = iEntries[i].iName;
            if (name)
            {
                line.Zero();
                TInt len = Min((TInt)strlen(name), 500);
                line.Append((const TUint8*)name, len);
                line.Append('\n');
                f.Write(line);
            }
        }
        f.Close();
    }
    fs.Close();
}

// ===========================================================================
// ReadFile (descriptor version)
// ===========================================================================
TBool CPvZVfs::ReadFile(const char* aFileName, TPtr8& aBuffer)
{
    TInt idx = GetEntryIndex(aFileName);
    if (idx == KErrNotFound)
        return EFalse;

    const SPakEntry& e = iEntries[idx];
    if (aBuffer.MaxLength() < (TInt)e.iSize)
        return EFalse;  // buffer too small

    aBuffer.SetLength(e.iSize);
    TInt pos = e.iOffset;
    if (iPakFile.Seek(ESeekStart, pos) != KErrNone)
        return EFalse;
    if (iPakFile.Read(aBuffer, e.iSize) != KErrNone)
        return EFalse;

    // XOR-decrypt
    XorData(aBuffer);
    return ETrue;
}

// ===========================================================================
// ReadFile (heap-allocated version)
// ===========================================================================
TBool CPvZVfs::ReadFile(const char* aFileName, void** aOutBuffer, TInt* aOutSize)
{
    if (!aFileName || !aOutBuffer || !aOutSize)
        return EFalse;

    TInt idx = GetEntryIndex(aFileName);
    if (idx == KErrNotFound)
        return EFalse;

    const SPakEntry& e = iEntries[idx];

    char* aBuf = new (ELeave) char[e.iSize];
    if (!aBuf)
        return EFalse;

    TPtr8 aDes(reinterpret_cast<TUint8*>(aBuf), e.iSize);
    aDes.SetLength(e.iSize);

    TInt pos = e.iOffset;
    if (iPakFile.Seek(ESeekStart, pos) != KErrNone)
    {
        delete[] aBuf;
        return EFalse;
    }
    if (iPakFile.Read(aDes, e.iSize) != KErrNone)
    {
        delete[] aBuf;
        return EFalse;
    }

    // XOR-decrypt
    {
        TUint8* p = reinterpret_cast<TUint8*>(aBuf);
        for (TUint32 j = 0; j < e.iSize; ++j)
            p[j] ^= KPAKXorKey;
    }

    *aOutBuffer = aBuf;
    *aOutSize   = static_cast<TInt>(e.iSize);
    return ETrue;
}

// ===========================================================================
// FileExists
// ===========================================================================
TBool CPvZVfs::FileExists(const char* aFileName)
{
    return GetEntryIndex(aFileName) != KErrNotFound;
}

// ===========================================================================
// GetFileSize
// ===========================================================================
TInt CPvZVfs::GetFileSize(const char* aFileName)
{
    TInt idx = GetEntryIndex(aFileName);
    if (idx == KErrNotFound)
        return KErrNotFound;
    return static_cast<TInt>(iEntries[idx].iSize);
}

// ===========================================================================
// XorData  --  XOR every byte with KPAKXorKey
// ===========================================================================
void CPvZVfs::XorData(TDes8& aBuffer)
{
    TInt len = aBuffer.Length();
    TUint8* p = const_cast<TUint8*>(aBuffer.Ptr());
    for (TInt i = 0; i < len; ++i)
        p[i] ^= KPAKXorKey;
}
