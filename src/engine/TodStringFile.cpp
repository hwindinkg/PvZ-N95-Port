/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Symbian S60 3rd FP1 string table implementation.
 * Loads a text file containing key=value pairs (one per line)
 * into an in-memory lookup table.
 */

#include "Common.h"
#include "TodStringFile.h"
#include <f32file.h>
#include <string.h>

namespace Sexy {

static const TInt KDefaultCapacity = 512;
static const TInt KCapacityGrow    = 256;

// ===========================================================================
// Two-phase construction
// ===========================================================================
TodStringFile* TodStringFile::NewL()
{
    TodStringFile* self = new (ELeave) TodStringFile();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

TodStringFile* TodStringFile::NewLC()
{
    TodStringFile* self = new (ELeave) TodStringFile();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self; // left on CleanupStack
}

TodStringFile::TodStringFile()
    : iEntries(NULL)
    , iEntryCount(0)
    , iEntryCapacity(0)
{
}

void TodStringFile::ConstructL()
{
    iEntries = new (ELeave) SStringEntry[KDefaultCapacity];
    iEntryCapacity = KDefaultCapacity;
    Mem::FillZ(iEntries, iEntryCapacity * sizeof(SStringEntry));
}

TodStringFile::~TodStringFile()
{
    if (iEntries)
    {
        for (TInt i = 0; i < iEntryCount; ++i)
        {
            delete[] iEntries[i].iKey;
            delete[] iEntries[i].iValue;
        }
        delete[] iEntries;
        iEntries = NULL;
    }
}

// ===========================================================================
// GrowL -- expand the entry array
// ===========================================================================
void TodStringFile::GrowL(TInt aMinCap)
{
    TInt aNewCap = iEntryCapacity;
    while (aNewCap < aMinCap)
        aNewCap += KCapacityGrow;

    SStringEntry* aNewArr = new (ELeave) SStringEntry[aNewCap];
    Mem::FillZ(aNewArr, aNewCap * sizeof(SStringEntry));

    // Transfer existing entries (shallow copy of key/value pointers)
    for (TInt i = 0; i < iEntryCount; ++i)
    {
        aNewArr[i].iKey   = iEntries[i].iKey;
        aNewArr[i].iValue = iEntries[i].iValue;
    }

    delete[] iEntries;
    iEntries = aNewArr;
    iEntryCapacity = aNewCap;
}

// ===========================================================================
// Helper: duplicate a C string with new (ELeave) char[]
// ===========================================================================
static char* DupStrL(const char* aStr)
{
    if (!aStr)
        return NULL;

    TInt len = 0;
    while (aStr[len] != '\0')
        ++len;

    char* aCopy = new (ELeave) char[len + 1];
    Mem::Copy(aCopy, aStr, len);
    aCopy[len] = '\0';
    return aCopy;
}

// ===========================================================================
// LoadFromFile -- open a text file, read its content, parse key=value pairs
// ===========================================================================
void TodStringFile::LoadFromFile(const char* aPath)
{
    if (!aPath)
        return;

    // Convert ASCII path to 16-bit descriptor
    TInt aPathLen = 0;
    while (aPath[aPathLen] != '\0')
        ++aPathLen;

    if (aPathLen == 0)
        return;

    TBuf8<256> aPath8;
    aPath8.Copy(reinterpret_cast<const TUint8*>(aPath), aPathLen);

    TBuf<256> aPath16;
    aPath16.Copy(aPath8);

    // Open and read the file
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);

    RFile file;
    User::LeaveIfError(file.Open(fs, aPath16, EFileRead));
    CleanupClosePushL(file);

    TInt aSize = 0;
    User::LeaveIfError(file.Size(aSize));

    if (aSize == 0)
    {
        CleanupStack::PopAndDestroy(2); // file, fs
        return;
    }

    HBufC8* aFileData = HBufC8::NewL(aSize);
    CleanupStack::PushL(aFileData);
    TPtr8 aDataPtr(aFileData->Des());
    User::LeaveIfError(file.Read(aDataPtr, aSize));

    CleanupStack::Pop(aFileData); // ownership transferred to us

    CleanupStack::PopAndDestroy(2); // file, fs

    // ---- Parse the data ----
    const char* aData = reinterpret_cast<const char*>(aFileData->Ptr());
    TInt aLen = aFileData->Length();
    TInt aPos = 0;

    while (aPos < aLen)
    {
        // Skip leading whitespace and carriage returns
        while (aPos < aLen &&
               (aData[aPos] == ' ' || aData[aPos] == '\t' || aData[aPos] == '\r'))
        {
            ++aPos;
        }

        // Skip empty lines and comment lines
        if (aPos >= aLen || aData[aPos] == '\n' || aData[aPos] == '#')
        {
            while (aPos < aLen && aData[aPos] != '\n')
                ++aPos;
            if (aPos < aLen)
                ++aPos; // skip \n
            continue;
        }

        // Read key (up to '=')
        TInt aKeyStart = aPos;
        while (aPos < aLen && aData[aPos] != '=')
        {
            if (aData[aPos] == '\n')
                break; // malformed line, skip
            ++aPos;
        }

        if (aPos >= aLen || aData[aPos] != '=')
        {
            // Skip to end of malformed line
            while (aPos < aLen && aData[aPos] != '\n')
                ++aPos;
            if (aPos < aLen)
                ++aPos;
            continue;
        }

        TInt aKeyLen = aPos - aKeyStart;

        // Trim trailing whitespace from key
        while (aKeyLen > 0 &&
               (aData[aKeyStart + aKeyLen - 1] == ' ' ||
                aData[aKeyStart + aKeyLen - 1] == '\t'))
        {
            --aKeyLen;
        }

        ++aPos; // skip '='

        // Read value (up to '\n' or end)
        TInt aValStart = aPos;
        while (aPos < aLen && aData[aPos] != '\n')
            ++aPos;

        TInt aValLen = aPos - aValStart;

        // Trim trailing whitespace from value
        while (aValLen > 0 &&
               (aData[aValStart + aValLen - 1] == ' ' ||
                aData[aValStart + aValLen - 1] == '\t' ||
                aData[aValStart + aValLen - 1] == '\r'))
        {
            --aValLen;
        }

        // Extract key and value strings
        char* aKey = new (ELeave) char[aKeyLen + 1];
        Mem::Copy(aKey, aData + aKeyStart, aKeyLen);
        aKey[aKeyLen] = '\0';

        char* aVal = new (ELeave) char[aValLen + 1];
        Mem::Copy(aVal, aData + aValStart, aValLen);
        aVal[aValLen] = '\0';

        // Store or update
        SetString(aKey, aVal);

        delete[] aKey;
        delete[] aVal;

        if (aPos < aLen)
            ++aPos; // skip \n
    }

    delete aFileData;
}

// ===========================================================================
// FindString -- linear search by key (case-sensitive)
// ===========================================================================
const char* TodStringFile::FindString(const char* aKey) const
{
    if (!aKey || !iEntries)
        return NULL;

    for (TInt i = 0; i < iEntryCount; ++i)
    {
        if (iEntries[i].iKey && strcmp(iEntries[i].iKey, aKey) == 0)
            return iEntries[i].iValue;
    }
    return NULL;
}

// ===========================================================================
// SetString -- add or overwrite a key=value entry
// ===========================================================================
void TodStringFile::SetString(const char* aKey, const char* aValue)
{
    if (!aKey || !aValue)
        return;

    // Check for existing key (case-sensitive)
    for (TInt i = 0; i < iEntryCount; ++i)
    {
        if (iEntries[i].iKey && strcmp(iEntries[i].iKey, aKey) == 0)
        {
            // Overwrite existing value
            delete[] iEntries[i].iValue;
            iEntries[i].iValue = DupStrL(aValue);
            return;
        }
    }

    // Ensure capacity
    if (iEntryCount >= iEntryCapacity)
    {
        GrowL(iEntryCount + 1);
    }

    // Add new entry
    iEntries[iEntryCount].iKey   = DupStrL(aKey);
    iEntries[iEntryCount].iValue = DupStrL(aValue);
    ++iEntryCount;
}

} // namespace Sexy
