#ifndef __SEEDBANK_H__
#define __SEEDBANK_H__

#include <e32def.h>
#include "../engine/Widget.h"
#include "SeedPacket.h"

class SeedBank : public Sexy::Widget
{
public:
    SeedBank() : mNumPackets(0) {}
    void Update() {}
    void Draw(Sexy::Graphics* g) { (void)g; }
    void UpdateWidth() {}
    void Activate() {}
    void Deactivate() {}
    void AddSeed(int theSeedType) { (void)theSeedType; }

    int mNumPackets;
    SeedPacket mSeedPackets[48];
};
#endif
