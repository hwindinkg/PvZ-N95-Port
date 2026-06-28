// engine/SymbianFixes.h
// Compatibility macros and helpers for Symbian GCCE.
// Removed: PVZ_gl* stub functions (replaced by real GLES 1.1 calls via GLInterface).

#ifndef SYMBIAN_FIXES_H
#define SYMBIAN_FIXES_H

#include <e32base.h>

// ---------------------------------------------------------------------------
// EGL attribute macros for platforms where headers may be incomplete
// ---------------------------------------------------------------------------
#ifndef EGL_OPENGL_ES_BIT
#define EGL_OPENGL_ES_BIT 0x0001
#endif

#ifndef EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#endif

#ifndef EGL_DEFAULT_DISPLAY
#define EGL_DEFAULT_DISPLAY ((EGLNativeDisplayType)0)
#endif

#ifndef EGL_RENDERABLE_TYPE
#define EGL_RENDERABLE_TYPE 0x3040
#endif

#ifndef EGL_SURFACE_TYPE
#define EGL_SURFACE_TYPE 0x3033
#endif

#ifndef EGL_WINDOW_BIT
#define EGL_WINDOW_BIT 0x0004
#endif

#ifndef EGL_BLUE_SIZE
#define EGL_BLUE_SIZE 0x3022
#endif

#ifndef EGL_GREEN_SIZE
#define EGL_GREEN_SIZE 0x3023
#endif

#ifndef EGL_RED_SIZE
#define EGL_RED_SIZE 0x3024
#endif

#ifndef EGL_DEPTH_SIZE
#define EGL_DEPTH_SIZE 0x3025
#endif

#ifndef EGL_NONE
#define EGL_NONE 0x3038
#endif

// ---------------------------------------------------------------------------
// Math helpers
// ---------------------------------------------------------------------------
inline TInt FloatToInt(float f) { return static_cast<TInt>(f); }

inline float IntToFloat(TInt i) { return static_cast<float>(i); }

inline TBool FloatApproxEqual(float a, float b, float eps = 0.0001f)
{
    float diff = a - b;
    return (diff >= -eps && diff <= eps);
}

// ---------------------------------------------------------------------------
// String helpers
// ---------------------------------------------------------------------------
inline TInt StringCompare(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2)) { ++s1; ++s2; }
    return static_cast<unsigned char>(*s1) - static_cast<unsigned char>(*s2);
}

#endif // SYMBIAN_FIXES_H
