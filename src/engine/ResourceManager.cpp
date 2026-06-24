#include "ResourceManager.h"
#include "MemoryImage.h"
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

    // Build candidate file paths from resource name.
    // Convention: "IMAGE_TITLESCREEN" -> "images/titlescreen.png"
    // Skip "IMAGE_" prefix, lowercase, replace spaces with nothing.
    const char* prefix = "IMAGE_";
    int prefixLen = strlen(prefix);
    int nameLen = strlen(aResName);

    if (nameLen <= prefixLen || strncmp(aResName, prefix, prefixLen) != 0)
        return NULL;

    // Extract the part after IMAGE_
    const char* stem = aResName + prefixLen;

    // Build path: "images/" + lowercase(stem) + ".png"
    char path[256];
    strcpy(path, "images/");

    char* dst = path + 7; // after "images/"
    const char* src = stem;
    int written = 7;
    while (*src && written < 245)
    {
        char c = *src;
        if (c >= 'A' && c <= 'Z')
            c += ('a' - 'A');  // tolower
        *dst++ = c;
        written++;
        src++;
    }
    strcpy(dst, ".png");

    Log("  trying path: ");
    Log(path);

    // Check if file exists in PAK
    TInt fsize = gPak->GetFileSize(path);
    if (fsize > 0)
    {
        Log("  FOUND in PAK, loading...");
        return LoadImageFromPak("", path);
    }

    // Try alternate: without the "images/" prefix (some PAKs use flat paths)
    Log("  not found, trying alternate paths...");

    // Try 2: lowercase name directly
    char path2[256];
    dst = path2;
    src = aResName;  // full resource name
    written = 0;
    while (*src && written < 250)
    {
        char c = *src;
        if (c >= 'A' && c <= 'Z')
            c += ('a' - 'A');
        *dst++ = c;
        written++;
        src++;
    }
    strcpy(dst, ".png");

    fsize = gPak->GetFileSize(path2);
    if (fsize > 0)
    {
        Log("  found alternate path");
        return LoadImageFromPak("", path2);
    }

    Log("  image not found in PAK");
    return NULL;
}

_Font* ResourceManager::GetFontThrow(const char* name)
{
    Log("GetFontThrow (stub) ");
    Log(name);
    return NULL;
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

// Drives the asynchronous ICL decode synchronously via a nested
// active-scheduler wait. The app's active scheduler is already running
// (installed by CAknAppUi before ConstructL), so a nested wait is safe.
class CIclDecodeWait : public CActive
    {
public:
    CIclDecodeWait() : CActive(CActive::EPriorityStandard)
        { CActiveScheduler::Add(this); }
    ~CIclDecodeWait()
        { Cancel(); }
    void WaitForCompletion()
        { iWait.Start(); }
public:
    CActiveSchedulerWait iWait;
protected:
    void RunL()
        { if (iWait.IsStarted()) iWait.AsyncStop(); }
    void DoCancel()
        { }
    };

void DecodeToBitmapL(RFs& aFs, const TUint8* aData, TInt aLen, CFbsBitmap& aBmp)
    {
    TPtrC8 dataPtr(aData, aLen);
    CImageDecoder* decoder = CImageDecoder::DataNewL(aFs, dataPtr);
    CleanupStack::PushL(decoder);

    TFrameInfo frameInfo = decoder->FrameInfo(0);
    TSize sz = frameInfo.iOverallSizeInPixels;
    User::LeaveIfError(aBmp.Create(sz, EColor16MA));

    CIclDecodeWait* waiter = new (ELeave) CIclDecodeWait();
    CleanupStack::PushL(waiter);

    decoder->Convert(&waiter->iStatus, aBmp);
    waiter->SetActive();
    waiter->WaitForCompletion();
    User::LeaveIfError(waiter->iStatus.Int());

    CleanupStack::PopAndDestroy(waiter);
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

    CFbsBitmap* bmp = new CFbsBitmap();
    if (!bmp)
        {
        fs.Close();
        return NULL;
        }

    TRAPD(err, DecodeToBitmapL(fs, aData, aLen, *bmp));
    if (err != KErrNone)
        {
        delete bmp;
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
            TInt y;
            for (y = 0; y < h; y++)
                {
                // EColor16MA scanline == 0xAARRGGBB per pixel, written
                // straight into the output row.
                TPtr8 row((TUint8*)(out + y * w * 4), 0, w * 4);
                bmp->GetScanLine(row, TPoint(0, y), w, EColor16MA);
                }
            aW = w;
            aH = h;
            }
        }

    delete bmp;
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