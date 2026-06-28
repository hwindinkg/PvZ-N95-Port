/*
 * ReanimatorRuntime.cpp — full Reanimation runtime implementation.
 * Port of upstream Reanimator.cpp (1501 lines) + EffectSystem.cpp (541 lines).
 *
 * [Session-13] This implements the full Reanimation system:
 * - GetCurrentTransform interpolates between keyframes using GetFrameTime
 * - DrawTrack renders each track's image at the interpolated position
 *   with center-based positioning (matching upstream ReanimBltMatrix)
 * - DrawRenderGroup renders only tracks in the specified group
 * - PlayReanim sets up the animation window via GetFramesForLayer
 * - Update advances mAnimTime based on mAnimRate (fps) and mFrameCount
 */
#include "ReanimatorRuntime.h"
#include "../engine/Graphics.h"
#include "../engine/MemoryImage.h"
#include "../engine/Color.h"
#include "../engine/ResourceManager.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

// SECONDS_PER_UPDATE = 0.01 (matches upstream)
#define SECONDS_PER_UPDATE 0.01f

// ===========================================================================
// Helper: case-insensitive string compare (Symbian has no strcasecmp)
// ===========================================================================
static int StrCaseCmp(const char* a, const char* b)
{
    while (*a && *b)
    {
        char ca = *a; if (ca >= 'A' && ca <= 'Z') ca += 32;
        char cb = *b; if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return *a - *b;
}

static float FloatLerp(float a, float b, float t) { return a + (b - a) * t; }
static int FloatRoundToInt(float f) { return (int)(f >= 0 ? f + 0.5f : f - 0.5f); }

// ===========================================================================
// Reanimation
// ===========================================================================
Reanimation::Reanimation()
    : mDefinition(NULL)
    , mAnimTime(0.0f)
    , mAnimRate(0.0f)
    , mLoopType(REANIM_LOOP)
    , mDead(false)
    , mFrameStart(0)
    , mFrameCount(0)
    , mFrameBasePose(-2)
    , mX(0), mY(0)
    , mScaleX(1), mScaleY(1)
    , mLoopCount(0)
    , mRenderOrder(0)
    , mAlpha(1.0f)
    , mTrackInstances(NULL)
    , mLastFrameTime(-1.0f)
{
}

Reanimation::~Reanimation()
{
    if (mTrackInstances)
        delete[] mTrackInstances;
}

void Reanimation::ReanimationInitialize(float x, float y, ReanimDefinition* def)
{
    mDefinition = def;
    mX = x;
    mY = y;
    mDead = false;
    mAnimTime = 0.0f;
    mAnimRate = 0.0f;
    mLoopType = REANIM_LOOP;
    mLoopCount = 0;
    mScaleX = 1.0f;
    mScaleY = 1.0f;
    mAlpha = 1.0f;
    mLastFrameTime = -1.0f;

    if (def && def->mTrackCount > 0)
    {
        mFrameCount = def->mTracks[0].mTransformCount;
        mTrackInstances = new ReanimatorTrackInstance[def->mTrackCount];
        for (int i = 0; i < def->mTrackCount; i++)
            mTrackInstances[i] = ReanimatorTrackInstance();
    }
    else
    {
        mFrameCount = 0;
        mTrackInstances = NULL;
    }
}

// ===========================================================================
// Playback
// ===========================================================================
int Reanimation::FindTrackIndex(const char* trackName)
{
    if (!mDefinition || !trackName)
        return 0;
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        if (mDefinition->mTracks[i].mName &&
            StrCaseCmp(mDefinition->mTracks[i].mName, trackName) == 0)
            return i;
    }
    return 0; // upstream returns 0 on miss (with a trace)
}

bool Reanimation::TrackExists(const char* trackName)
{
    if (!mDefinition || !trackName)
        return false;
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        if (mDefinition->mTracks[i].mName &&
            StrCaseCmp(mDefinition->mTracks[i].mName, trackName) == 0)
            return true;
    }
    return false;
}

void Reanimation::GetFramesForLayer(const char* trackName, int& frameStart, int& frameCount)
{
    frameStart = 0;
    frameCount = 1;
    if (!mDefinition || mDefinition->mTrackCount == 0)
        return;

    int trackIdx = FindTrackIndex(trackName);
    if (trackIdx < 0 || trackIdx >= mDefinition->mTrackCount)
        return;

    ReanimTrack& track = mDefinition->mTracks[trackIdx];
    if (track.mTransformCount == 0)
        return;

    // Find first non-blank transform (mFrame >= 0)
    for (int i = 0; i < track.mTransformCount; i++)
    {
        if (track.mTransforms[i].mFrame >= 0.0f)
        {
            frameStart = i;
            break;
        }
    }
    // Find last non-blank transform
    for (int j = frameStart; j < track.mTransformCount; j++)
    {
        if (track.mTransforms[j].mFrame >= 0.0f)
            frameCount = j - frameStart + 1;
    }
}

void Reanimation::SetFramesForLayer(const char* trackName)
{
    if (mAnimRate >= 0)
        mAnimTime = 0.0f;
    else
        mAnimTime = 0.9999999f;
    mLastFrameTime = -1.0f;
    GetFramesForLayer(trackName, mFrameStart, mFrameCount);
}

void Reanimation::PlayReanim(const char* trackName, ReanimLoopType loopType,
                              int blendTime, float animRate)
{
    if (animRate != 0.0f)
        mAnimRate = animRate;
    mLoopType = loopType;
    mLoopCount = 0;
    SetFramesForLayer(trackName);
}

// ===========================================================================
// Update
// ===========================================================================
void Reanimation::Update()
{
    if (mFrameCount == 0 || mDead)
        return;

    mLastFrameTime = mAnimTime;
    mAnimTime += SECONDS_PER_UPDATE * mAnimRate / (float)mFrameCount;

    if (mAnimRate > 0)
    {
        switch (mLoopType)
        {
        case REANIM_LOOP:
        case REANIM_LOOP_FULL_LAST_FRAME:
            while (mAnimTime >= 1.0f)
            {
                mLoopCount++;
                mAnimTime -= 1.0f;
            }
            break;
        case REANIM_PLAY_ONCE:
        case REANIM_PLAY_ONCE_FULL_LAST_FRAME:
            if (mAnimTime >= 1.0f)
            {
                mLoopCount = 1;
                mAnimTime = 1.0f;
                mDead = true;
            }
            break;
        case REANIM_PLAY_ONCE_AND_HOLD:
        case REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD:
            if (mAnimTime >= 1.0f)
            {
                mLoopCount = 1;
                mAnimTime = 1.0f;
            }
            break;
        }
    }
    else if (mAnimRate < 0)
    {
        switch (mLoopType)
        {
        case REANIM_LOOP:
        case REANIM_LOOP_FULL_LAST_FRAME:
            while (mAnimTime < 0.0f)
            {
                mLoopCount++;
                mAnimTime += 1.0f;
            }
            break;
        case REANIM_PLAY_ONCE:
        case REANIM_PLAY_ONCE_FULL_LAST_FRAME:
            if (mAnimTime < 0.0f)
            {
                mLoopCount = 1;
                mAnimTime = 0.0f;
                mDead = true;
            }
            break;
        case REANIM_PLAY_ONCE_AND_HOLD:
        case REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD:
            if (mAnimTime < 0.0f)
            {
                mLoopCount = 1;
                mAnimTime = 0.0f;
            }
            break;
        }
    }
}

// ===========================================================================
// Frame time + transform
// ===========================================================================
void Reanimation::GetFrameTime(ReanimatorFrameTime* ft)
{
    int aFrameCount;
    if (mLoopType == REANIM_PLAY_ONCE_FULL_LAST_FRAME ||
        mLoopType == REANIM_LOOP_FULL_LAST_FRAME ||
        mLoopType == REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD)
        aFrameCount = mFrameCount;
    else
        aFrameCount = mFrameCount - 1;

    float aAnimPosition = mFrameStart + mAnimTime * (float)aFrameCount;
    float aAnimFrameBefore = floorf(aAnimPosition);
    ft->mFraction = aAnimPosition - aAnimFrameBefore;
    ft->mAnimFrameBeforeInt = FloatRoundToInt(aAnimFrameBefore);

    if (ft->mAnimFrameBeforeInt >= mFrameStart + mFrameCount - 1)
    {
        ft->mAnimFrameBeforeInt = mFrameStart + mFrameCount - 1;
        ft->mAnimFrameAfterInt = ft->mAnimFrameBeforeInt;
    }
    else
    {
        ft->mAnimFrameAfterInt = ft->mAnimFrameBeforeInt + 1;
    }
}

// [Session-13] Helper: scan backwards from index to find the nearest
// transform with an image (images only appear in the first transform
// of each track in the XML format).
static void ScanForImage(ReanimTrack& track, int fromIdx, ReanimTransform* t)
{
    t->mImage = NULL;
    t->mImageName = "";
    t->mFontName = "";
    t->mText = "";
    for (int k = fromIdx; k >= 0; k--)
    {
        ReanimTransform& scan = track.mTransforms[k];
        if (scan.mImage || (scan.mImageName && scan.mImageName[0] != '\0'))
        {
            t->mImage = scan.mImage;
            t->mImageName = scan.mImageName;
            t->mFontName = scan.mFontName;
            t->mText = scan.mText;
            return;
        }
    }
}

void Reanimation::GetTransformAtTime(int trackIndex, ReanimTransform* t,
                                      ReanimatorFrameTime* ft)
{
    if (!mDefinition || trackIndex < 0 || trackIndex >= mDefinition->mTrackCount)
        return;

    ReanimTrack& track = mDefinition->mTracks[trackIndex];
    int n = track.mTransformCount;
    if (n <= 0)
        return;

    int before = ft->mAnimFrameBeforeInt;
    int after = ft->mAnimFrameAfterInt;
    if (before < 0) before = 0;
    if (before >= n) before = n - 1;
    if (after < 0) after = 0;
    if (after >= n) after = n - 1;

    ReanimTransform& a = track.mTransforms[before];
    ReanimTransform& b = track.mTransforms[after];
    float frac = ft->mFraction;

    t->mTransX = FloatLerp(a.mTransX, b.mTransX, frac);
    t->mTransY = FloatLerp(a.mTransY, b.mTransY, frac);
    t->mSkewX  = FloatLerp(a.mSkewX,  b.mSkewX,  frac);
    t->mSkewY  = FloatLerp(a.mSkewY,  b.mSkewY,  frac);
    t->mScaleX = FloatLerp(a.mScaleX, b.mScaleX, frac);
    t->mScaleY = FloatLerp(a.mScaleY, b.mScaleY, frac);
    t->mAlpha  = FloatLerp(a.mAlpha,  b.mAlpha,  frac);
    t->mFrame  = a.mFrame;

    // Image: scan backwards from 'before' to find the nearest transform
    // with an image (images persist from the last keyframe that set them).
    ScanForImage(track, before, t);
}

void Reanimation::GetCurrentTransform(int trackIndex, ReanimTransform* t)
{
    ReanimatorFrameTime ft;
    GetFrameTime(&ft);
    GetTransformAtTime(trackIndex, t, &ft);
}

// ===========================================================================
// Rendering
// ===========================================================================
bool Reanimation::DrawTrack(Sexy::Graphics* g, int trackIndex)
{
    if (!mDefinition || !g)
        return false;
    if (trackIndex < 0 || trackIndex >= mDefinition->mTrackCount)
        return false;

    ReanimTransform t;
    GetCurrentTransform(trackIndex, &t);

    // Check if this frame is blank (mFrame < 0 means hidden)
    if (t.mFrame < 0.0f)
        return false;

    // Apply image override
    if (mTrackInstances && mTrackInstances[trackIndex].mImageOverride)
        t.mImage = mTrackInstances[trackIndex].mImageOverride;

    if (!t.mImage)
        return false;

    // Compute alpha
    float alpha = t.mAlpha * mAlpha;
    if (alpha <= 0.0f)
        return false;
    int aImageAlpha = (int)(alpha * 255.0f);
    if (aImageAlpha > 255) aImageAlpha = 255;
    if (aImageAlpha <= 0) return false;

    // Center-based positioning (matches upstream ReanimBltMatrix fast path):
    //   posX = transX - scaleX * imageWidth * 0.5
    //   posY = transY - scaleY * imageHeight * 0.5
    float imgW = t.mImage->GetWidth();
    float imgH = t.mImage->GetHeight();
    float scaledW = imgW * t.mScaleX * mScaleX;
    float scaledH = imgH * t.mScaleY * mScaleY;

    // Position in reanim space (800×600, center-origin)
    float posX = mX + t.mTransX - scaledW * 0.5f;
    float posY = mY + t.mTransY - scaledH * 0.5f;

    // Scale to canvas (×0.5: reanim 800×600 → canvas 400×300)
    // +400, +300 offset to convert center-origin to top-left-origin
    float cx = (posX + 400.0f) * 0.5f;
    float cy = (posY + 300.0f) * 0.5f;
    float cw = scaledW * 0.5f;
    float ch = scaledH * 0.5f;

    if (cw < 1.0f || ch < 1.0f)
        return false;

    // Set white color for texture (GL_MODULATE)
    g->SetColor(Sexy::Color(255, 255, 255, 255));

    Sexy::MemoryImage* mem = static_cast<Sexy::MemoryImage*>(t.mImage);
    g->DrawImage(mem, (int)cx, (int)cy, (int)cw, (int)ch);
    return true;
}

void Reanimation::DrawRenderGroup(Sexy::Graphics* g, int renderGroup)
{
    if (mDead || !mDefinition || !g)
        return;

    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        if (mTrackInstances && mTrackInstances[i].mRenderGroup == renderGroup)
        {
            DrawTrack(g, i);
        }
    }
}

void Reanimation::Draw(Sexy::Graphics* g)
{
    DrawRenderGroup(g, 0); // RENDER_GROUP_NORMAL = 0
}

// ===========================================================================
// Utility
// ===========================================================================
void Reanimation::AssignRenderGroupToTrack(const char* trackName, int group)
{
    int idx = FindTrackIndex(trackName);
    if (mTrackInstances && idx >= 0 && idx < mDefinition->mTrackCount)
        mTrackInstances[idx].mRenderGroup = group;
}

void Reanimation::AssignRenderGroupToPrefix(const char* prefix, int group)
{
    if (!mDefinition || !prefix || !mTrackInstances)
        return;
    int prefixLen = strlen(prefix);
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        if (mDefinition->mTracks[i].mName &&
            StrCaseCmp(mDefinition->mTracks[i].mName, prefix) == 0)
        {
            // Check it's a prefix match (not partial)
            char c = mDefinition->mTracks[i].mName[prefixLen];
            if (c == '\0' || c == '_' || c == ' ')
                mTrackInstances[i].mRenderGroup = group;
        }
    }
}

void Reanimation::SetImageOverride(const char* trackName, Sexy::Image* img)
{
    int idx = FindTrackIndex(trackName);
    if (mTrackInstances && idx >= 0 && idx < mDefinition->mTrackCount)
        mTrackInstances[idx].mImageOverride = img;
}

// ===========================================================================
// ReanimationHolder (EffectSystem)
// ===========================================================================
ReanimationHolder::ReanimationHolder()
    : mCount(0)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
        mReanimations[i] = NULL;
}

ReanimationHolder::~ReanimationHolder()
{
    Clear();
}

Reanimation* ReanimationHolder::AllocReanimation(float x, float y, int renderOrder,
                                                   ReanimDefinition* def)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] == NULL)
        {
            Reanimation* r = new Reanimation();
            if (!r) return NULL;
            r->ReanimationInitialize(x, y, def);
            r->mRenderOrder = renderOrder;
            mReanimations[i] = r;
            if (i >= mCount) mCount = i + 1;
            return r;
        }
    }
    return NULL; // no free slot
}

void ReanimationHolder::RemoveReanimation(Reanimation* r)
{
    if (!r) return;
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] == r)
        {
            delete r;
            mReanimations[i] = NULL;
            return;
        }
    }
}

Reanimation* ReanimationHolder::GetReanimation(int idx)
{
    if (idx < 0 || idx >= MAX_REANIMATIONS)
        return NULL;
    return mReanimations[idx];
}

void ReanimationHolder::UpdateAll()
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] && !mReanimations[i]->mDead)
            mReanimations[i]->Update();
    }
}

void ReanimationHolder::DrawAll(Sexy::Graphics* g)
{
    DrawAllRenderGroup(g, 0);
}

void ReanimationHolder::DrawAllRenderGroup(Sexy::Graphics* g, int renderGroup)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] && !mReanimations[i]->mDead)
            mReanimations[i]->DrawRenderGroup(g, renderGroup);
    }
}

void ReanimationHolder::Clear()
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i])
        {
            delete mReanimations[i];
            mReanimations[i] = NULL;
        }
    }
    mCount = 0;
}

// ===========================================================================
// EffectSystem
// ===========================================================================
EffectSystem* gEffectSystem = NULL;

EffectSystem::EffectSystem()
    : mReanimationHolder(NULL)
{
}

EffectSystem::~EffectSystem()
{
    EffectSystemDispose();
}

void EffectSystem::EffectSystemInitialize()
{
    mReanimationHolder = new ReanimationHolder();
}

void EffectSystem::EffectSystemDispose()
{
    if (mReanimationHolder)
    {
        mReanimationHolder->Clear();
        delete mReanimationHolder;
        mReanimationHolder = NULL;
    }
}

void EffectSystem::Update()
{
    if (mReanimationHolder)
        mReanimationHolder->UpdateAll();
}

void EffectSystem::Draw(Sexy::Graphics* g)
{
    if (mReanimationHolder)
        mReanimationHolder->DrawAll(g);
}
