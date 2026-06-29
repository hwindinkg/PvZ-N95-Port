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

// [Session-14] Case-insensitive PREFIX compare (strncasecmp equivalent).
// Symbian has no strncasecmp; this matches upstream's AssignRenderGroupToPrefix
// which uses strncasecmp(trackName, prefix, prefixLen) to test whether a track
// name STARTS WITH the given prefix (case-insensitive).
static int StrnCaseCmp(const char* a, const char* b, int n)
{
    for (int i = 0; i < n; i++)
    {
        char ca = a[i]; char cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb || ca == '\0') return ca - cb;
    }
    return 0;  // first n chars match
}

static float FloatLerp(float a, float b, float t) { return a + (b - a) * t; }
static int FloatRoundToInt(float f) { return (int)(f >= 0 ? f + 0.5f : f - 0.5f); }

// ===========================================================================
// Reanimation
// ===========================================================================
Reanim2::Reanim2()
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

Reanim2::~Reanim2()
{
    if (mTrackInstances)
        delete[] mTrackInstances;
}

void Reanim2::ReanimationInitialize(float x, float y, ReanimDefinition* def)
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
    mAlpha = 1.0f;  // [Session-13] mAlpha is a multiplier on t.mAlpha (0-255).
                     // 1.0 means use the transform's alpha as-is.
    mLastFrameTime = -1.0f;

    if (def && def->mTrackCount > 0)
    {
        mFrameCount = def->mTracks[0].mTransformCount;
        mTrackInstances = new ReanimTrackInst[def->mTrackCount];
        for (int i = 0; i < def->mTrackCount; i++)
            mTrackInstances[i] = ReanimTrackInst();
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
int Reanim2::FindTrackIndex(const char* trackName)
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

bool Reanim2::TrackExists(const char* trackName)
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

void Reanim2::GetFramesForLayer(const char* trackName, int& frameStart, int& frameCount)
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

    // [Session-13] In the XML format, mFrame is -1 by default (no <f> tag).
    // But transforms with images are NOT blank. Use the full transform range
    // [0, mTransformCount) as the animation window. The marker tracks
    // (anim_open, anim_sign) don't have meaningful <f> values in XML —
    // the animation plays across ALL transforms.
    frameStart = 0;
    frameCount = track.mTransformCount;
}

void Reanim2::SetFramesForLayer(const char* trackName)
{
    if (mAnimRate >= 0)
        mAnimTime = 0.0f;
    else
        mAnimTime = 0.9999999f;
    mLastFrameTime = -1.0f;
    GetFramesForLayer(trackName, mFrameStart, mFrameCount);
}

void Reanim2::PlayReanim(const char* trackName, ReanimLoopType loopType,
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
void Reanim2::Update()
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
void Reanim2::GetFrameTime(ReanimatorFrameTime* ft)
{
    int aFrameCount;
    if (mLoopType == REANIM_PLAY_ONCE_FULL_LAST_FRAME ||
        mLoopType == REANIM_LOOP_FULL_LAST_FRAME ||
        mLoopType == REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD)
        aFrameCount = mFrameCount;
    else
        aFrameCount = mFrameCount - 1;

    float aAnimPosition = mFrameStart + mAnimTime * (float)aFrameCount;
    // [Session-13] floorf is C99, not available in Symbian GCCE math.h.
    // Use manual floor: cast to int (truncates toward zero), adjust for negatives.
    float aAnimFrameBefore = (float)(int)aAnimPosition;
    if (aAnimPosition < 0.0f && aAnimPosition != aAnimFrameBefore)
        aAnimFrameBefore -= 1.0f;
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

void Reanim2::GetTransformAtTime(int trackIndex, ReanimTransform* t,
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

void Reanim2::GetCurrentTransform(int trackIndex, ReanimTransform* t)
{
    ReanimatorFrameTime ft;
    GetFrameTime(&ft);
    GetTransformAtTime(trackIndex, t, &ft);
}

// ===========================================================================
// Rendering
// ===========================================================================
bool Reanim2::DrawTrack(Sexy::Graphics* g, int trackIndex)
{
    if (!mDefinition || !g)
        return false;
    if (trackIndex < 0 || trackIndex >= mDefinition->mTrackCount)
        return false;

    ReanimTransform t;
    GetCurrentTransform(trackIndex, &t);

    // [Session-13] In the XML format, mFrame is -1 by default (no <f> tag).
    // But that doesn't mean the track is hidden — the IMAGE is what matters.
    // Only skip if there's no image AND no image name.
    // (Upstream's compiled format has mFrame=-1 for truly blank frames, but
    // our XML format just doesn't have <f> tags for most transforms.)
    //
    // Apply image override
    if (mTrackInstances && mTrackInstances[trackIndex].mImageOverride)
        t.mImage = mTrackInstances[trackIndex].mImageOverride;

    if (!t.mImage)
        return false;

    // Compute alpha — t.mAlpha is 0-255 from the reanim transform.
    // mAlpha is the global Reanimation alpha multiplier (default 1.0).
    // Final alpha = t.mAlpha * mAlpha, clamped to [0, 255].
    int aImageAlpha = (int)(t.mAlpha * mAlpha);
    if (aImageAlpha <= 0)
        return false;
    if (aImageAlpha > 255) aImageAlpha = 255;

    // [Session-13] Position = (mX + transX, mY + transY) — NO center offset.
    float imgW = t.mImage->GetWidth();
    float imgH = t.mImage->GetHeight();
    float scaledW = imgW * t.mScaleX * mScaleX;
    float scaledH = imgH * t.mScaleY * mScaleY;
    float posX = mX + t.mTransX;
    float posY = mY + t.mTransY;

    // Scale to canvas (×0.5: reanim 800×600 → canvas 400×300)
    float cx = posX * 0.5f;
    float cy = posY * 0.5f;
    float cw = scaledW * 0.5f;
    float ch = scaledH * 0.5f;

    if (cw < 1.0f || ch < 1.0f)
        return false;

    // [Session-13] Removed off-screen culling — GL clips automatically.
    // Buttons at y=624+ are off-screen at frame 0 but slide into view
    // during anim_open. Culling prevented them from being drawn even
    // when partially visible.

    // [Session-13] Use vertex alpha for transparency modulation.
    // GL_MODULATE multiplies texture color by vertex color.
    // t.mAlpha is 0-255, scale to 0-255 for the vertex alpha.
    g->SetColor(Sexy::Color(255, 255, 255, aImageAlpha));

    Sexy::MemoryImage* mem = static_cast<Sexy::MemoryImage*>(t.mImage);
    g->DrawImage(mem, (int)cx, (int)cy, (int)cw, (int)ch);
    return true;
}

void Reanim2::DrawRenderGroup(Sexy::Graphics* g, int renderGroup)
{
    if (mDead || !mDefinition || !g)
        return;

    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        // [Session-13] RENDER_GROUP_HIDDEN = -1: never draw these tracks.
        // DrawRenderGroup(g, 0) should ONLY draw group 0 tracks, not -1.
        // DrawRenderGroup(g, 1) draws BG (group 1).
        if (mTrackInstances && mTrackInstances[i].mRenderGroup == renderGroup)
        {
            DrawTrack(g, i);
        }
    }
}

void Reanim2::Draw(Sexy::Graphics* g)
{
    DrawRenderGroup(g, 0); // RENDER_GROUP_NORMAL = 0
}

// ===========================================================================
// Utility
// ===========================================================================
void Reanim2::AssignRenderGroupToTrack(const char* trackName, int group)
{
    int idx = FindTrackIndex(trackName);
    if (mTrackInstances && idx >= 0 && idx < mDefinition->mTrackCount)
        mTrackInstances[idx].mRenderGroup = group;
}

void Reanim2::AssignRenderGroupToPrefix(const char* prefix, int group)
{
    if (!mDefinition || !prefix || !mTrackInstances)
        return;
    int prefixLen = strlen(prefix);
    if (prefixLen <= 0)
        return;
    // [Session-14] 1:1 upstream: use strncasecmp for PREFIX match.
    // Previously this used StrCaseCmp (full compare) which only matched
    // exact track names — so AssignRenderGroupToPrefix("flower", -1)
    // never matched "SelectorScreen_Flower1". Now it correctly matches
    // any track whose name STARTS WITH the prefix (case-insensitive).
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        const char* nm = mDefinition->mTracks[i].mName;
        if (nm && (int)strlen(nm) >= prefixLen &&
            StrnCaseCmp(nm, prefix, prefixLen) == 0)
        {
            mTrackInstances[i].mRenderGroup = group;
        }
    }
}

void Reanim2::SetImageOverride(const char* trackName, Sexy::Image* img)
{
    int idx = FindTrackIndex(trackName);
    if (mTrackInstances && idx >= 0 && idx < mDefinition->mTrackCount)
        mTrackInstances[idx].mImageOverride = img;
}

// ===========================================================================
// class ReanimHolder2 (EffectSystem)
// ===========================================================================
ReanimHolder2::ReanimHolder2()
    : mCount(0)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
        mReanimations[i] = NULL;
}

ReanimHolder2::~ReanimHolder2()
{
    Clear();
}

Reanim2* ReanimHolder2::AllocReanimation(float x, float y, int renderOrder,
                                                   ReanimDefinition* def)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] == NULL)
        {
            Reanim2* r = new Reanim2();
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

void ReanimHolder2::RemoveReanimation(Reanim2* r)
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

Reanim2* ReanimHolder2::GetReanimation(int idx)
{
    if (idx < 0 || idx >= MAX_REANIMATIONS)
        return NULL;
    return mReanimations[idx];
}

void ReanimHolder2::UpdateAll()
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] && !mReanimations[i]->mDead)
            mReanimations[i]->Update();
    }
}

void ReanimHolder2::DrawAll(Sexy::Graphics* g)
{
    DrawAllRenderGroup(g, 0);
}

void ReanimHolder2::DrawAllRenderGroup(Sexy::Graphics* g, int renderGroup)
{
    for (int i = 0; i < MAX_REANIMATIONS; i++)
    {
        if (mReanimations[i] && !mReanimations[i]->mDead)
            mReanimations[i]->DrawRenderGroup(g, renderGroup);
    }
}

void ReanimHolder2::Clear()
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
EffectSystem2* gEffectSystem2 = NULL;

EffectSystem2::EffectSystem2()
    : mReanimHolder2(NULL)
{
}

EffectSystem2::~EffectSystem2()
{
    EffectSystemDispose();
}

void EffectSystem2::EffectSystemInitialize()
{
    mReanimHolder2 = new ReanimHolder2();
}

void EffectSystem2::EffectSystemDispose()
{
    if (mReanimHolder2)
    {
        mReanimHolder2->Clear();
        delete mReanimHolder2;
        mReanimHolder2 = NULL;
    }
}

void EffectSystem2::Update()
{
    if (mReanimHolder2)
        mReanimHolder2->UpdateAll();
}

void EffectSystem2::Draw(Sexy::Graphics* g)
{
    if (mReanimHolder2)
        mReanimHolder2->DrawAll(g);
}
