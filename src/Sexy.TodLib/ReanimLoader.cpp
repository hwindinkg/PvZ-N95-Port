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
 */
#include "ReanimLoader.h"
#include "../engine/PvZVfs.h"
#include "../engine/PakInterface.h"
#include "../engine/Image.h"
#include "../engine/ResourceManager.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

// ===========================================================================
// Simple XML tag finder — finds <tag>content</tag> in a buffer
// Returns content start/end, or NULL if not found.
// ===========================================================================
static const char* FindTag(const char* buf, int bufLen, const char* tag,
                           const char* searchFrom, int& outContentLen)
{
    char openTag[32];
    char closeTag[32];
    // Build "<tag>" and "</tag>"
    openTag[0] = '<';
    int tagLen = strlen(tag);
    strcpy(openTag + 1, tag);
    openTag[1 + tagLen] = '>';
    openTag[2 + tagLen] = '\0';

    closeTag[0] = '<';
    closeTag[1] = '/';
    strcpy(closeTag + 2, tag);
    closeTag[2 + tagLen] = '>';
    closeTag[3 + tagLen] = '\0';

    // Find openTag from searchFrom
    const char* start = NULL;
    int searchLen = bufLen - (int)(searchFrom - buf);
    if (searchLen <= 0) return NULL;

    for (int i = 0; i <= searchLen - (int)strlen(openTag); i++)
    {
        if (strncmp(searchFrom + i, openTag, strlen(openTag)) == 0)
        {
            start = searchFrom + i + strlen(openTag);
            break;
        }
    }
    if (!start) return NULL;

    // Find closeTag from start
    int remaining = bufLen - (int)(start - buf);
    for (int i = 0; i <= remaining - (int)strlen(closeTag); i++)
    {
        if (strncmp(start + i, closeTag, strlen(closeTag)) == 0)
        {
            outContentLen = i;
            return start;
        }
    }
    return NULL;
}

// Extract float from tag content
static float ParseFloatTag(const char* trackBuf, int trackLen, const char* tag)
{
    int contentLen = 0;
    const char* content = FindTag(trackBuf, trackLen, tag, trackBuf, contentLen);
    if (!content || contentLen <= 0) return -9999.0f; // sentinel = not found
    // Parse float
    char tmp[32];
    if (contentLen >= 32) contentLen = 31;
    strncpy(tmp, content, contentLen);
    tmp[contentLen] = '\0';
    return (float)atof(tmp);
}

// Extract string from tag content (allocates new char[])
static char* ParseStringTag(const char* trackBuf, int trackLen, const char* tag)
{
    int contentLen = 0;
    const char* content = FindTag(trackBuf, trackLen, tag, trackBuf, contentLen);
    if (!content || contentLen <= 0) return (char*)"";
    char* str = new char[contentLen + 1];
    strncpy(str, content, contentLen);
    str[contentLen] = '\0';
    return str;
}

// ===========================================================================
// ReanimDefinition destructor
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
// ReanimLoadCompiled — load and parse a .reanim XML file from PAK
// ===========================================================================
TBool ReanimLoadCompiled(const char* aPakPath, ReanimDefinition& outDefinition)
{
    if (!gPak)
        return EFalse;

    TInt fileSize = gPak->GetFileSize(aPakPath);
    if (fileSize <= 0)
        return EFalse;

    // Read + XOR decrypt
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
    int xmlLen = fileSize;

    // Parse <fps>
    int fpsLen = 0;
    const char* fpsContent = FindTag(xmlBuf, xmlLen, "fps", xmlBuf, fpsLen);
    if (fpsContent && fpsLen > 0)
    {
        char tmp[32];
        if (fpsLen >= 32) fpsLen = 31;
        strncpy(tmp, fpsContent, fpsLen);
        tmp[fpsLen] = '\0';
        outDefinition.mFPS = (float)atof(tmp);
    }
    else
    {
        outDefinition.mFPS = 12.0f;
    }

    // Count <track> tags
    int trackCount = 0;
    const char* searchPos = xmlBuf;
    int dummyLen;
    while (true)
    {
        const char* trackStart = FindTag(xmlBuf, xmlLen, "track", searchPos, dummyLen);
        if (!trackStart) break;
        // Make sure it's <track> not </track>
        if (trackStart[-2] == '/') { searchPos = trackStart; continue; }
        trackCount++;
        // Advance past this track
        // Find </track>
        const char* trackEnd = FindTag(xmlBuf, xmlLen, "/track", trackStart, dummyLen);
        if (trackEnd) searchPos = trackEnd + 8; // past </track>
        else break;
    }

    if (trackCount <= 0 || trackCount > 500)
    {
        User::Free(xmlBuf);
        return EFalse;
    }

    outDefinition.mTrackCount = trackCount;
    outDefinition.mTracks = new ReanimTrack[trackCount];

    // Parse each <track>
    searchPos = xmlBuf;
    for (int i = 0; i < trackCount; i++)
    {
        // Find <track> content
        int trackContentLen = 0;
        const char* trackContent = FindTag(xmlBuf, xmlLen, "track", searchPos, trackContentLen);
        if (!trackContent || trackContentLen <= 0)
        {
            outDefinition.mTracks[i].mName = (char*)"";
            outDefinition.mTracks[i].mTransformCount = 0;
            outDefinition.mTracks[i].mTransforms = NULL;
            searchPos = trackContent + trackContentLen + 8;
            continue;
        }
        // Make sure not </track>
        if (trackContent[-2] == '/')
        {
            i--;
            searchPos = trackContent;
            continue;
        }

        const char* trackBuf = trackContent;
        int trackLen = trackContentLen;

        // Parse <name>
        outDefinition.mTracks[i].mName = ParseStringTag(trackBuf, trackLen, "name");

        // Count <t> tags within this track
        int tCount = 0;
        const char* tSearch = trackBuf;
        while (true)
        {
            int tLen = 0;
            const char* tContent = FindTag(trackBuf, trackLen, "t", tSearch, tLen);
            if (!tContent) break;
            // Make sure it's <t> not some other tag starting with t
            // Check the char after "t" in the open tag is ">" 
            if (tContent[-1] != '>') { tSearch = tContent; continue; }
            tCount++;
            tSearch = tContent + tLen + 4; // past </t>
        }

        outDefinition.mTracks[i].mTransformCount = tCount;
        outDefinition.mTracks[i].mTransforms = NULL;

        if (tCount > 0)
        {
            outDefinition.mTracks[i].mTransforms = new ReanimTransform[tCount];

            // Parse each <t>
            tSearch = trackBuf;
            for (int j = 0; j < tCount; j++)
            {
                int tLen = 0;
                const char* tContent = FindTag(trackBuf, trackLen, "t", tSearch, tLen);
                if (!tContent || tLen < 0) { tSearch++; j--; continue; }
                if (tContent[-1] != '>') { tSearch = tContent; j--; continue; }

                const char* tBuf = tContent;
                int tBufLen = tLen;
                ReanimTransform& t = outDefinition.mTracks[i].mTransforms[j];

                // Defaults
                t.mTransX = 0; t.mTransY = 0;
                t.mSkewX = 0; t.mSkewY = 0;
                t.mScaleX = 1; t.mScaleY = 1;
                t.mFrame = 0; t.mAlpha = 255;
                t.mImage = NULL;
                t.mFontName = (char*)"";
                t.mText = (char*)"";

                // Parse floats
                float v;
                v = ParseFloatTag(tBuf, tBufLen, "x");  if (v != -9999.0f) t.mTransX = v;
                v = ParseFloatTag(tBuf, tBufLen, "y");  if (v != -9999.0f) t.mTransY = v;
                v = ParseFloatTag(tBuf, tBufLen, "kx"); if (v != -9999.0f) t.mSkewX = v;
                v = ParseFloatTag(tBuf, tBufLen, "ky"); if (v != -9999.0f) t.mSkewY = v;
                v = ParseFloatTag(tBuf, tBufLen, "sx"); if (v != -9999.0f) t.mScaleX = v;
                v = ParseFloatTag(tBuf, tBufLen, "sy"); if (v != -9999.0f) t.mScaleY = v;
                v = ParseFloatTag(tBuf, tBufLen, "f");  if (v != -9999.0f) t.mFrame = v;
                v = ParseFloatTag(tBuf, tBufLen, "a");  if (v != -9999.0f) t.mAlpha = v;

                // Parse image name
                int imgLen = 0;
                const char* imgContent = FindTag(tBuf, tBufLen, "i", tBuf, imgLen);
                if (imgContent && imgLen > 0 && imgContent[-1] == '>')
                {
                    char* imgName = new char[imgLen + 1];
                    strncpy(imgName, imgContent, imgLen);
                    imgName[imgLen] = '\0';
                    // Load image via ResourceManager
                    if (gResourceManager)
                        t.mImage = gResourceManager->GetImage(imgName);
                    delete[] imgName;
                }

                // Parse font name
                t.mFontName = ParseStringTag(tBuf, tBufLen, "font");

                // Parse text
                t.mText = ParseStringTag(tBuf, tBufLen, "text");

                tSearch = tContent + tLen + 4; // past </t>
            }
        }

        // Advance searchPos past this </track>
        searchPos = trackContent + trackContentLen + 8;
    }

    User::Free(xmlBuf);
    return ETrue;
}

// ===========================================================================
// ReanimFindTrack — find a track by name (case-insensitive)
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
