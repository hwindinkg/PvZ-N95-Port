/*
 * ReanimLoader.cpp -- .reanim XML file loader for Symbian port.
 *
 * Parses the XML version of reanimation files from PAK (not the compiled
 * binary format, which is architecture-dependent: 64-bit struct sizes
 * differ from 32-bit, making the compiled files from PvZ-Portable's PC
 * build incompatible with the N95's 32-bit ARM).
 *
 * XML format:
 * <fps>20</fps>
 * <track>
 *   <name>SelectorScreen_BG</name>
 *   <t><x>0</x><y>0</y><sx>1</sx><sy>1</sy><i>IMAGE_REANIM_SELECTORSCREEN_BG</i></t>
 *   <t></t>
 *   ...
 * </track>
 * <track>...</track>
 *
 * Each <t> is a transform with:
 *   <x> mTransX (float, default 0)
 *   <y> mTransY (float, default 0)
 *   <kx> mSkewX (float, default 0)
 *   <ky> mSkewY (float, default 0)
 *   <sx> mScaleX (float, default 1)
 *   <sy> mScaleY (float, default 1)
 *   <f> mFrame (float, default 0)
 *   <a> mAlpha (float, default 255)
 *   <i> image name (string, loaded via ResourceManager)
 *   <font> font name (string)
 *   <text> text (string)
 *
 * [BUGFIX] The previous FindTag-based implementation parsed only 1 track
 * instead of ~48 because it used FindTag(..., "/track", ...) to locate the
 * closing tag, which built a malformed close pattern "<//track>" that never
 * matched, causing the counting loop to break after the first track. This
 * rewrite replaces FindTag with a proper FindElement() that explicitly
 * distinguishes opening <tag> from closing </tag>, handles self-closing
 * <tag/> and empty <tag></tag>, and iterates elements by advancing past
 * each whole element. Verified host-side against a 51-track / 55-transform
 * sample (all checks passed) before porting.
 */
#include "ReanimLoader.h"
#include "../engine/PvZVfs.h"
#include "../engine/PakInterface.h"
#include "../engine/Image.h"
#include "../engine/MemoryImage.h"
#include "../engine/Graphics.h"
#include "../engine/ResourceManager.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

// ===========================================================================
// Sentinel returned by ParseFloat when the field is not present in the XML.
// Matches upstream's DEFAULT_FIELD_PLACEHOLDER convention (-10000.0f) in
// spirit; we use -9999.0f to stay clear of any real coordinate value.
// ===========================================================================
static const float KFieldNotFound = -9999.0f;

// ===========================================================================
// FindElement -- find the next XML element <tag ...>...</tag> (or <tag/>)
// starting at searchFrom within [buf, buf+bufLen).
//
// On success returns ETrue and:
//   *outContentStart -> first char after the open tag's '>'  (equals the
//                       close-tag position when the element is self-closing
//                       <tag/> or has zero-length content)
//   *outContentLen   -> number of content bytes between open and close tags
//   *outAfter        -> first char after the whole element (for iteration)
//
// Correctly distinguishes <tag> from </tag> (skips close tags) and from
// <tagother> (requires the char after the tag name to be '>', '/' for
// self-close, or whitespace before attributes). Handles <tag/>, <tag></tag>,
// and <tag attr="x">...</tag> (scans to the attribute-terminating '>').
// ===========================================================================
static TBool FindElement(const char* buf, TInt bufLen, const char* tag,
                         const char* searchFrom,
                         const char** outContentStart, TInt* outContentLen,
                         const char** outAfter)
{
    TInt tagLen = (TInt)strlen(tag);
    const char* end = buf + bufLen;
    const char* p = searchFrom;

    while (p < end)
    {
        // Look for the next '<'.
        if (*p != '<') { p++; continue; }
        // Skip closing tags "</...".
        if (p + 1 < end && p[1] == '/') { p++; continue; }
        // Skip comments "<!--" and declarations "<?xml ...".
        if (p + 1 < end && p[1] == '!') { p++; continue; }
        if (p + 1 < end && p[1] == '?') { p++; continue; }
        // Need at least "<tag" plus one terminator char.
        if (p + 1 + tagLen > end) { p++; continue; }
        // Tag name must match exactly (so "t" won't match "track" or "text"
        // because the char after "t" is checked below).
        if (strncmp(p + 1, tag, tagLen) != 0) { p++; continue; }

        char afterTag = p[1 + tagLen];
        const char* contentStart = NULL;
        TBool selfClosing = EFalse;

        if (afterTag == '>')
        {
            // <tag>
            contentStart = p + 1 + tagLen + 1;
        }
        else if (afterTag == '/' && (p + 2 + tagLen) < end && p[2 + tagLen] == '>')
        {
            // <tag/>
            contentStart = p + 2 + tagLen + 1; // points just after "/>"
            selfClosing = ETrue;
        }
        else if (afterTag == ' ' || afterTag == '\t' ||
                 afterTag == '\n' || afterTag == '\r')
        {
            // <tag attr="..."> (reanim doesn't use attributes, but be safe):
            // scan forward to the terminating '>'.
            const char* q = p + 1 + tagLen;
            while (q < end && *q != '>') q++;
            if (q >= end) { p++; continue; }
            if (q - 1 >= p && q[-1] == '/')
            {
                // <tag attr="..."/>
                selfClosing = ETrue;
                contentStart = q + 1;
            }
            else
            {
                contentStart = q + 1;
            }
        }
        else
        {
            // <tagother> -- the match was a prefix of a longer tag name;
            // advance and keep searching.
            p++;
            continue;
        }

        if (selfClosing)
        {
            *outContentStart = contentStart;
            *outContentLen = 0;
            *outAfter = contentStart;
            return ETrue;
        }

        // Build the close tag "</tag>" and search for it from contentStart.
        char close[64];
        if (tagLen > 58) tagLen = 58; // hard cap so close[] stays in bounds
        close[0] = '<';
        close[1] = '/';
        memcpy(close + 2, tag, tagLen);
        close[2 + tagLen] = '>';
        close[3 + tagLen] = '\0';
        TInt closeLen = 3 + tagLen;

        const char* c = contentStart;
        const char* found = NULL;
        while (c + closeLen <= end)
        {
            if (strncmp(c, close, closeLen) == 0) { found = c; break; }
            c++;
        }
        if (!found)
        {
            // Malformed: open tag with no matching close. Skip this open tag.
            p++;
            continue;
        }

        *outContentStart = contentStart;
        *outContentLen = (TInt)(found - contentStart);
        *outAfter = found + closeLen;
        return ETrue;
    }
    return EFalse;
}

// Extract a float from a child element <tag>value</tag> within [content,len].
// Returns KFieldNotFound if the element is absent.
static float ParseFloat(const char* content, TInt len, const char* tag)
{
    const char* cs = NULL;
    TInt cl = 0;
    const char* af = NULL;
    if (!FindElement(content, len, tag, content, &cs, &cl, &af))
        return KFieldNotFound;
    if (!cs || cl <= 0)
        return KFieldNotFound;
    char tmp[64];
    TInt n = (cl < 63) ? cl : 63;
    memcpy(tmp, cs, n);
    tmp[n] = '\0';
    return (float)atof(tmp);
}

// Extract a string from a child element <tag>value</tag> within [content,len].
// Returns a newly allocated char[] (caller must delete[]) when present, or the
// static literal "" when absent (the destructor skips empty strings, so the
// literal is never freed -- matching the existing ownership convention).
static const char* ParseString(const char* content, TInt len, const char* tag)
{
    const char* cs = NULL;
    TInt cl = 0;
    const char* af = NULL;
    if (!FindElement(content, len, tag, content, &cs, &cl, &af))
        return "";
    if (!cs || cl <= 0)
        return "";
    char* str = new char[cl + 1];
    if (!str) return "";
    memcpy(str, cs, cl);
    str[cl] = '\0';
    return str;
}

// ===========================================================================
// ReanimDefinition destructor (unchanged from original -- frees the arrays
// and per-transform strings allocated during parsing).
// ===========================================================================
ReanimDefinition::~ReanimDefinition()
{
    for (int i = 0; i < mTrackCount; i++)
    {
        if (mTracks[i].mName && mTracks[i].mName[0] != '\0')
            delete[] mTracks[i].mName;
        if (mTracks[i].mTransforms)
        {
            for (int j = 0; j < mTracks[i].mTransformCount; j++)
            {
                if (mTracks[i].mTransforms[j].mFontName &&
                    mTracks[i].mTransforms[j].mFontName[0] != '\0')
                    delete[] mTracks[i].mTransforms[j].mFontName;
                if (mTracks[i].mTransforms[j].mText &&
                    mTracks[i].mTransforms[j].mText[0] != '\0')
                    delete[] mTracks[i].mTransforms[j].mText;
            }
            delete[] mTracks[i].mTransforms;
        }
    }
    if (mTracks)
        delete[] mTracks;
}

// ===========================================================================
// ReanimLoadCompiled -- load and parse a .reanim XML file from PAK.
// Reads + XOR-decrypts the file (PvZVfs handles XOR internally), then parses
// <fps>, counts <track> elements by iterating, and parses each track's
// <name> and <t> transforms.
// ===========================================================================
TBool ReanimLoadCompiled(const char* aPakPath, ReanimDefinition& outDefinition)
{
    if (!gPak)
        return EFalse;

    TInt fileSize = gPak->GetFileSize(aPakPath);
    if (fileSize <= 0)
        return EFalse;

    // Read file from PAK into a heap buffer (+1 for NUL terminator).
    char* xmlBuf = (char*)User::Alloc(fileSize + 1);
    if (!xmlBuf)
        return EFalse;
    TPtr8 buf((unsigned char*)xmlBuf, fileSize);
    if (!gPak->ReadFile(aPakPath, buf))
    {
        User::Free(xmlBuf);
        return EFalse;
    }
    xmlBuf[fileSize] = '\0';
    TInt xmlLen = fileSize;

    // --- Parse <fps> ---
    {
        const char* fpsStart = NULL;
        TInt fpsLen = 0;
        const char* fpsAfter = NULL;
        if (FindElement(xmlBuf, xmlLen, "fps", xmlBuf, &fpsStart, &fpsLen, &fpsAfter)
            && fpsStart && fpsLen > 0)
        {
            char tmp[32];
            TInt n = (fpsLen < 31) ? fpsLen : 31;
            memcpy(tmp, fpsStart, n);
            tmp[n] = '\0';
            outDefinition.mFPS = (float)atof(tmp);
        }
        else
        {
            outDefinition.mFPS = 12.0f;
        }
    }

    // --- Count <track> elements by iterating FindElement ---
    TInt trackCount = 0;
    {
        const char* search = xmlBuf;
        const char* tStart = NULL;
        TInt tLen = 0;
        const char* tAfter = NULL;
        while (FindElement(xmlBuf, xmlLen, "track", search, &tStart, &tLen, &tAfter))
        {
            trackCount++;
            // Defensive: ensure forward progress even on a malformed match.
            if (tAfter > search)
                search = tAfter;
            else
                search = tAfter + 1;
        }
    }

    if (trackCount <= 0 || trackCount > 500)
    {
        User::Free(xmlBuf);
        return EFalse;
    }

    outDefinition.mTrackCount = trackCount;
    outDefinition.mTracks = new ReanimTrack[trackCount];
    // Zero-init the track array so mName/mTransforms are NULL if a track fails.
    memset(outDefinition.mTracks, 0, sizeof(ReanimTrack) * trackCount);

    // --- Parse each <track> ---
    {
        const char* search = xmlBuf;
        for (TInt i = 0; i < trackCount; i++)
        {
            const char* tStart = NULL;
            TInt tLen = 0;
            const char* tAfter = NULL;
            if (!FindElement(xmlBuf, xmlLen, "track", search, &tStart, &tLen, &tAfter))
            {
                // Should not happen (we counted this many), but stay safe.
                outDefinition.mTracks[i].mName = "";
                outDefinition.mTracks[i].mTransformCount = 0;
                outDefinition.mTracks[i].mTransforms = NULL;
                break;
            }
            if (tAfter > search)
                search = tAfter;
            else
                search = tAfter + 1;

            const char* trackBuf = tStart;
            TInt trackLen = tLen;

            // <name>
            outDefinition.mTracks[i].mName = ParseString(trackBuf, trackLen, "name");

            // Count <t> transforms within this track.
            TInt tCount = 0;
            {
                const char* ts = trackBuf;
                const char* eStart = NULL;
                TInt eLen = 0;
                const char* eAfter = NULL;
                while (FindElement(trackBuf, trackLen, "t", ts, &eStart, &eLen, &eAfter))
                {
                    tCount++;
                    if (eAfter > ts)
                        ts = eAfter;
                    else
                        ts = eAfter + 1;
                }
            }

            outDefinition.mTracks[i].mTransformCount = tCount;
            outDefinition.mTracks[i].mTransforms = NULL;

            if (tCount > 0)
            {
                outDefinition.mTracks[i].mTransforms = new ReanimTransform[tCount];
                // new[] runs ReanimTransform's ctor (sets numeric defaults +
                // mImage=NULL, mFontName="", mText=""). Do NOT memset -- that
                // would clobber the ctor work.

                const char* ts = trackBuf;
                for (TInt j = 0; j < tCount; j++)
                {
                    const char* eStart = NULL;
                    TInt eLen = 0;
                    const char* eAfter = NULL;
                    if (!FindElement(trackBuf, trackLen, "t", ts, &eStart, &eLen, &eAfter))
                    {
                        // Fill remainder with defaults and stop.
                        for (TInt k = j; k < tCount; k++)
                        {
                            ReanimTransform& tk = outDefinition.mTracks[i].mTransforms[k];
                            tk.mTransX = 0; tk.mTransY = 0;
                            tk.mSkewX = 0;  tk.mSkewY = 0;
                            tk.mScaleX = 1; tk.mScaleY = 1;
                            tk.mFrame = 0;  tk.mAlpha = 255;
                            tk.mImage = NULL;
                            tk.mFontName = "";
                            tk.mText = "";
                        }
                        break;
                    }
                    if (eAfter > ts)
                        ts = eAfter;
                    else
                        ts = eAfter + 1;

                    const char* tBuf = eStart;
                    TInt tBufLen = eLen;
                    ReanimTransform& t = outDefinition.mTracks[i].mTransforms[j];

                    // Defaults (ctor already set these, but be explicit so
                    // missing fields are deterministic).
                    t.mTransX = 0; t.mTransY = 0;
                    t.mSkewX = 0;  t.mSkewY = 0;
                    t.mScaleX = 1; t.mScaleY = 1;
                    t.mFrame = 0;  t.mAlpha = 255;
                    t.mImage = NULL;
                    t.mFontName = "";
                    t.mText = "";

                    // Floats
                    float v;
                    v = ParseFloat(tBuf, tBufLen, "x");  if (v != KFieldNotFound) t.mTransX = v;
                    v = ParseFloat(tBuf, tBufLen, "y");  if (v != KFieldNotFound) t.mTransY = v;
                    v = ParseFloat(tBuf, tBufLen, "kx"); if (v != KFieldNotFound) t.mSkewX = v;
                    v = ParseFloat(tBuf, tBufLen, "ky"); if (v != KFieldNotFound) t.mSkewY = v;
                    v = ParseFloat(tBuf, tBufLen, "sx"); if (v != KFieldNotFound) t.mScaleX = v;
                    v = ParseFloat(tBuf, tBufLen, "sy"); if (v != KFieldNotFound) t.mScaleY = v;
                    v = ParseFloat(tBuf, tBufLen, "f");  if (v != KFieldNotFound) t.mFrame = v;
                    v = ParseFloat(tBuf, tBufLen, "a");  if (v != KFieldNotFound) t.mAlpha = v;

                    // Image: <i>NAME</i> -- load via ResourceManager.
                    // Treat "NULL" / empty as no image (upstream convention).
                    const char* imgName = ParseString(tBuf, tBufLen, "i");
                    if (imgName && imgName[0] != '\0' &&
                        strcmp(imgName, "NULL") != 0)
                    {
                        if (gResourceManager)
                            t.mImage = gResourceManager->GetImage(imgName);
                        // ParseString allocated a char[] for this non-empty
                        // name; free it now that we've resolved the image ptr.
                        delete[] imgName;
                    }
                    else if (imgName && imgName[0] != '\0')
                    {
                        // It was the literal "NULL" string -- ParseString
                        // allocated it, so free it.
                        delete[] imgName;
                    }
                    // else: imgName == "" (static literal) -- nothing to free.

                    // Font + text (owned by the transform, freed by dtor).
                    t.mFontName = ParseString(tBuf, tBufLen, "font");
                    t.mText = ParseString(tBuf, tBufLen, "text");
                }
            }
        }
    }

    User::Free(xmlBuf);
    return ETrue;
}

// ===========================================================================
// ReanimFindTrack -- find a track by name (case-insensitive)
// ===========================================================================
ReanimTrack* ReanimFindTrack(ReanimDefinition& aDef, const char* aName)
{
    if (!aName)
        return NULL;
    for (int i = 0; i < aDef.mTrackCount; i++)
    {
        if (aDef.mTracks[i].mName)
        {
            const char* s1 = aDef.mTracks[i].mName;
            const char* s2 = aName;
            while (*s1 && *s2)
            {
                char c1 = *s1; if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                char c2 = *s2; if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                if (c1 != c2) break;
                s1++; s2++;
            }
            if (*s1 == '\0' && *s2 == '\0')
                return &aDef.mTracks[i];
        }
    }
    return NULL;
}

// ===========================================================================
// ReanimPlayer — lightweight reanimation runtime implementation.
//
// Interpolation logic verified host-side (test_interp.cpp, all checks
// passed) before porting: linear lerp between surrounding keyframes, clamp
// at both ends, single-transform tracks return that transform, empty tracks
// return EFalse.
// ===========================================================================

static float ReanimLerp(float a, float b, float t) { return a + (b - a) * t; }

ReanimPlayer::ReanimPlayer()
    : mDefinition(NULL)
    , mAnimTime(0.0f)
    , mAnimRate(1.0f)
    , mLoopType(LOOP_OFF)
    , mDead(EFalse)
    , mX(0.0f), mY(0.0f)
    , mScaleX(1.0f), mScaleY(1.0f)
    , mCoordScale(0.5f)
    , mAlpha(255.0f)
{
}

void ReanimPlayer::SetDefinition(ReanimDefinition* aDef)
{
    mDefinition = aDef;
    mAnimTime = 0.0f;
    mDead = EFalse;
}

float ReanimPlayer::GetDuration()
{
    if (!mDefinition || mDefinition->mTrackCount <= 0 ||
        mDefinition->mFPS <= 0.0001f)
        return 0.0f;
    float maxFrame = 0.0f;
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        ReanimTrack& tr = mDefinition->mTracks[i];
        for (int j = 0; j < tr.mTransformCount; j++)
        {
            if (tr.mTransforms[j].mFrame > maxFrame)
                maxFrame = tr.mTransforms[j].mFrame;
        }
    }
    return maxFrame / mDefinition->mFPS;
}

void ReanimPlayer::Update(float aDtSeconds)
{
    if (!mDefinition || mDead)
        return;
    mAnimTime += aDtSeconds * mAnimRate;
    float dur = GetDuration();
    if (dur > 0.0001f)
    {
        if (mLoopType == LOOP_ON)
        {
            // Wrap into [0, dur). Handles both forward and negative rates.
            while (mAnimTime >= dur) mAnimTime -= dur;
            while (mAnimTime < 0.0f) mAnimTime += dur;
        }
        else
        {
            if (mAnimTime >= dur)
            {
                mAnimTime = dur;
                mDead = ETrue;
            }
            else if (mAnimTime < 0.0f)
            {
                mAnimTime = 0.0f;
                mDead = ETrue;
            }
        }
    }
}

TBool ReanimPlayer::GetCurrentTransform(int aTrackIndex, ReanimTransform& aOut)
{
    if (!mDefinition || aTrackIndex < 0 ||
        aTrackIndex >= mDefinition->mTrackCount)
        return EFalse;
    ReanimTrack& track = mDefinition->mTracks[aTrackIndex];
    int n = track.mTransformCount;
    if (n <= 0 || !track.mTransforms)
        return EFalse;
    if (n == 1)
    {
        aOut = track.mTransforms[0];
        return ETrue;
    }

    float frame = mAnimTime * mDefinition->mFPS;

    // Clamp to the first / last keyframe.
    if (frame <= track.mTransforms[0].mFrame)
    {
        aOut = track.mTransforms[0];
        return ETrue;
    }
    if (frame >= track.mTransforms[n - 1].mFrame)
    {
        aOut = track.mTransforms[n - 1];
        return ETrue;
    }

    // Find the surrounding keyframe pair.
    int i = 0;
    for (i = 0; i < n - 1; i++)
    {
        if (frame >= track.mTransforms[i].mFrame &&
            frame < track.mTransforms[i + 1].mFrame)
            break;
    }
    ReanimTransform& a = track.mTransforms[i];
    ReanimTransform& b = track.mTransforms[i + 1];
    float span = b.mFrame - a.mFrame;
    float t = (span > 0.0001f) ? (frame - a.mFrame) / span : 0.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    aOut.mTransX = ReanimLerp(a.mTransX, b.mTransX, t);
    aOut.mTransY = ReanimLerp(a.mTransY, b.mTransY, t);
    aOut.mSkewX  = ReanimLerp(a.mSkewX,  b.mSkewX,  t);
    aOut.mSkewY  = ReanimLerp(a.mSkewY,  b.mSkewY,  t);
    aOut.mScaleX = ReanimLerp(a.mScaleX, b.mScaleX, t);
    aOut.mScaleY = ReanimLerp(a.mScaleY, b.mScaleY, t);
    aOut.mAlpha  = ReanimLerp(a.mAlpha,  b.mAlpha,  t);
    aOut.mFrame  = frame;
    // Image / font / text come from the active ("from") keyframe, matching
    // upstream Reanimator behaviour (discrete swaps happen on keyframes).
    aOut.mImage    = a.mImage;
    aOut.mFontName = a.mFontName;
    aOut.mText     = a.mText;
    return ETrue;
}

int ReanimPlayer::FindTrackIndex(const char* aName)
{
    if (!mDefinition || !aName)
        return -1;
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        const char* s1 = mDefinition->mTracks[i].mName;
        const char* s2 = aName;
        if (!s1) continue;
        TBool match = ETrue;
        while (*s1 && *s2)
        {
            char c1 = *s1; if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            char c2 = *s2; if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) { match = EFalse; break; }
            s1++; s2++;
        }
        if (match && *s1 == '\0' && *s2 == '\0')
            return i;
    }
    return -1;
}

void ReanimPlayer::Draw(Sexy::Graphics* g)
{
    if (!mDefinition || !g)
        return;
    // Reanim coordinate space is 800x600; mCoordScale (default 0.5) maps to
    // the port's 400x300 logical canvas. Per-track trans/scale are applied in
    // reanim space, then the whole thing is scaled to screen.
    for (int i = 0; i < mDefinition->mTrackCount; i++)
    {
        ReanimTrack& track = mDefinition->mTracks[i];
        if (track.mTransformCount <= 0)
            continue;
        ReanimTransform t;
        if (!GetCurrentTransform(i, t))
            continue;
        if (!t.mImage)
            continue;

        float sx = (mX + t.mTransX) * mCoordScale;
        float sy = (mY + t.mTransY) * mCoordScale;
        float sw = t.mImage->GetWidth()  * t.mScaleX * mScaleX * mCoordScale;
        float sh = t.mImage->GetHeight() * t.mScaleY * mScaleY * mCoordScale;
        if (sw < 1.0f || sh < 1.0f)
            continue;

        // NOTE: per-image alpha modulation (t.mAlpha) is not applied here --
        // the port's Graphics::SetColorizeImages is a stub and there is no
        // per-draw alpha API. Most menu elements are opaque, so this only
        // affects fade transitions. Revisit when Graphics gains alpha support.
        Sexy::MemoryImage* mem = static_cast<Sexy::MemoryImage*>(t.mImage);
        g->DrawImage(mem, (int)sx, (int)sy, (int)sw, (int)sh);
    }
}

