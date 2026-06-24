/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * Ported from PvZ-Portable Sexy.TodLib/DataArray.h for Symbian S60 3rd FP1.
 * Typed data array with slot-based allocation and ID tracking.
 */

#ifndef __DATAARRAY_H__
#define __DATAARRAY_H__

#include <string.h>
#include "TodDebug.h"
#include "TodCommon.h"

enum
{
	DATA_ARRAY_INDEX_MASK = 65535,
	DATA_ARRAY_KEY_MASK = -65536,
	DATA_ARRAY_KEY_SHIFT = 16,
	DATA_ARRAY_MAX_SIZE = 65536,
	DATA_ARRAY_KEY_FIRST = 1
};

template <typename T> class DataArray
{
public:
	class DataArrayItem
	{
	public:
		T					mItem;
		unsigned int		mID;
	};
	typedef DataArrayItem ItemType;


public:
	DataArrayItem*			mBlock;
	unsigned int			mMaxUsedCount;
	unsigned int			mMaxSize;
	unsigned int			mFreeListHead;
	unsigned int			mSize;
	unsigned int			mNextKey;
	const char*				mName;

public:
	DataArray()
	{
		mBlock = NULL;
		mMaxUsedCount = 0U;
		mMaxSize = 0U;
		mFreeListHead = 0U;
		mSize = 0U;
		mNextKey = 1U;
		mName = NULL;
	}

	~DataArray()
	{
		DataArrayDispose();
	}

	void DataArrayInitialize(unsigned int theMaxSize, const char* theName)
	{
		TOD_ASSERT(mBlock == NULL);
		mBlock = static_cast<DataArrayItem*>(operator new(sizeof(ItemType) * theMaxSize));
		mMaxSize = theMaxSize;
		mNextKey = 1001U;
		mName = theName;
	}

	void DataArrayDispose()
	{
		if (mBlock != NULL)
		{
			DataArrayFreeAll();
			operator delete(mBlock);
			mBlock = NULL;
			mMaxUsedCount = 0U;
			mMaxSize = 0U;
			mFreeListHead = 0U;
			mSize = 0U;
			mName = NULL;
		}
	}

	void DataArrayFree(T* theItem)
	{
		DataArrayItem* aItem = reinterpret_cast<DataArrayItem*>(theItem);
		TOD_ASSERT(DataArrayGet(aItem->mID) == theItem, "Failed: DataArrayFree(0x%x) in %s", theItem, mName);
		theItem->~T();
		unsigned int anId = aItem->mID & DATA_ARRAY_INDEX_MASK;
		aItem->mID = mFreeListHead;
		mFreeListHead = anId;
		mSize--;
	}

	T* DataArrayFirst(unsigned int& theID) { theID = 0; if (mMaxUsedCount == 0) return NULL; theID = 1; return &mBlock[0].mItem; }
	T* DataArrayNext(unsigned int& theID) { unsigned int idx = theID; if (idx >= mMaxUsedCount) return NULL; theID = idx + 1; return &mBlock[idx].mItem; }
	T* DataArrayFirst() { return &mBlock[0].mItem; }
	T* DataArrayNext(T*& theItem) { (void)theItem; return NULL; }

	void DataArrayFreeAll()
	{
		T* aItem = NULL;
		while (IterateNext(aItem))
			DataArrayFree(aItem);

		mFreeListHead = 0U;
		mMaxUsedCount = 0U;
	}

	inline unsigned int DataArrayGetID(T* theItem)
	{
		DataArrayItem* aItem = reinterpret_cast<DataArrayItem*>(theItem);
		TOD_ASSERT(DataArrayGet(aItem->mID) == theItem, "Failed: DataArrayGetID(0x%x) for %s", theItem, mName);
		return aItem->mID;
	}

	bool IterateNext(T*& theItem)
	{
		ItemType * aItem = reinterpret_cast<ItemType *>(theItem);
		if (aItem == NULL)
			aItem = &mBlock[0];
		else
			aItem++;

		ItemType * aLast = &mBlock[mMaxUsedCount];
		while (aItem < aLast)
		{
			if (aItem->mID & DATA_ARRAY_KEY_MASK)
			{
				theItem = reinterpret_cast<T*>(aItem);
				return true;
			}
			aItem++;
		}
		return false;
	}

	T* DataArrayAlloc()
	{
		TOD_ASSERT(mSize < mMaxSize, "Data array full: %s", mName);
		TOD_ASSERT(mFreeListHead <= mMaxUsedCount, "DataArrayAlloc error in %s", mName);
		unsigned int aNext = mMaxUsedCount;
		if (mFreeListHead == mMaxUsedCount)
			mFreeListHead = ++mMaxUsedCount;
		else
		{
			aNext = mFreeListHead;
			mFreeListHead = mBlock[mFreeListHead].mID;
		}

		ItemType * aNewItem = &mBlock[aNext];
		memset(aNewItem, 0, sizeof(ItemType));
		aNewItem->mID = (mNextKey++ << DATA_ARRAY_KEY_SHIFT) | aNext;
		if (mNextKey == DATA_ARRAY_MAX_SIZE) mNextKey = 1;
		mSize++;

		new (aNewItem)T();
		return reinterpret_cast<T*>(aNewItem);
	}

	T& operator[](unsigned int theIndex)
	{
		TOD_ASSERT(theIndex < mMaxUsedCount, "DataArray operator[] out of bounds: %s", mName);
		return mBlock[theIndex].mItem;
	}

	const T& operator[](unsigned int theIndex) const
	{
		TOD_ASSERT(theIndex < mMaxUsedCount, "DataArray operator[] out of bounds: %s", mName);
		return mBlock[theIndex].mItem;
	}

	T* DataArrayTryToGet(unsigned int theId)
	{
		if (!theId || (theId & DATA_ARRAY_INDEX_MASK) >= mMaxSize)
			return NULL;

		DataArrayItem* aBlock = &mBlock[theId & DATA_ARRAY_INDEX_MASK];
		return (aBlock->mID == theId) ? &aBlock->mItem : NULL;
	}

	T* DataArrayGet(unsigned int theId)
	{
		TOD_ASSERT(DataArrayTryToGet(theId) != NULL, "Failed: DataArrayGet(0x%x) for %s", theId, mName);
		return &mBlock[static_cast<short>(theId)].mItem;
	}
};

#endif
