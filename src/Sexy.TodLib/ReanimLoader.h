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

namespace Sexy { class Image; }

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
    Sexy::Image* mImage;   // resolved image (or NULL)
    const char*  mFontName; // font resource name (or "")
    const char*  mText;     // text string (or "")

    ReanimTransform() : mTransX(0), mTransY(0), mSkewX(0), mSkewY(0),
        mScaleX(1), mScaleY(1), mFrame(0), mAlpha(255),
        mImage(NULL), mFontName(""), mText("") {}
};

// ReanimTrack — named animation track with array of transforms
struct ReanimTrack
{
    const char*       mName;        // track name (e.g. "SelectorScreen_BG")
    int               mTransformCount;
    ReanimTransform*  mTransforms;  // array of mTransformCount transforms

    ReanimTrack() : mName(""), mTransformCount(0), mTransforms(NULL) {}
};

// ReanimDefinition — parsed .reanim.compiled data
struct ReanimDefinition
{
    int           mTrackCount;
    ReanimTrack*  mTracks;       // array of mTrackCount tracks
    float         mFPS;          // animation FPS (usually 12-20)

    ReanimDefinition() : mTrackCount(0), mTracks(NULL), mFPS(12.0f) {}
    ~ReanimDefinition();
};

// Load a .reanim.compiled file from PAK.
// Returns true on success, fills outDefinition.
// The caller owns outDefinition (call delete on it when done).
TBool ReanimLoadCompiled(const char* aPakPath, ReanimDefinition& outDefinition);

// Find a track by name (case-insensitive). Returns NULL if not found.
ReanimTrack* ReanimFindTrack(ReanimDefinition& aDef, const char* aName);

#endif // __REANIMLOADER_H__
