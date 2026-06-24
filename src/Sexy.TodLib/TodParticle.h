#ifndef __TODPARTICLE_H__
#define __TODPARTICLE_H__

#include <e32def.h>
#include "DataArray.h"

class TodParticleSystem
{
public:
    float mX, mY;
    int mParticleType;
    TBool mDead;
    int mRenderOrder;
    int mEffectType;
    TBool mDontUpdate;
    float mParticleRotation;

    TodParticleSystem() : mX(0), mY(0), mParticleType(-1), mDead(EFalse), mRenderOrder(0), mEffectType(0), mDontUpdate(EFalse), mParticleRotation(0) {}

    void ParticleSystemDelete() { mDead = ETrue; }
    void ParticleSystemDie() { mDead = ETrue; }
    void Update() {}
    void Draw(class Sexy::Graphics* g) {}
    void SetPosition(float x, float y) { mX = x; mY = y; }
    void SystemMove(float x, float y) { mX = x; mY = y; }
    void OverrideColor(int idx, const class Sexy::Color& color) { (void)idx; (void)color; }
    void OverrideScale(int idx, float scale) { (void)idx; (void)scale; }
    void OverrideExtraAdditiveDraw(int idx, bool enabled) { (void)idx; (void)enabled; }
};

class ReanimatorCache; // forward

class EffectSystem
{
public:
    DataArray<TodParticleSystem> mParticleSystems;
    DataArray<class Reanimation> mReanimations;
    ReanimatorCache* mReanimationHolder;
    class ParticleHolder* mParticleHolder;

    void EffectSystemInitialize() { mParticleSystems.DataArrayInitialize(128, "Particles"); }
    void EffectSystemDispose() { mParticleSystems.DataArrayDispose(); }
    void ProcessDeleteQueue()
    {
        TodParticleSystem* aParticle = NULL;
        while (mParticleSystems.IterateNext(aParticle))
        {
            if (aParticle->mDead)
            {
                TodParticleSystem* aDead = aParticle;
                aParticle = NULL;
                mParticleSystems.DataArrayFree(aDead);
            }
        }
    }
    TodParticleSystem* AllocParticleSystem(int type, float x, float y)
    {
        TodParticleSystem* p = mParticleSystems.DataArrayAlloc();
        p->mParticleType = type;
        p->SetPosition(x, y);
        return p;
    }
    void Update() {}
    void UpdateAll() {}
    void EffectSystemFreeAll() { EffectSystemDispose(); }
};

// ParticleHolder stub class
class ParticleHolder
{
public:
    DataArray<TodParticleSystem> mParticleSystems;
    ParticleHolder() { mParticleSystems.DataArrayInitialize(128, "Particles"); }
    TodParticleSystem* AllocParticleSystem(float x, float y, int renderOrder, int effectType)
    {
        TodParticleSystem* p = mParticleSystems.DataArrayAlloc();
        p->mX = x; p->mY = y; p->mRenderOrder = renderOrder; p->mEffectType = effectType;
        return p;
    }
};

#endif // __TODPARTICLE_H__