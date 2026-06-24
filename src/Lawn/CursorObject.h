#ifndef __CURSOROBJECT_H__
#define __CURSOROBJECT_H__

#include <e32def.h>
#include "../engine/Common.h"

class CursorObject
{
public:
    int mCursorType;
    int mSeedType;
    int mX, mY;
    TBool mVisible;
    TBool mDragging;
    int mType;
    int mImitaterType;
    int mCoinID;
    int mSeedBankIndex;
    int mGlovePlantID;
    int mCobCannonPlantID;
    int mReanimCursorID;

    CursorObject() : mCursorType(-1), mSeedType(-1), mX(0), mY(0), mVisible(EFalse), mDragging(EFalse), mType(-1), mImitaterType(-1), mCoinID(-1), mSeedBankIndex(-1), mGlovePlantID(-1), mCobCannonPlantID(-1), mReanimCursorID(-1) {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    void Update() {}
};

#endif
