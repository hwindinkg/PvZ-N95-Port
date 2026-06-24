// stb_image implementation for Symbian (GCCE / C++03)
// Only PNG decoding from memory buffers is needed.
// No libc available on SDK - use new[]/delete[] instead of malloc/free
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO       // No file I/O (we load from PAK into memory)
#define STBI_NO_HDR         // No HDR support
#define STBI_NO_LINEAR      // No linear scaling
#define STBI_ONLY_PNG       // Only PNG for fastest compile

// Use Symbian heap instead of libc malloc/free
#include <e32std.h>
#define STBI_MALLOC(sz)     User::Alloc(sz)
#define STBI_FREE(p)        User::Free(p)
#define STBI_REALLOC(p,sz)  User::ReAlloc(p,sz)

#include "stb_image.h"
