/*
 * ReanimatorRuntime.h — full Reanimation runtime (port of upstream
 * Reanimator.cpp + EffectSystem.cpp).
 *
 * [Session-13] This is a 1:1 port of the upstream Reanimation system,
 * adapted to the port's infrastructure (ReanimLoader.h's ReanimDefinition,
 * Graphics, MemoryImage). It implements:
 *   - Reanimation class (plays an animation, interpolates transforms)
 *   - ReanimationHolder (EffectSystem — manages active Reanimations)
 *   - DrawTrack / DrawRenderGroup / Draw (render with render groups)
 *   - GetCurrentTransform / GetTransformAtTime / GetFrameTime
 *   - PlayReanim / GetFramesForLayer / SetFramesForLayer
 *   - FindTrackIndex / TrackExists
 *   - Update (advances mAnimTime based on mAnimRate)
 *
 * The key insight from upstream:
 * - Each .reanim has N tracks, ALL sharing a timeline of K transforms (e.g. 706).
 * - "anim_open", "anim_idle" etc. are MARKER tracks — their non-blank
 *   transform window defines [mFrameStart, mFrameCount] for that action.
 * - mAnimTime ∈ [0,1] walks through the window; mAnimRate (fps) sets speed.
 * - GetFrameTime maps mAnimTime → transform array indices.
 * - DrawTrack renders each visible track's image at the interpolated position.
 * - Render groups control which tracks are drawn (BG=group 1, buttons=group 0).
 */
#ifndef __REANIMATORRUNTIME_H__
#define __REANIMATORRUNTIME_H__

#include <e32def.h>
#include "ReanimLoader.h"
#include "../ConstEnums.h"  // ReanimLoopType enum (REANIM_LOOP, REANIM_PLAY_ONCE, etc.)

namespace Sexy { class Graphics; class Image; }

// ===========================================================================
// ReanimatorFrameTime — current frame position (from upstream Reanimator.h)
// ===========================================================================
struct ReanimatorFrameTime
{
    float mFraction;           // lerp weight between before/after frames
    int   mAnimFrameBeforeInt; // index of the "before" transform
    int   mAnimFrameAfterInt;  // index of the "after" transform
};

// ===========================================================================
// ReanimatorTrackInstance — per-track runtime state (from upstream)
// ===========================================================================
struct ReanimatorTrackInstance
{
    int   mRenderGroup;        // which render group this track belongs to
    float mShakeX;
    float mShakeY;
    Sexy::Image* mImageOverride;
    bool  mIgnoreClipRect;

    ReanimatorTrackInstance()
        : mRenderGroup(0), mShakeX(0), mShakeY(0),
          mImageOverride(NULL), mIgnoreClipRect(false) {}
};

// ===========================================================================
// Reanimation — a playing animation instance (port of upstream Reanimation)
// ===========================================================================
class Reanimation
{
public:
    ReanimDefinition*       mDefinition;
    float                   mAnimTime;      // [0,1] position in current loop
    float                   mAnimRate;      // playback speed (fps)
    ReanimLoopType          mLoopType;
    bool                    mDead;
    int                     mFrameStart;    // first transform index for this action
    int                     mFrameCount;    // number of transforms in this action
    int                     mFrameBasePose;
    float                   mX, mY;         // position offset (reanim space)
    float                   mScaleX, mScaleY;
    int                     mLoopCount;
    int                     mRenderOrder;
    float                   mAlpha;         // global alpha multiplier
    ReanimatorTrackInstance* mTrackInstances; // per-track runtime state
    float                   mLastFrameTime;

    Reanimation();
    ~Reanimation();

    // -- Setup ----------------------------------------------------------
    void  ReanimationInitialize(float x, float y, ReanimDefinition* def);
    void  ReanimationDie() { mDead = true; }

    // -- Playback -------------------------------------------------------
    void  PlayReanim(const char* trackName, ReanimLoopType loopType,
                     int blendTime, float animRate);
    void  SetFramesForLayer(const char* trackName);
    void  GetFramesForLayer(const char* trackName, int& frameStart, int& frameCount);
    int   FindTrackIndex(const char* trackName);
    bool  TrackExists(const char* trackName);

    // -- Update ---------------------------------------------------------
    void  Update();

    // -- Transform ------------------------------------------------------
    void  GetFrameTime(ReanimatorFrameTime* ft);
    void  GetTransformAtTime(int trackIndex, ReanimTransform* t, ReanimatorFrameTime* ft);
    void  GetCurrentTransform(int trackIndex, ReanimTransform* t);

    // -- Rendering ------------------------------------------------------
    void  Draw(Sexy::Graphics* g);
    void  DrawRenderGroup(Sexy::Graphics* g, int renderGroup);
    bool  DrawTrack(Sexy::Graphics* g, int trackIndex);

    // -- Utility --------------------------------------------------------
    void  AssignRenderGroupToTrack(const char* trackName, int group);
    void  AssignRenderGroupToPrefix(const char* prefix, int group);
    void  SetPosition(float x, float y) { mX = x; mY = y; }
    void  SetImageOverride(const char* trackName, Sexy::Image* img);
};

// ===========================================================================
// ReanimationHolder — manages active Reanimations (port of EffectSystem)
// ===========================================================================
class ReanimationHolder
{
public:
    static const int MAX_REANIMATIONS = 64;

    ReanimationHolder();
    ~ReanimationHolder();

    Reanimation* AllocReanimation(float x, float y, int renderOrder,
                                  ReanimDefinition* def);
    void         RemoveReanimation(Reanimation* r);
    Reanimation* GetReanimation(int idx);
    void         UpdateAll();
    void         DrawAll(Sexy::Graphics* g);
    void         DrawAllRenderGroup(Sexy::Graphics* g, int renderGroup);
    void         Clear();

private:
    Reanimation* mReanimations[MAX_REANIMATIONS];
    int          mCount;
};

// ===========================================================================
// EffectSystem — the global reanimation + particle manager
// ===========================================================================
class EffectSystem
{
public:
    EffectSystem();
    ~EffectSystem();

    ReanimationHolder* mReanimationHolder;

    void EffectSystemInitialize();
    void EffectSystemDispose();
    void Update();
    void Draw(Sexy::Graphics* g);
};

extern EffectSystem* gEffectSystem;

#endif // __REANIMATORRUNTIME_H__
