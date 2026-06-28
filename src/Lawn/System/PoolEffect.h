#ifndef __POOLEFFECT_H__
#define __POOLEFFECT_H__
#include <e32base.h>
class PoolEffect
{
public:
    PoolEffect() {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    void PoolEffectInitialize() {}
    void PoolEffectDispose() {}
};
#endif
