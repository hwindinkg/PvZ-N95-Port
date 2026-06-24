#ifndef COMMON_H
#define COMMON_H

#include <string>

// stl_stubs/string provides our std::string (uses _STL_STUBS_STRING guard).
// We provide a Sexy-specific typedef for use throughout the engine.
namespace Sexy {
    typedef std::string SexyString;
}

#include <e32def.h>
#include <e32cmn.h>
#include <e32std.h>
#include <e32base.h>

// ---------------------------------------------------------------------------
// Basic unsigned typedefs (C++03 / Symbian-safe)
// ---------------------------------------------------------------------------
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef long long      int64;

// Standard integer types (Symbian GCCE doesn't have <stdint.h>)
typedef signed   char      int8_t;
typedef signed   short     int16_t;
typedef signed   int       int32_t;
typedef signed   long long int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// ---------------------------------------------------------------------------
// Array-size helper (equivalent to LENGTH in PopCap code)
// ---------------------------------------------------------------------------
#ifndef LENGTH
#define LENGTH(arr)  (static_cast<int>(sizeof(arr) / sizeof((arr)[0])))
#endif

// ---------------------------------------------------------------------------
// MIN / MAX / CLAMP
// ---------------------------------------------------------------------------
#ifndef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(val, lo, hi)  (MIN(MAX((val), (lo)), (hi)))
#endif

// ---------------------------------------------------------------------------
// Debug assertion
// ---------------------------------------------------------------------------
#ifdef _DEBUG
    #define DebugAssert(cond, msg)                                              \
        do {                                                                    \
            if (!(cond)) {                                                      \
                RDebug::Print(_L("ASSERT FAILED: %s -- %s"), #cond, msg);       \
                User::Panic(_L("PvZAssert"), __LINE__);                         \
            }                                                                   \
        } while (0)
#else
    #define DebugAssert(cond, msg)                                              \
        do {                                                                    \
            if (!(cond)) {                                                      \
                RDebug::Print(_L("ASSERT FAILED (REL): %s -- %s"), #cond, msg); \
                User::Panic(_L("PvZAssert"), __LINE__);                         \
            }                                                                   \
        } while (0)
#endif

// ---------------------------------------------------------------------------
// Unused variable helper
// ---------------------------------------------------------------------------
#define UNUSED(x)  (void)(x)

// ---------------------------------------------------------------------------
// Byte-swap helpers (C++03, no constexpr)
// ---------------------------------------------------------------------------
inline ushort ByteSwap16(ushort v)
{
    return static_cast<ushort>((v >> 8) | (v << 8));
}

inline uint ByteSwap32(uint v)
{
    return ((v & 0x000000FFu) << 24) |
           ((v & 0x0000FF00u) <<  8) |
           ((v & 0x00FF0000u) >>  8) |
           ((v & 0xFF000000u) >> 24);
}

// Endian helpers (little-endian on Symbian ARM)
inline uint32_t ToLE32(uint32_t v)
{
    // ARM (N95) is little-endian; no swap needed.
    return v;
}

inline uint32_t FromLE32(uint32_t v)
{
    return v;
}

// ---------------------------------------------------------------------------
// PopCap-style string helpers (Symbian-compatible stubs)
// ---------------------------------------------------------------------------
namespace Sexy
{

inline std::string StrFormat(const char* fmt, ...)
{
    // Basic stub -- returns the format string as-is.
    // Replace with a real vsnprintf-based implementation if needed.
    (void)fmt;
    return std::string(fmt);
}

inline std::string VFormat(const char* fmt, va_list)
{
    (void)fmt;
    return std::string(fmt);
}

inline int StrFindNoCase(const char* theStr, const char* theFind)
{
    if (theStr == NULL || theFind == NULL)
        return -1;
    // Simple case-insensitive substring search.
    int slen = 0, flen = 0;
    while (theStr[slen] != 0) ++slen;
    while (theFind[flen] != 0) ++flen;
    for (int i = 0; i <= slen - flen; ++i)
    {
        int j;
        for (j = 0; j < flen; ++j)
        {
            char c1 = theStr[i + j];
            char c2 = theFind[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
            if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
            if (c1 != c2) break;
        }
        if (j == flen) return i;
    }
    return -1;
}

inline bool FileExists(const std::string& theFileName)
{
    // Stub: always returns false. Replace with RFs::Entry() if needed.
    (void)theFileName;
    return false;
}

inline std::string GetFileDir(const std::string& thePath, bool withSlash = false)
{
    (void)withSlash;
    // Stub: just returns path as-is.
    return thePath;
}

inline std::string GetFileName(const std::string& thePath, bool noExtension = false)
{
    (void)noExtension;
    return thePath;
}

} // namespace Sexy

typedef long intptr_t;

// ---------------------------------------------------------------------------
// C library function stubs for Symbian GCCE (no shared libc)
// These are wrapped in extern "C" to avoid linkage conflicts with
// stdapis/string.h declarations (which use extern "C" IMPORT_C).
// ---------------------------------------------------------------------------
#include <time.h>
#include <math.h>
#include <stdarg.h>

// Only define if stdapis/string.h hasn't already declared these
#ifndef _STRING_H
extern "C" {
inline size_t strlen(const char* s) { size_t n = 0; while (s[n]) n++; return n; }
inline int strcmp(const char* a, const char* b) { while (*a && *a == *b) { a++; b++; } return *(const unsigned char*)a - *(const unsigned char*)b; }
inline char* strstr(const char* h, const char* n) { if (!*n) return (char*)h; for (; *h; h++) { const char* hh = h; const char* nn = n; while (*hh && *nn && *hh == *nn) { hh++; nn++; } if (!*nn) return (char*)h; } return NULL; }
inline char* strcpy(char* d, const char* s) { char* r = d; while ((*d++ = *s++)); return r; }
} // extern "C"
#endif

inline int sprintf(char* buf, const char* fmt, ...) { (void)buf; (void)fmt; return 0; }
inline int vsprintf(char* buf, const char* fmt, va_list args) { (void)buf; (void)fmt; (void)args; return 0; }
inline time_t time(time_t* t) { if (t) *t = 0; return 0; }
inline double sin(double x) { (void)x; return 0.0; }
inline double cos(double x) { (void)x; return 1.0; }
inline double fabs(double x) { return x < 0 ? -x : x; }
inline double sqrt(double x) { (void)x; return 0.0; }
inline double pow(double x, double y) { (void)x; (void)y; return 0.0; }
inline double atan2(double y, double x) { (void)y; (void)x; return 0.0; }
inline float sinf(float x) { (void)x; return 0.0f; }
inline float cosf(float x) { (void)x; return 1.0f; }
inline float fabsf(float x) { return x < 0 ? -x : x; }

#endif // COMMON_H
