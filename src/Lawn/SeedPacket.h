#ifndef __SEEDPACKET_H__
#define __SEEDPACKET_H__

#include <e32def.h>
#include "../engine/Widget.h"

class SeedPacket : public Sexy::Widget
{
public:
    int mIndex;
    int mSeedType;
    int mPacketType;
    int mImitaterType;
    TBool mActive;
    TBool mRefreshing;
    float mTimer;

    SeedPacket() : mIndex(0), mSeedType(-1), mPacketType(-1), mImitaterType(-1), mActive(EFalse), mRefreshing(EFalse), mTimer(0) {}
    void Draw(Sexy::Graphics* g) { (void)g; }
    void Update() {}
    void Activate() {}
    void Refresh() {}
    void SetPacketType(int type) { mPacketType = type; }
    void SetPacketType(int type, int imitaterType) { mPacketType = type; mImitaterType = imitaterType; }
    void SetActivate(bool) {}
    void FlashIfReady() {}
};

#endif
