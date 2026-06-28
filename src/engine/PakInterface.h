/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable PakInterface.h -- simplified PFILE wrappers
 * for the Symbian VFS layer.
 */

#ifndef PAKINTERFACE_H
#define PAKINTERFACE_H

#include <e32base.h>

// Forward declaration
class CPvZVfs;

/**
 * PFILE -- opaque handle for a "currently open" PAK file entry.
 *
 * Mirrors the original PopCap PFILE concept but uses the Symbian
 * CPvZVfs for actual I/O.  mPos tracks the virtual read cursor
 * inside the file entry.
 */
struct PFILE
{
    const char* iName;      // entry name (cached from the entry table)
    TUint32     iOffset;    // absolute offset of data in the PAK file
    TUint32     iSize;      // uncompressed size
    TUint32     iPos;       // current read position within this entry
};

// ---------------------------------------------------------------------------
// Global VFS instance & current-file pointer
// ---------------------------------------------------------------------------
/** The global CPvZVfs instance (owns the PAK file handle). */
extern CPvZVfs* gPak;

/** Points to the currently opened PFILE, or NULL if none. */
extern PFILE*   gPakPtr;

// ---------------------------------------------------------------------------
// Convenience wrappers (used by game code that expects the old PopCap API)
// ---------------------------------------------------------------------------

/**
 * Open a file entry from the PAK by name.  Sets gPakPtr.
 * @param aFileName  Path within the PAK (e.g. "sounds/click.ogg").
 * @return KErrNone on success, KErrNotFound if missing.
 */
TInt PakOpenCur(const char* aFileName);

/**
 * Return the size (in bytes) of the currently open file (gPakPtr).
 * @return Size, or 0 if no file is open.
 */
TInt PakGetFileSize();

/**
 * Read up to aSize bytes from the current file into aBuffer.
 * Advances the internal read cursor.
 * @return Number of bytes actually read, or 0 on error.
 */
TInt PakReadFile(void* aBuffer, TInt aSize);

/**
 * Return the number of bytes remaining in the current file.
 * @return Remaining bytes, or 0 if no file is open.
 */
TInt PakGetAvailable();

#endif // PAKINTERFACE_H
