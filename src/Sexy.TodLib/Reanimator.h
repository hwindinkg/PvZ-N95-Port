#ifndef __REANIMATOR_H__
#define __REANIMATOR_H__

#include <e32def.h>
#include "DataArray.h"
#include "../ConstEnums.h"
#include "../engine/Color.h"

// Forward declarations
namespace Sexy { class Image; class Graphics; }
struct ReanimatorTransform;
struct ReanimatorTrackInstance;
struct ReanimatorDefinition;
class Reanimation;
inline Reanimation* FindReanimAttachment(AttachmentID theAttachmentID) { (void)theAttachmentID; return NULL; }

// ---------------------------------------------------------------------------
// ReanimatorTransform -- per-frame bone transform
// ---------------------------------------------------------------------------
struct ReanimatorTransform
{
    float mTransX;
    float mTransY;
    float mSkewX;
    float mSkewY;
    float mScaleX;
    float mScaleY;
    float mAlpha;
    int   mFrame;
    ReanimatorTransform()
        : mTransX(0), mTransY(0), mSkewX(0), mSkewY(0),
          mScaleX(1), mScaleY(1), mAlpha(255), mFrame(0) {}
};

// ---------------------------------------------------------------------------
// ReanimatorTrackInstance -- per-track runtime state
// ---------------------------------------------------------------------------
struct ReanimatorTrackDefinition
{
    const char* mName;
    ReanimatorTrackDefinition() : mName(NULL) {}
};

struct ReanimatorTrackInstance
{
    AttachmentID mAttachmentID;
    float        mShakeX;
    float        mShakeY;
    float        mBlendWeight;
    float        mAnimTime;
    int          mTrackColor;
    Sexy::Image* mImageOverride;
    bool         mIgnoreClipRect;

    ReanimatorTrackInstance()
        : mAttachmentID(ATTACHMENTID_NULL), mShakeX(0), mShakeY(0),
          mBlendWeight(1.0f), mAnimTime(0), mTrackColor(0),
          mImageOverride(NULL), mIgnoreClipRect(false) {}
};

// ---------------------------------------------------------------------------
// ReanimatorDefinition -- placeholder for reanimation definition data
// ---------------------------------------------------------------------------
#define MAX_REANIM_TRACKS 64

struct ReanimatorTrackArray
{
    int count;
    ReanimatorTrackDefinition tracks[MAX_REANIM_TRACKS];
    ReanimatorTrackArray() : count(0) {}
};

struct ReanimatorDefinition
{
    int mReanimType;
    int mNumTracks;
    float mFPS;
    ReanimatorTrackArray mTracks;
    ReanimatorDefinition() : mReanimType(-1), mNumTracks(0), mFPS(12.0f) {}
};

class Reanimation
{
public:
    int mReanimationType;
    float mAnimTime;
    float mAnimRate;
    float mFrameBase;
    float mFrameCount;
    TBool mLooping;
    TBool mDead;
    int mLoopType;
    TBool mIsAttachment;
    float mX;
    float mY;
    float mScale;
    int mRenderOrder;
    Sexy::Color mColorOverride;
    int mFilterEffect;

    // Extended members used by plant.cpp
    int mFrameBasePose;
    int mLoopCount;
    ReanimatorDefinition* mDefinition;
    Sexy::Color mExtraAdditiveColor;
    TBool mEnableExtraAdditiveDraw;
    Sexy::Color mExtraOverlayColor;
    TBool mEnableExtraOverlayDraw;
    float mShakeOverrideX;
    float mShakeOverrideY;
    TBool mShakeOverrideActive;
    Sexy::Image* mImageOverride;  // for GetImageOverride/SetImageOverride
    class Sexy::SexyMatrix3* mOverlayMatrix;
    ReanimatorTrackInstance* mTrackInstances; // array of per-track state

    // Reanimation die state
    TBool mReanimationDie;
    TBool mReanimationDieWasGiven;

    Reanimation()
        : mReanimationType(-1), mAnimTime(0), mAnimRate(0),
          mFrameBase(0), mFrameCount(0), mLooping(EFalse), mDead(EFalse),
          mLoopType(0), mIsAttachment(EFalse), mX(0), mY(0), mScale(1),
          mRenderOrder(0), mFilterEffect(0),
          mFrameBasePose(0), mLoopCount(0), mDefinition(NULL),
          mEnableExtraAdditiveDraw(EFalse), mEnableExtraOverlayDraw(EFalse),
          mShakeOverrideX(0), mShakeOverrideY(0), mShakeOverrideActive(EFalse),
          mImageOverride(NULL), mOverlayMatrix(NULL), mTrackInstances(NULL),
          mReanimationDie(EFalse), mReanimationDieWasGiven(EFalse) {}

    void ReanimationInitializeType(float x, float y, int theType) { mX = x; mY = y; mReanimationType = theType; }
    void ReanimationDelete() { mDead = ETrue; }
    TBool ReanimationLoaded() { return ETrue; }
    void SetAnimType(int type) { mReanimationType = type; }
    void SetBasePoseFromAnim(const char*) {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    void SetPosition(float x, float y) { mX = x; mY = y; }
    void SetScale(float s) { mScale = s; }
    void SetAnimRate(float rate) { mAnimRate = rate; }
    void PlayAnimation() { mLooping = ETrue; }

    // --- Stub methods for plant.cpp compatibility ---
    bool TrackExists(const char* theTrackName) { (void)theTrackName; return true; }
    void SetFramesForLayer(const char* theLayerName) { (void)theLayerName; }
    void AttachToAnotherReanimation(Reanimation* theAttachReanim, const char* theTrackName) { (void)theAttachReanim; (void)theTrackName; }
    void SetTruncateDisappearingFrames() {}
    void SetTruncateDisappearingFrames(const char* theTrackName, bool val) { (void)theTrackName; (void)val; }
    void AssignRenderGroupToTrack(const char* theTrackName, int theRenderGroup) { (void)theTrackName; (void)theRenderGroup; }
    void AssignRenderGroupToPrefix(const char* thePrefix, int theRenderGroup) { (void)thePrefix; (void)theRenderGroup; }
    void StartBlend(float theBlendTime) { (void)theBlendTime; }
    void ShowOnlyTrack(const char* theTrackName) { (void)theTrackName; }
    void OverrideScale(float theScaleX, float theScaleY) { (void)theScaleX; (void)theScaleY; }
    bool ShouldTriggerTimedEvent(float theEventTime) { (void)theEventTime; return false; }
    ReanimatorTrackInstance* GetTrackInstanceByName(const char* theTrackName) { (void)theTrackName; return NULL; }
    int FindTrackIndex(const char* theTrackName) { (void)theTrackName; return 0; }
    bool GetCurrentTransform(ReanimatorTransform& theTransform) { (void)theTransform; return true; }
    bool GetCurrentTransform(int theTrackIndex, ReanimatorTransform* theTransform) { (void)theTrackIndex; (void)theTransform; return true; }
    void DrawRenderGroup(int theRenderGroup) { (void)theRenderGroup; }
    void DrawRenderGroup(Sexy::Graphics* g, int theRenderGroup) { (void)g; (void)theRenderGroup; }
    bool IsAnimPlaying(const char* theTrackName) { (void)theTrackName; return true; }
    float GetTrackVelocity(const char* theTrackName) { (void)theTrackName; return 0.0f; }
    void SetShakeOverride(float theShakeX, float theShakeY) { mShakeOverrideX = theShakeX; mShakeOverrideY = theShakeY; mShakeOverrideActive = ETrue; }
    void SetShakeOverride(const char* theTrackName, float theIntensity) { (void)theTrackName; (void)theIntensity; }
    Sexy::Image* GetImageOverride(const char* theTrackName) { (void)theTrackName; return NULL; }
    void SetImageOverride(const char* theTrackName, Sexy::Image* theImage) { (void)theTrackName; (void)theImage; }
    void PropogateColorToAttachments() {}
    void OverrideExtraAdditiveDraw(float, const Sexy::Color&) {}
    void OverrideExtraAdditiveDraw(const Sexy::Color&) {}
    void ReanimationDie() { mDead = ETrue; mReanimationDie = ETrue; }
    void SetGroupScale(const char*, float) {}
    void ShowZombieWalking(bool) {}
    bool PlayReanim(const char*, float, int, float) { return true; }
    void UpdateRememberedFrame(const char*, bool) {}
    void AddReanimation(float x, float y, int theType) { (void)x; (void)y; (void)theType; }
};

class ReanimatorCache
{
public:
    DataArray<Reanimation> mReanimations;
    void ReanimatorCacheInitialize() { mReanimations.DataArrayInitialize(256, "ReanimCache"); }
    void ReanimatorCacheDispose() { mReanimations.DataArrayDispose(); }
    Reanimation* AllocReanimation(int type, float x, float y, int renderOrder)
    {
        Reanimation* r = mReanimations.DataArrayAlloc();
        r->SetAnimType(type);
        r->SetPosition(x, y);
        r->mRenderOrder = renderOrder;
        return r;
    }
    Reanimation* AllocReanimation(float x, float y, int renderOrder, int type)
    {
        return AllocReanimation(type, x, y, renderOrder);
    }
    void RemoveReanimation(Reanimation* r) { if (r) mReanimations.DataArrayFree(r); }
    Reanimation* GetReanimation(int idx)
    {
        if (idx >= 0 && (unsigned)idx < mReanimations.mMaxUsedCount)
            return &mReanimations.mBlock[idx].mItem;
        return NULL;
    }
    void UpdateAll() {}
    void DrawCachedPlant(Sexy::Graphics* g, float x, float y, Reanimation* r) { (void)g; (void)x; (void)y; (void)r; }
    void DrawCachedPlant(Sexy::Graphics* g, float x, float y, SeedType theSeedType, DrawVariation theDrawVar) { (void)g; (void)x; (void)y; (void)theSeedType; (void)theDrawVar; }
    void DrawCachedZombie(Sexy::Graphics* g, float x, float y, Reanimation* r) { (void)g; (void)x; (void)y; (void)r; }
    void DrawCachedZombie(Sexy::Graphics* g, float x, float y, ZombieType theZombieType) { (void)g; (void)x; (void)y; (void)theZombieType; }
    void DrawCachedMower(Sexy::Graphics* g, float x, float y, int theMowerType) { (void)g; (void)x; (void)y; (void)theMowerType; }
};

#endif // __REANIMATOR_H__