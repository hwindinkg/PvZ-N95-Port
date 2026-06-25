// clib_stubs.cpp
// Minimal C runtime functions that the N95 link set does not provide.
//
// The Symbian stdapis headers (<string.h>, <math.h>, <stdlib.h>) DECLARE these
// as extern "C", but no C library (estlib / PIPS) is linked into this EXE, so
// armlink reports them as "Unresolved symbol".  Common.h provides C++-mangled
// inline versions for code that includes it, but any translation unit that
// pulls the prototypes from the system headers references the unmangled C
// symbols below.  We supply real, external definitions here.
//
// This file intentionally does NOT include Common.h, so the extern "C"
// definitions never clash with Common.h's inline C++ helpers.

#include <e32std.h>
#include <e32math.h>

typedef unsigned int sexy_size_t;

extern "C" {

sexy_size_t strlen(const char* s)
{
    sexy_size_t n = 0;
    while (s[n]) ++n;
    return n;
}

char* strcpy(char* d, const char* s)
{
    char* r = d;
    while ((*d++ = *s++) != 0) {}
    return r;
}

int strcmp(const char* a, const char* b)
{
    while (*a && *a == *b) { ++a; ++b; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char* a, const char* b, sexy_size_t n)
{
    while (n && *a && *a == *b) { ++a; ++b; --n; }
    if (n == 0) return 0;
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char* strstr(const char* haystack, const char* needle)
{
    if (!*needle) return (char*)haystack;
    for (; *haystack; ++haystack)
    {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { ++h; ++n; }
        if (!*n) return (char*)haystack;
    }
    return 0;
}

double fmod(double x, double y)
{
    if (y == 0.0) return 0.0;
    double q = x / y;
    // Truncate toward zero, matching the C library fmod definition.
    double t = (q < 0.0) ? -(double)(long)(-q) : (double)(long)q;
    return x - t * y;
}

int rand(void)
{
    // Math::Random() returns a 32-bit pseudo-random TUint; mask to RAND_MAX.
    return (int)(Math::Random() & 0x7fff);
}

} // extern "C"
