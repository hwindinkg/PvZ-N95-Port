/*
 * ReanimLoader.h -- .reanim XML file loader for Symbian port.
 *
 * Parses the architecture-independent XML version of PopCap reanimation
 * files from PAK (NOT the .reanim.compiled binary, whose 64-bit struct
 * sizes from the PC build are incompatible with the N95's 32-bit ARM).
 *
 *   1. Read XML from PAK (PvZVfs handles XOR 0xF7 decryption)
 *   2. Hand-rolled XML scan (FindElement) -> ReanimDefinition -> tracks
 *      -> transforms, loading <i> image refs via ResourceManager.
 *
 * This is a focused port of the reanimation-XML subset of upstream
 * Definition.cpp (1445 lines) / DefinitionLoadXML. It does NOT implement
 * the full DefMap/DefField system, particle/trail/emitter definitions, or
 * the compiled-binary reader. Those arrive with the Stage 2/4 ports.
 */
#ifndef __REANIMLOADER_H__
#define __REANIMLOADER_H__

#include <e32def.h>

namespace Sexy { class Image; class Graphics; }

// ReanimatorTransform — per-frame bone transform (matches upstream layout)
struct ReanimTransform
{
    float mTransX;     // x
    float mTransY;     // y
    float mSkewX;      // kx
    float mSkewY;      // ky
    float mScaleX;     // sx
    float mScaleY;     // sy
    float mFrame;      // f (stored as float, cast to int for frame index)
    float mAlpha;      // a (0-255)
    // DT_IMAGE: loaded separately (image name string follows in stream)
    // DT_FONT: loaded separately (font name string follows in stream)
    // DT_STRING: loaded separately (text string follows in stream)
    Sexy::Image* mImage;   // resolved image (or NULL until lazy-loaded in Draw)
    const char*  mImageName; // image resource name (e.g. "IMAGE_REANIM_..."), or ""
    const char*  mFontName; // font resource name (or "")
    const char*  mText;     // text string (or "")

    ReanimTransform() : mTransX(0), mTransY(0), mSkewX(0), mSkewY(0),
        mScaleX(1), mScaleY(1), mFrame(-1.0f), mAlpha(255),
        mImage(NULL), mImageName(""), mFontName(""), mText("") {}
};

// ReanimTrack — named animation track with array of transforms
struct ReanimTrack
{
    const char*       mName;        // track name (e.g. "SelectorScreen_BG")
    int               mTransformCount;
    ReanimTransform*  mTransforms;  // array of mTransformCount transforms

    ReanimTrack() : mName(""), mTransformCount(0), mTransforms(NULL) {}
};

// ReanimDefinition — parsed .reanim XML data
struct ReanimDefinition
{
    int           mTrackCount;
    ReanimTrack*  mTracks;       // array of mTrackCount tracks
    float         mFPS;          // animation FPS (usually 12-20)

    ReanimDefinition() : mTrackCount(0), mTracks(NULL), mFPS(12.0f) {}
    ~ReanimDefinition();
};

// Load a .reanim XML file from PAK.
// Returns true on success, fills outDefinition.
// The caller owns outDefinition (call delete on it when done).
TBool ReanimLoadCompiled(const char* aPakPath, ReanimDefinition& outDefinition);

// Find a track by name (case-insensitive). Returns NULL if not found.
ReanimTrack* ReanimFindTrack(ReanimDefinition& aDef, const char* aName);

// ===========================================================================
// ReanimPlayer — lightweight reanimation runtime.
//
// Plays a ReanimDefinition (parsed by ReanimLoadCompiled) by advancing
// mAnimTime and interpolating each track's transforms between keyframes,
// then drawing every track's image at the interpolated position/scale/alpha.
//
// This is a focused port of the draw/interpolation core of upstream
// Reanimator.cpp (1501 lines). It does NOT implement: render groups,
// attachments, blend layers, color override, filter effects, skew matrices,
// or TodTriangleGroup textured-triangle rendering. Tracks are drawn as
// axis-aligned scaled sprites (transX/transY/scaleX/scaleY/alpha), which is
// sufficient for the SelectorScreen menu and a foundation for gameplay.
//
// Coordinate space: reanim files use 800x600. Set mCoordScale to map to the
// target canvas (0.5 for the port's 400x300 logical canvas).
//
// Lifetime: the player does NOT own the ReanimDefinition; the caller must
// keep it alive for the player's lifetime.
// ===========================================================================
class ReanimPlayer
{
public:
    enum LoopType { LOOP_OFF = 0, LOOP_ON = 1 };

    ReanimDefinition* mDefinition;   // non-owning; NULL until SetDefinition
    float             mAnimTime;     // current playback position, in seconds
    float             mAnimRate;     // playback rate multiplier (1.0 = normal)
    int               mLoopType;     // LOOP_OFF or LOOP_ON
    TBool             mDead;         // true once a non-looping anim finishes

    // Global transform applied to every track (in reanim's 800x600 space).
    float             mX, mY;
    float             mScaleX, mScaleY;
    float             mCoordScale;   // reanim-space -> screen-space (0.5 = halve)
    float             mAlpha;        // global alpha multiplier (0-255)

    ReanimPlayer();

    // Bind a definition and reset playback.
    void  SetDefinition(ReanimDefinition* aDef);

    // Advance playback. aDtSeconds is the elapsed wall-clock seconds.
    void  Update(float aDtSeconds);

    // Render every track's current frame. Tracks with no image are skipped.
    void  Draw(Sexy::Graphics* g);

    // Find a track index by name (case-insensitive). Returns -1 if not found.
    int   FindTrackIndex(const char* aName);

    // Compute the interpolated transform for track aTrackIndex at the current
    // mAnimTime. Returns EFalse if the track has no transforms.
    TBool GetCurrentTransform(int aTrackIndex, ReanimTransform& aOut);

    // Total duration in seconds (max keyframe / FPS), or 0 if no tracks.
    float GetDuration();
};

#endif // __REANIMLOADER_H__

