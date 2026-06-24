#ifndef __ENGINE_RESOURCEMANAGER_H__
#define __ENGINE_RESOURCEMANAGER_H__

#include <e32base.h>
#include <string.h>
#include "Common.h"

// intptr_t isn't available in Symbian GCCE stdapis.
// On ARM 32-bit, a pointer fits in TInt32.
#ifndef intptr_t
#define intptr_t TInt32
#endif

namespace Sexy { class Image; class MemoryImage; }
class _Font;  // opaque pointer (no definition needed)

// Exception for resource loading (used by Resources.cpp try/catch blocks)
struct ResourceManagerException
{
    // Empty struct - we don't throw, but need the type for compilation
};

// Simple map entry for image resources
struct SImageEntry
{
    char*         iName;     // resource name (e.g. "IMAGE_TITLESCREEN")
    Sexy::Image*  iImage;    // loaded image
};

// Max resources we can track
const TInt KMaxResources = 1024;

class ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();

    // Parse resources.xml from PAK (stub for now - we load images directly)
    bool ParseResourcesFile(const char* path);

    // Load resource group (stub)
    void StartLoadResources(const char* group);

    // Check for errors
    bool HadError();

    // Get number of resources in group
    int GetNumResources(const char* group);

    // Image access
    Sexy::Image* GetImage(const char* name);
    Sexy::Image* GetImageThrow(const char* name);

    // Font access (stubs - always return NULL)
    _Font* GetFontThrow(const char* name);

    // Sound access (stub - always returns 0)
    intptr_t GetSoundThrow(const char* name);

    // DeleteResources
    void DeleteResources(const char* group);
    void DeleteImage(const char* name);

    // Load/unload (simplified)
    bool LoadResources(const char* group);

    // Direct image loading from PAK
    Sexy::Image* LoadImageFromPak(const char* aPakPath, const char* aFileName);

    // Try to load image by resource name mapping (e.g. "IMAGE_TITLESCREEN")
    Sexy::Image* LoadImageByResName(const char* aResName);

private:
    SImageEntry  iImageCache[KMaxResources];
    TInt         iImageCount;
    bool         iFailed;

    TInt FindImageIndex(const char* name) const;
    bool AddImage(const char* name, Sexy::Image* img);
};

extern ResourceManager* gResourceManager;

#endif // __ENGINE_RESOURCEMANAGER_H__
