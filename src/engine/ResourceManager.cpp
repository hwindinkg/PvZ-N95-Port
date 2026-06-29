#include "ResourceManager.h"
#include "MemoryImage.h"
#include "SystemFont.h"   // for GetFontThrow fallback
#include "PvZVfs.h"
#include "PakInterface.h"
#include "GLInterface.h"
#include <e32debug.h>
#include <fbs.h>
#include <imageconversion.h>

// No external image decoder available for this SDK.
// Images will be loaded as placeholders until PNG decoding is added.

// File logging
static void Log(const char* aMsg)
{
    TBuf8<256> buf;
    TPtrC8 msg8((const TUint8*)aMsg);
    buf.Copy(msg8);
    buf.Append('\n');
    RFs fs;
    if (fs.Connect() != KErrNone) return;
    RFile f;
    TInt err = f.Open(fs, _L("C:\\Data\\PvZ\\rmgr_log.txt"), EFileWrite|EFileShareAny);
    if (err == KErrNotFound)
        err = f.Create(fs, _L("C:\\Data\\PvZ\\rmgr_log.txt"), EFileWrite|EFileShareAny);
    if (err == KErrNone) {
        TInt pos = 0;
        f.Seek(ESeekEnd, pos);
        f.Write(buf);
        f.Close();
    }
    fs.Close();
}

ResourceManager* gResourceManager = NULL;

ResourceManager::ResourceManager()
    : iImageCount(0)
    , iFailed(false)
{
}

ResourceManager::~ResourceManager()
{
    for (TInt i = 0; i < iImageCount; i++)
    {
        delete[] iImageCache[i].iName;
        delete iImageCache[i].iImage;
    }
    iImageCount = 0;
}

bool ResourceManager::ParseResourcesFile(const char* path)
{
    Log("ResourceManager::ParseResourcesFile");
    Log(path);
    // For now, we don't parse XML - we handle direct image loading
    return true;
}

void ResourceManager::StartLoadResources(const char* group)
{
    Log("StartLoadResources");
    Log(group);
}

bool ResourceManager::HadError()
{
    return iFailed;
}

int ResourceManager::GetNumResources(const char* group)
{
    return 0;
}

Sexy::Image* ResourceManager::GetImage(const char* name)
{
    if (!name) return NULL;

    TInt idx = FindImageIndex(name);
    if (idx >= 0)
        return iImageCache[idx].iImage;

    // On-demand loading: try to load from PAK using resource name mapping
    Sexy::Image* loaded = LoadImageByResName(name);
    if (loaded)
    {
        AddImage(name, loaded);
        return loaded;
    }

    return NULL;
}

Sexy::Image* ResourceManager::GetImageThrow(const char* name)
{
    Sexy::Image* img = GetImage(name);
    // Note: doesn't actually throw (exceptions may be disabled),
    // just returns NULL. The Resources.cpp try/catch handles NULL gracefully.
    return img;
}

// Try to determine the PAK file path from a resource name like "IMAGE_TITLESCREEN"
// and load the image. Returns NULL if no mapping exists or file not in PAK.
Sexy::Image* ResourceManager::LoadImageByResName(const char* aResName)
{
    if (!aResName || !gPak)
        return NULL;
    Log("LoadImageByResName: ");
    Log(aResName);

    // Build a lowercase stem from the resource name.
    //
    // [Stage-1 fix] IMAGE_REANIM_* assets live in the PAK under
    // "reanim/<CamelCase>.png|jpg" -- e.g. IMAGE_REANIM_SELECTORSCREEN_BG ->
    // reanim/SelectorScreen_BG.jpg. Stripping only "IMAGE_" would leave the
    // stem "REANIM_SELECTORSCREEN_BG" -> "reanim_selectorscreen_bg", which
    // never matches the real "SelectorScreen_BG" filename. So strip the full
    // "IMAGE_REANIM_" prefix first, leaving "SELECTORSCREEN_BG" ->
    // "selectorscreen_bg", which the case-insensitive PAK matches against
    // "reanim/SelectorScreen_BG.jpg". Other IMAGE_* assets still strip just
    // "IMAGE_".
    const char* stemSrc = aResName;
    static const char* kReanimPrefix = "IMAGE_REANIM_";
    int reanimPrefixLen = (int)strlen(kReanimPrefix);
    static const char* kImgPrefix = "IMAGE_";
    int imgPrefixLen = (int)strlen(kImgPrefix);
    if (strncmp(aResName, kReanimPrefix, reanimPrefixLen) == 0)
        stemSrc = aResName + reanimPrefixLen;
    else if (strncmp(aResName, kImgPrefix, imgPrefixLen) == 0)
        stemSrc = aResName + imgPrefixLen;
    if (!*stemSrc)
        return NULL;

    char stem[200];
    {
        char* d = stem;
        const char* s2 = stemSrc;
        int w = 0;
        while (*s2 && w < 199)
        {
            char c = *s2;
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A'); // tolower
            *d++ = c; w++; s2++;
        }
        *d = 0;
    }

    // The port previously hardcoded "images/<stem>.png". Real PvZ assets are a
    // mix: photos like the title screen are .jpg, sprites are .png, some packs
    // store compiled "._" images or flat paths. Probe a matrix of
    // {prefix} x {extension} and load the first one that exists in the PAK.
    // (ICL's CImageDecoder auto-detects JPEG/PNG/GIF from the data, so the
    // extension only needs to match what is actually stored in the PAK.)
    // [M4 fix] Try .png BEFORE .jpg. PNG supports alpha transparency (needed
    // for logos like IMAGE_PVZ_LOGO which have transparent backgrounds).
    // JPEG has no alpha channel -- loading pvz_logo.jpg instead of pvz_logo.png
    // gives the logo a black background. If the PAK has both, .png wins.
    // [M4 reanim fix] IMAGE_REANIM_SELECTORSCREEN_* assets are in the PAK
    // under 'reanim/' prefix (not 'images/'). Also try 'reanim/' and
    // 'images/reloaded/mainmenu/' prefixes. PAK stores CamelCase names
    // (e.g. 'reanim\SelectorScreen_BG.jpg') but GetEntryIndex is
    // case-insensitive + normalises path separators, so lowercase stem works.
    static const char* kPrefixes[] = {
        "images/", "reanim/", "images/reloaded/mainmenu/", "", "IMAGES/", "data/images/"
    };
    static const char* kExts[]     = { ".png", ".jpg", ".jpeg", ".gif", "._", "" };
    // [Session-13] Alpha mask prefixes: upstream ImageLib looks for
    // "_<filename>" and "<filename>_" as alpha mask images.
    // e.g. "reanim/selectorscreen_bg_center.jpg" → alpha mask at
    // "reanim/_selectorscreen_bg_center.jpg" or "reanim/selectorscreen_bg_center_.jpg"
    // The alpha mask's blue channel becomes the alpha channel of the main image.
    const int nPre = (int)(sizeof(kPrefixes)/sizeof(kPrefixes[0]));
    const int nExt = (int)(sizeof(kExts)/sizeof(kExts[0]));

    char path[256];
    for (int pi = 0; pi < nPre; pi++)
    {
        for (int ei = 0; ei < nExt; ei++)
        {
            // path = prefix + stem + ext (guard against overflow)
            int pl = (int)strlen(kPrefixes[pi]);
            int sl = (int)strlen(stem);
            int xl = (int)strlen(kExts[ei]);
            if (pl + sl + xl >= (int)sizeof(path))
                continue;
            strcpy(path, kPrefixes[pi]);
            strcpy(path + pl, stem);
            strcpy(path + pl + sl, kExts[ei]);

            TInt fsize = gPak->GetFileSize(path);
            if (fsize > 0)
            {
                Log("  FOUND in PAK: ");
                Log(path);
                Sexy::Image* img = LoadImageFromPak("", path);
                if (img)
                {
                    // [Session-13] Compose alpha from "_<stem>" or "<stem>_" image.
                    // Upstream ImageLib looks for alpha mask images named
                    // "_<filename>" or "<filename>_". The alpha mask's blue
                    // channel becomes the alpha channel of the main image.
                    // This is how JPEG images (opaque, no alpha) get transparency.
                    Sexy::MemoryImage* mem = static_cast<Sexy::MemoryImage*>(img);
                    for (int api = 0; api < nPre; api++)
                    {
                        for (int aei = 0; aei < nExt; aei++)
                        {
                            char alphaPath[256];
                            int apl = (int)strlen(kPrefixes[api]);
                            int asl = (int)strlen(stem);
                            int axl = (int)strlen(kExts[aei]);
                            // Try "_<stem>.<ext>"
                            if (apl + 1 + asl + axl < (int)sizeof(alphaPath))
                            {
                                strcpy(alphaPath, kPrefixes[api]);
                                alphaPath[apl] = '_';
                                strcpy(alphaPath + apl + 1, stem);
                                strcpy(alphaPath + apl + 1 + asl, kExts[aei]);
                                TInt aSize = gPak->GetFileSize(alphaPath);
                                if (aSize > 0)
                                {
                                    Log("  ALPHA MASK FOUND: ");
                                    Log(alphaPath);
                                    Sexy::Image* alphaImg = LoadImageFromPak("", alphaPath);
                                    if (alphaImg)
                                    {
                                        Sexy::MemoryImage* aMem = static_cast<Sexy::MemoryImage*>(alphaImg);
                                        unsigned int* dstBits = (unsigned int*)mem->GetBits();
                                        unsigned int* srcBits = (unsigned int*)aMem->GetBits();
                                        int pixels = mem->GetWidth() * mem->GetHeight();
                                        int aPixels = aMem->GetWidth() * aMem->GetHeight();
                                        if (pixels == aPixels && dstBits && srcBits)
                                        {
                                            for (int i = 0; i < pixels; i++)
                                                dstBits[i] = (dstBits[i] & 0x00FFFFFF) | ((srcBits[i] & 0xFF) << 24);
                                            Log("  (alpha composed)");
                                        }
                                        delete alphaImg;
                                        goto alphaDone;
                                    }
                                }
                            }
                            // Try "<stem>_.<ext>"
                            if (apl + asl + 1 + axl < (int)sizeof(alphaPath))
                            {
                                strcpy(alphaPath, kPrefixes[api]);
                                strcpy(alphaPath + apl, stem);
                                alphaPath[apl + asl] = '_';
                                strcpy(alphaPath + apl + asl + 1, kExts[aei]);
                                TInt aSize = gPak->GetFileSize(alphaPath);
                                if (aSize > 0)
                                {
                                    Log("  ALPHA MASK FOUND: ");
                                    Log(alphaPath);
                                    Sexy::Image* alphaImg = LoadImageFromPak("", alphaPath);
                                    if (alphaImg)
                                    {
                                        Sexy::MemoryImage* aMem = static_cast<Sexy::MemoryImage*>(alphaImg);
                                        unsigned int* dstBits = (unsigned int*)mem->GetBits();
                                        unsigned int* srcBits = (unsigned int*)aMem->GetBits();
                                        int pixels = mem->GetWidth() * mem->GetHeight();
                                        int aPixels = aMem->GetWidth() * aMem->GetHeight();
                                        if (pixels == aPixels && dstBits && srcBits)
                                        {
                                            for (int i = 0; i < pixels; i++)
                                                dstBits[i] = (dstBits[i] & 0x00FFFFFF) | ((srcBits[i] & 0xFF) << 24);
                                            Log("  (alpha composed)");
                                        }
                                        delete alphaImg;
                                        goto alphaDone;
                                    }
                                }
                            }
                        }
                    }
                    alphaDone:;

                    // [M4 fix] Apply colorkey for logo images stored as JPEG
                    if (strstr(aResName, "LOGO") != NULL &&
                        strstr(aResName, "POPCAP") == NULL)
                    {
                        mem->ApplyColorKey(0x00000000, 8);
                        Log("  (colorkey applied: black -> transparent)");
                    }
                    return img;
                }
                // Found the file but decode failed -- keep probing other
                // candidates rather than giving up entirely.
                Log("  (decode failed, continuing probe)");
            }
        }
    }

    Log("  image not found in PAK (tried images/ & flat, .jpg/.png/.gif/._/.jpeg)");
    Log("  stem was: ");
    Log(stem);
    return NULL;
}

_Font* ResourceManager::GetFontThrow(const char* name)
{
    Log("GetFontThrow -> SystemFont fallback: ");
    Log(name);
    // [M4 #4 fix] Return the shared SystemFont instance (8x8 bitmap font with
    // hardcoded ASCII glyphs). This is NOT a 1:1 port of upstream ImageFont
    // (which loads .dat font description files from PAK for per-char metrics +
    // kerning + multi-layer rendering). The PAK does not contain font .dat
    // files (rmgr_log shows FONT_IMAGE_HOUSEOFTERROR28 "not found"). SystemFont
    // provides visible text as a functional fallback.
    //
    // All FONT_* globals (FONT_DWARVENTODCRAFT18GREENINSET, FONT_BRIANNETOD16,
    // FONT_PICO129, etc.) will point to the same SystemFont instance. They
    // differ visually only in the upstream's per-font image + metrics; here
    // they all render as the same 8x8 monospace.
    // SystemFont is in namespace Sexy; GetFontThrow returns _Font* (= Sexy::Font*).
    return Sexy::SystemFont::Get();
}

intptr_t ResourceManager::GetSoundThrow(const char* name)
{
    Log("GetSoundThrow (stub) ");
    Log(name);
    return 0;
}

void ResourceManager::DeleteResources(const char* group)
{
    // Simplified: just clear all
    for (TInt i = 0; i < iImageCount; i++)
    {
        delete[] iImageCache[i].iName;
        delete iImageCache[i].iImage;
    }
    iImageCount = 0;
}

void ResourceManager::DeleteImage(const char* name)
{
    TInt idx = FindImageIndex(name);
    if (idx >= 0)
    {
        delete[] iImageCache[idx].iName;
        delete iImageCache[idx].iImage;
        // Shift remaining entries
        for (TInt j = idx; j < iImageCount - 1; j++)
            iImageCache[j] = iImageCache[j + 1];
        iImageCount--;
    }
}

bool ResourceManager::LoadResources(const char* group)
{
    Log("LoadResources ");
    Log(group);
    return true;
}

TInt ResourceManager::FindImageIndex(const char* name) const
{
    if (!name) return -1;
    for (TInt i = 0; i < iImageCount; i++)
    {
        if (iImageCache[i].iName && strcmp(iImageCache[i].iName, name) == 0)
            return i;
    }
    return -1;
}

bool ResourceManager::AddImage(const char* name, Sexy::Image* img)
{
    if (!name || !img || iImageCount >= KMaxResources)
        return false;

    TInt nameLen = strlen(name);
    iImageCache[iImageCount].iName = new char[nameLen + 1];
    if (!iImageCache[iImageCount].iName)
        return false;
    strcpy(iImageCache[iImageCount].iName, name);
    iImageCache[iImageCount].iImage = img;
    iImageCount++;
    return true;
}

// ---------------------------------------------------------------------------
// PNG / JPEG decoding via the Image Conversion Library (ICL).
// Decodes an in-memory image buffer into a 32-bit ARGB (0xAARRGGBB) pixel
// array compatible with Sexy::MemoryImage and GLInterface texture upload.
// (Replaces the old purple checkerboard placeholder.)
// ---------------------------------------------------------------------------
namespace {

// Helper: log "<label> WxH" using the narrow Log().
static void LogSize(const char* aLabel, TInt aW, TInt aH)
    {
    TBuf8<96> b;
    b.Copy(TPtrC8((const TUint8*)aLabel));
    b.AppendFormat(_L8(" %dx%d"), aW, aH);
    char tmp[100];
    TInt n = b.Length(); if (n > 99) n = 99;
    for (TInt i = 0; i < n; i++) tmp[i] = (char)b[i];
    tmp[n] = 0;
    Log(tmp);
    }

// Active-object wrapper around CImageDecoder::Convert.
// ICL decode is asynchronous and is serviced PARTLY by active objects running
// in THIS thread. Blocking the UI thread with User::WaitForRequest deadlocks:
// the codec plug-in's own AOs can never run, so the request never completes,
// and because the UI thread owns the window server connection the ENTIRE phone
// UI freezes (battery-pull required). The correct pattern is a CActive driving
// a nested CActiveSchedulerWait so the scheduler keeps pumping until decode
// completes. A guard bounds the progressive-decode underflow loop.
class CDecodeWaiter : public CActive
    {
public:
    // aMask may be NULL (opaque image, single-bitmap Convert) or point to an
    // alpha/transparency mask bitmap (two-bitmap Convert overload).
    CDecodeWaiter(CImageDecoder& aDecoder, CFbsBitmap& aBmp, CFbsBitmap* aMask)
        : CActive(EPriorityStandard), iDecoder(aDecoder), iBmp(aBmp), iMask(aMask),
          iErr(KErrNone), iGuard(0)
        { CActiveScheduler::Add(this); }
    ~CDecodeWaiter() { Cancel(); }

    // Runs the whole decode (Convert + any ContinueConvert passes) and blocks
    // via a nested scheduler until done. Returns the final error code.
    TInt RunDecode()
        {
        if (iMask)
            iDecoder.Convert(&iStatus, iBmp, *iMask);   // color + alpha mask
        else
            iDecoder.Convert(&iStatus, iBmp);           // opaque
        SetActive();
        iWait.Start();
        return iErr;
        }

private:
    void RunL()
        {
        if (iStatus == KErrUnderflow && iGuard < 128)
            {
            iGuard++;
            iDecoder.ContinueConvert(&iStatus);  // progressive/interlaced pass
            SetActive();
            return;            // keep pumping; do NOT stop the wait yet
            }
        iErr = iStatus.Int();
        if (iWait.IsStarted()) iWait.AsyncStop();
        }
    void DoCancel()
        {
        iDecoder.Cancel();
        if (iWait.IsStarted()) iWait.AsyncStop();
        }

private:
    CImageDecoder& iDecoder;
    CFbsBitmap&    iBmp;
    CFbsBitmap*    iMask;
    CActiveSchedulerWait iWait;
    TInt iErr;
    TInt iGuard;
    };

// Decode into a COLOR bitmap (+ optional alpha MASK).
//
// [N95 PNG fix] The N95's ICL PNG codec returns KErrNotSupported(-5) when asked
// to Convert directly into a 32-bit EColor16MA destination (JPEG works, PNG does
// not -- 106/106 PNGs failed with -5 while JPGs decoded fine). The portable
// pattern is to decode into a 24-bit EColor16M color bitmap (broadly supported)
// plus, when the frame has transparency, a separate grayscale alpha mask, then
// recombine into ARGB in DecodeImageToArgb().
//
// Sets aHasMask = ETrue iff a mask bitmap was created & filled. Leaves on error.
void DecodeToBitmapL(RFs& aFs, const TUint8* aData, TInt aLen,
                     CFbsBitmap& aBmp, CFbsBitmap& aMask, TBool& aHasMask)
    {
    Log("  [dec] DataNewL");
    TPtrC8 dataPtr(aData, aLen);
    CImageDecoder* decoder = CImageDecoder::DataNewL(aFs, dataPtr);
    CleanupStack::PushL(decoder);

    TFrameInfo frameInfo = decoder->FrameInfo(0);
    TSize sz = frameInfo.iOverallSizeInPixels;
    LogSize("  [dec] frame", sz.iWidth, sz.iHeight);

    const TBool transparency =
        (frameInfo.iFlags & TFrameInfo::ETransparencyPossible) ? ETrue : EFalse;
    const TBool fullAlpha =
        (frameInfo.iFlags & TFrameInfo::EAlphaChannel) ? ETrue : EFalse;

    // 24-bit color bitmap -- the codec CAN convert into this (unlike EColor16MA).
    User::LeaveIfError(aBmp.Create(sz, EColor16M));

    aHasMask = EFalse;
    if (transparency)
        {
        // 8-bit alpha mask for a real alpha channel; 1-bit for color-key/palette
        // transparency. GetScanLine(EGray256) later normalizes both to 0..255.
        TDisplayMode maskMode = fullAlpha ? EGray256 : EGray2;
        User::LeaveIfError(aMask.Create(sz, maskMode));
        aHasMask = ETrue;
        }
    Log(aHasMask ? "  [dec] EColor16M + alpha mask; Convert via CActive"
                 : "  [dec] EColor16M opaque; Convert via CActive");

    CDecodeWaiter* waiter =
        new (ELeave) CDecodeWaiter(*decoder, aBmp, aHasMask ? &aMask : NULL);
    CleanupStack::PushL(waiter);
    TInt err = waiter->RunDecode();
    CleanupStack::PopAndDestroy(waiter);

    LogSize("  [dec] convert done err", err, 0);
    User::LeaveIfError(err);

    CleanupStack::PopAndDestroy(decoder);
    }

// Returns a new[]-allocated ARGB (0xAARRGGBB) buffer, or NULL on failure.
// Caller frees with delete[] (or hands ownership to MemoryImage::SetBits).
unsigned char* DecodeImageToArgb(const TUint8* aData, TInt aLen, TInt& aW, TInt& aH)
    {
    aW = 0;
    aH = 0;
    if (!aData || aLen <= 0)
        return NULL;

    RFs fs;
    if (fs.Connect() != KErrNone)
        return NULL;

    CFbsBitmap* bmp  = new CFbsBitmap();
    CFbsBitmap* mask = new CFbsBitmap();
    if (!bmp || !mask)
        {
        delete bmp;
        delete mask;
        fs.Close();
        return NULL;
        }

    TBool hasMask = EFalse;
    TRAPD(err, DecodeToBitmapL(fs, aData, aLen, *bmp, *mask, hasMask));
    if (err != KErrNone)
        {
        delete bmp;
        delete mask;
        fs.Close();
        return NULL;
        }

    TSize sz = bmp->SizeInPixels();
    TInt w = sz.iWidth;
    TInt h = sz.iHeight;
    unsigned char* out = NULL;

    if (w > 0 && h > 0)
        {
        out = new unsigned char[w * h * 4];
        if (out)
            {
            TInt x, y;
            // 1) Read RGB from the 24-bit color bitmap, requesting EColor16MA so
            //    GetScanLine packs 0xAARRGGBB. The source has no alpha, so we
            //    FORCE every alpha byte to 0xFF (opaque) -- never trust the
            //    converter to fill it.  In a little-endian 0xAARRGGBB word the
            //    alpha byte is offset +3.
            for (y = 0; y < h; y++)
                {
                TPtr8 row((TUint8*)(out + y * w * 4), 0, w * 4);
                bmp->GetScanLine(row, TPoint(0, y), w, EColor16MA);
                for (x = 0; x < w; x++)
                    out[(y * w + x) * 4 + 3] = 0xFF;
                }

            // 2) If we decoded a mask, overlay its 0..255 alpha per pixel.
            if (hasMask)
                {
                unsigned char* mrow = new unsigned char[w];
                if (mrow)
                    {
                    for (y = 0; y < h; y++)
                        {
                        TPtr8 m((TUint8*)mrow, 0, w);
                        // EGray256 read => 1 byte/pixel, 0=transparent..255=opaque.
                        // (A 1-bit EGray2 mask is up-converted to 0/255 here too.)
                        mask->GetScanLine(m, TPoint(0, y), w, EGray256);
                        for (x = 0; x < w; x++)
                            out[(y * w + x) * 4 + 3] = mrow[x];
                        }
                    delete[] mrow;
                    }
                }

            aW = w;
            aH = h;
            }
        }

    delete bmp;
    delete mask;
    fs.Close();
    return out;
    }

} // anonymous namespace

Sexy::Image* ResourceManager::LoadImageFromPak(const char* aPakPath, const char* aFileName)
{
    Log("LoadImageFromPak: ");
    Log(aFileName);

    if (!gPak || !aFileName)
        return NULL;

    // Allocate buffer for the file
    TInt fsize = gPak->GetFileSize(aFileName);
    if (fsize <= 0)
    {
        Log("File not found in PAK");
        return NULL;
    }

    // Read file from PAK
    void* buf = NULL;
    TInt size = 0;
    if (!gPak->ReadFile(aFileName, &buf, &size))
    {
        Log("ReadFile failed");
        return NULL;
    }

    if (!buf || size <= 0)
    {
        Log("Empty file data");
        return NULL;
    }

    // ---- Decode PNG/JPEG bytes via the Image Conversion Library (ICL) ----
    TInt decW = 0;
    TInt decH = 0;
    unsigned char* argb = DecodeImageToArgb((const TUint8*)buf, size, decW, decH);

    delete[] (char*)buf;
    buf = NULL;

    if (!argb || decW <= 0 || decH <= 0)
    {
        Log("  ICL decode failed");
        if (argb)
            delete[] argb;
        return NULL;
    }

    Sexy::MemoryImage* img = new Sexy::MemoryImage();
    if (!img)
    {
        delete[] argb;
        return NULL;
    }

    img->SetSize(decW, decH);
    // EColor16MA scanlines are 0xAARRGGBB == MemoryImage's native ARGB layout.
    // own=true: MemoryImage takes ownership and frees the buffer with delete[].
    img->SetBits(argb, decW * decH * 4, true);

    Log("  decoded OK");
    return img;
}