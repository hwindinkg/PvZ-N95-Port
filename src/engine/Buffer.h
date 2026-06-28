/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Ported from PvZ-Portable SexyAppFramework/misc/Buffer.h.
 * Simple binary data buffer -- no STL, no exceptions, C++03.
 *
 * Byte-level read/write buffer with automatic growth.
 * Lives in namespace Sexy with member naming mData/mSize/mCapacity.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <e32base.h>

namespace Sexy
{

class Buffer
{
public:
    Buffer();
    ~Buffer();

    /** Allocate initial buffer and prepare for read/write. */
    void OpenBuffer();

    /** Free the internal buffer and reset all state. */
    void Close();

    // -- Write methods ------------------------------------------------
    void WriteByte(TUint8 theByte);
    void WriteShort(TInt16 theShort);
    void WriteLong(TInt32 theLong);
    void WriteFloat(float theFloat);
    void WriteString(const char* theString);
    void WriteLine(const char* theString);
    void WriteBytes(const TUint8* theByte, TInt theCount);

    // -- Read methods -------------------------------------------------
    TUint8 ReadByte();
    TInt16 ReadShort();
    TInt32 ReadLong();
    float  ReadFloat();
    void   ReadString(char* aOut, TInt aMaxLen);
    void   ReadBytes(TUint8* aData, TInt aLen);

    // -- Data access --------------------------------------------------
    TUint8* GetData() const;
    TInt    GetSize() const;

    // -- State queries ------------------------------------------------
    TBool AtEnd() const;

    // -- Write from raw pointer ---------------------------------------
    void SetData(const TUint8* aPtr, TInt aCount);

private:
    /** Grow the internal buffer to at least newCapacity bytes. */
    void GrowBuffer(TInt newCapacity);

    TUint8* mData;       // heap-allocated data buffer
    TInt    mSize;       // number of meaningful bytes written
    TInt    mCapacity;   // allocated capacity in bytes
    TInt    mReadPos;    // current read cursor (byte offset)
    TInt    mWritePos;   // current write cursor (byte offset)
};

} // namespace Sexy

#endif // BUFFER_H
