/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Symbian S60 3rd FP1 VFS layer for PAK archive reading.
 * Replaces the SDL/file-system code from PvZ-Portable.
 */

#ifndef PVZ_VFS_H
#define PVZ_VFS_H

#include <e32base.h>
#include <f32file.h>

/**
 * CPvZVfs -- PAK archive reader for PvZ-Portable on Symbian.
 *
 * Reads the PopCap .pak file format (XOR 0xF7 scrambled).
 * Caches the entry table in memory; file data is read on demand
 * via RFile::Read() with explicit offsets.
 */
class CPvZVfs : public CBase
{
public:
    static CPvZVfs* NewL();
    static CPvZVfs* NewLC();
    ~CPvZVfs();

    /** Load/replace the PAK file. Any previously opened file is closed. */
    void LoadPakL(const TDesC& aPakPath);

    /**
     * Read an entire file entry from the PAK into a descriptor.
     * @param aFileName  Entry name (null-terminated ASCII).
     * @param aBuffer    Descriptor to receive the data. Its MaxLength()
     *                   must be >= the file size.
     * @return ETrue on success, EFalse if the entry was not found or
     *         the buffer is too small.
     */
    TBool ReadFile(const char* aFileName, TPtr8& aBuffer);

    /**
     * Read an entire file entry from the PAK into a heap-allocated buffer.
     * @param aFileName   Entry name (null-terminated ASCII).
     * @param aOutBuffer  On success receives a heap-allocated buffer
     *                    (caller must delete[] it).
     * @param aOutSize    Receives the number of bytes written.
     * @return ETrue on success, EFalse if the entry was not found.
     */
    TBool ReadFile(const char* aFileName, void** aOutBuffer, TInt* aOutSize);

    /** Check whether an entry exists in the PAK. */
    TBool FileExists(const char* aFileName);

    /** Return the uncompressed size of a PAK entry, or KErrNotFound. */
    TInt GetFileSize(const char* aFileName);

    /**
     * Find the index of a PAK entry by name (case-insensitive).
     * @return Index into iEntries, or KErrNotFound.
     */
    TInt GetEntryIndex(const char* aFileName) const;

    /** Return the number of entries in the PAK. */
    TInt GetEntryCount() const { return iEntryCount; }

    /** Return the name of entry at index, or NULL if out of range. */
    const char* GetEntryName(TInt aIndex) const
        { return (aIndex >= 0 && aIndex < iEntryCount) ? iEntries[aIndex].iName : NULL; }

    /** Dump all entry names to a file (for debugging). */
    void DumpEntries(const TDesC& aLogPath);

private:
    CPvZVfs();
    void ConstructL();

    /** Parse the entry table from the open PAK file. */
    void ParseTableL(const TDesC& aPakPath);

    /** XOR every byte in the descriptor with 0xF7. */
    static void XorData(TDes8& aBuffer);

    struct SPakEntry
    {
        char*   iName;      // null-terminated ASCII filename
        TUint32 iOffset;    // absolute offset of data in the PAK file
        TUint32 iSize;      // size of file data
    };

    RFs         iFs;            // file-server session
    RFile       iPakFile;       // open PAK file handle
    SPakEntry*  iEntries;       // cached entry table
    TInt        iEntryCount;    // number of valid entries
    TUint32     iDataStart;     // absolute offset where file data begins

    /** Reusable buffer for header/entry table parsing. */
    HBufC8*     iReadBuffer;
};

#endif // PVZ_VFS_H
