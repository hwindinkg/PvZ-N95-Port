#ifndef __FILTEREFFECT_H__
#define __FILTEREFFECT_H__

#include <e32def.h>

struct FilterEffect
{
    int mFilterEffectType;
    TBool mDead;
    float mProgress;

    FilterEffect() : mFilterEffectType(-1), mDead(EFalse), mProgress(0) {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
};

#endif
