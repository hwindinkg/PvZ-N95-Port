/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * Byte-level binary buffer matching the simplified Buffer API.
 * Lives in namespace Sexy.
 */

#include "Buffer.h"
#include "Common.h"
#include <string.h>  // strlen, Mem::Copy, Mem::FillZ

static const TInt KBufferInitialSize = 256;

namespace Sexy {

// ===========================================================================
// Construction / destruction
// ===========================================================================
Buffer::Buffer()
    : mData(NULL)
    , mSize(0)
    , mCapacity(0)
    , mReadPos(0)
    , mWritePos(0)
{
}

Buffer::~Buffer()
{
    Close();
}

void Buffer::OpenBuffer()
{
    Close();
    mCapacity = KBufferInitialSize;
    mData     = new TUint8[mCapacity];
    if (mData)
        Mem::FillZ(mData, mCapacity);
}

void Buffer::Close()
{
    delete[] mData;
    mData     = NULL;
    mSize     = 0;
    mCapacity = 0;
    mReadPos  = 0;
    mWritePos = 0;
}

// ===========================================================================
// GrowBuffer  --  ensure capacity >= newCapacity bytes
// ===========================================================================
void Buffer::GrowBuffer(TInt newCapacity)
{
    if (newCapacity <= mCapacity)
        return;

    TInt aNewSize = mCapacity ? mCapacity * 2 : KBufferInitialSize;
    while (aNewSize < newCapacity)
        aNewSize <<= 1;

    TUint8* aNewData = new TUint8[aNewSize];
    if (!aNewData)
        return;

    if (mData && mSize > 0)
        Mem::Copy(aNewData, mData, mSize);
    Mem::FillZ(aNewData + mSize, aNewSize - mSize);

    delete[] mData;
    mData     = aNewData;
    mCapacity = aNewSize;
}

// ===========================================================================
// WriteByte
// ===========================================================================
void Buffer::WriteByte(TUint8 theByte)
{
    if (mWritePos >= mCapacity)
        GrowBuffer(mWritePos + 1);
    if (mWritePos < mCapacity)
    {
        mData[mWritePos] = theByte;
        ++mWritePos;
        if (mWritePos > mSize)
            mSize = mWritePos;
    }
}

// ===========================================================================
// WriteShort  (16-bit LE)
// ===========================================================================
void Buffer::WriteShort(TInt16 theShort)
{
    WriteByte(static_cast<TUint8>(theShort & 0xFF));
    WriteByte(static_cast<TUint8>((theShort >> 8) & 0xFF));
}

// ===========================================================================
// WriteLong  (32-bit LE)
// ===========================================================================
void Buffer::WriteLong(TInt32 theLong)
{
    WriteByte(static_cast<TUint8>(theLong & 0xFF));
    WriteByte(static_cast<TUint8>((theLong >> 8) & 0xFF));
    WriteByte(static_cast<TUint8>((theLong >> 16) & 0xFF));
    WriteByte(static_cast<TUint8>((theLong >> 24) & 0xFF));
}

// ===========================================================================
// WriteFloat  (IEEE 754 32-bit, stored as LE int32)
// ===========================================================================
void Buffer::WriteFloat(float theFloat)
{
    TUint32 aBits;
    Mem::Copy(&aBits, &theFloat, sizeof(aBits));
    WriteLong(static_cast<TInt32>(aBits));
}

// ===========================================================================
// WriteString  --  length-prefixed (short LE) + string data
// ===========================================================================
void Buffer::WriteString(const char* theString)
{
    TInt aLen = theString ? static_cast<TInt>(strlen(theString)) : 0;
    WriteShort(static_cast<TInt16>(aLen));
    WriteBytes(reinterpret_cast<const TUint8*>(theString), aLen);
}

// ===========================================================================
// WriteLine  --  raw string + "\r\n"
// ===========================================================================
void Buffer::WriteLine(const char* theString)
{
    TInt aLen = theString ? static_cast<TInt>(strlen(theString)) : 0;
    WriteBytes(reinterpret_cast<const TUint8*>(theString), aLen);
    WriteByte(static_cast<TUint8>('\r'));
    WriteByte(static_cast<TUint8>('\n'));
}

// ===========================================================================
// WriteBytes
// ===========================================================================
void Buffer::WriteBytes(const TUint8* theByte, TInt theCount)
{
    for (TInt i = 0; i < theCount; ++i)
        WriteByte(theByte[i]);
}

// ===========================================================================
// ReadByte
// ===========================================================================
TUint8 Buffer::ReadByte()
{
    if (mReadPos >= mSize)
        return 0;  // underflow
    return mData[mReadPos++];
}

// ===========================================================================
// ReadShort  (16-bit LE)
// ===========================================================================
TInt16 Buffer::ReadShort()
{
    TUint8 lo = ReadByte();
    TUint8 hi = ReadByte();
    return static_cast<TInt16>(lo | (static_cast<TInt16>(hi) << 8));
}

// ===========================================================================
// ReadLong  (32-bit LE)
// ===========================================================================
TInt32 Buffer::ReadLong()
{
    TUint8 b0 = ReadByte();
    TUint8 b1 = ReadByte();
    TUint8 b2 = ReadByte();
    TUint8 b3 = ReadByte();
    return static_cast<TInt32>(b0)
         | (static_cast<TInt32>(b1) << 8)
         | (static_cast<TInt32>(b2) << 16)
         | (static_cast<TInt32>(b3) << 24);
}

// ===========================================================================
// ReadFloat  (IEEE 754 32-bit)
// ===========================================================================
float Buffer::ReadFloat()
{
    TUint32 aBits = static_cast<TUint32>(ReadLong());
    float aFloat = 0.0f;
    Mem::Copy(&aFloat, &aBits, sizeof(aFloat));
    return aFloat;
}

// ===========================================================================
// ReadString  --  length-prefixed (short LE)
// ===========================================================================
void Buffer::ReadString(char* aOut, TInt aMaxLen)
{
    if (!aOut || aMaxLen <= 0)
        return;

    TInt aLen = ReadShort();
    if (aLen > aMaxLen - 1)
        aLen = aMaxLen - 1;

    for (TInt i = 0; i < aLen; ++i)
        aOut[i] = static_cast<char>(ReadByte());
    aOut[aLen] = '\0';
}

// ===========================================================================
// ReadBytes
// ===========================================================================
void Buffer::ReadBytes(TUint8* aData, TInt aLen)
{
    for (TInt i = 0; i < aLen; ++i)
        aData[i] = ReadByte();
}

// ===========================================================================
// GetData  --  return pointer to internal buffer
// ===========================================================================
TUint8* Buffer::GetData() const
{
    return mData;
}

// ===========================================================================
// GetSize  --  number of meaningful bytes
// ===========================================================================
TInt Buffer::GetSize() const
{
    return mSize;
}

// ===========================================================================
// AtEnd  --  have we read all bytes?
// ===========================================================================
TBool Buffer::AtEnd() const
{
    return mReadPos >= mSize;
}

// ===========================================================================
// SetData  --  copy external data into buffer
// ===========================================================================
void Buffer::SetData(const TUint8* aPtr, TInt aCount)
{
    Close();
    if (aCount <= 0)
        return;

    mData = new TUint8[aCount];
    if (!mData)
        return;

    Mem::Copy(mData, aPtr, aCount);
    mSize     = aCount;
    mCapacity = aCount;
    mReadPos  = 0;
    mWritePos = aCount;
}

} // namespace Sexy
