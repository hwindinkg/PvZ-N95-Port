/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Symbian S60 3rd FP1 string table for loading LawnStrings.txt
 * (key=value pairs).
 */

#ifndef TOD_STRING_FILE_H
#define TOD_STRING_FILE_H

#include <e32base.h>

namespace Sexy {

/**
 * TodStringFile -- simple string table that loads a text file
 * containing key=value pairs (one per line).
 *
 * Empty lines and lines beginning with '#' are treated as comments
 * and skipped. Only unique keys are stored; duplicate keys overwrite
 * earlier values.
 */
class TodStringFile : public CBase
{
public:
    static TodStringFile* NewL();
    static TodStringFile* NewLC();
    ~TodStringFile();

    /**
     * Load key=value pairs from a text file.
     * @param aPath  Filesystem path (ASCII, converted to TDesC internally).
     */
    void LoadFromFile(const char* aPath);

    /**
     * Look up a string by key.
     * @return The value string, or NULL if the key is not found.
     */
    const char* FindString(const char* aKey) const;

    /**
     * Set or overwrite a string entry.
     * Both key and value are copied internally.
     */
    void SetString(const char* aKey, const char* aValue);

private:
    TodStringFile();
    void ConstructL();

    /** Grow (or allocate) the entry array to hold at least aMinCap entries. */
    void GrowL(TInt aMinCap);

    struct SStringEntry
    {
        char* iKey;
        char* iValue;
    };

    SStringEntry* iEntries;
    TInt          iEntryCount;
    TInt          iEntryCapacity;
};

} // namespace Sexy

#endif // TOD_STRING_FILE_H
