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

// --- malloc / free / realloc stubs (for miniz C library) ---
// miniz uses C standard malloc/free/realloc. The port uses Symbian's
// User::Alloc/User::Free via custom new/delete, but has no libc.
// Provide C-callable wrappers. Use 'unsigned int' instead of size_t
// (Symbian GCCE may not have size_t in this context).
// NOTE: we're already inside the extern "C" block opened at the top
// of this file, so no need for a second extern "C".

void* malloc(unsigned int aSize)
{
    if (aSize == 0) return NULL;
    return User::Alloc(aSize);
}

void free(void* aPtr)
{
    if (aPtr) User::Free(aPtr);
}

void* realloc(void* aPtr, unsigned int aNewSize)
{
    if (aPtr == NULL) return malloc(aNewSize);
    if (aNewSize == 0) { free(aPtr); return NULL; }
    return User::ReAlloc(aPtr, aNewSize);
}

void* calloc(unsigned int aCount, unsigned int aSize)
{
    unsigned int total = aCount * aSize;
    void* p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

// --- strncpy / atof stubs (for ReanimLoader XML parser) ---

char* strncpy(char* aDst, const char* aSrc, unsigned int aN)
{
    char* r = aDst;
    while (aN > 0 && *aSrc) { *aDst++ = *aSrc++; --aN; }
    while (aN > 0) { *aDst++ = 0; --aN; }
    return r;
}

double atof(const char* aStr)
{
    // Simple atof: parse sign, integer part, decimal part
    double result = 0.0;
    double sign = 1.0;
    int i = 0;

    // Skip whitespace
    while (aStr[i] == ' ' || aStr[i] == '\t') i++;

    // Sign
    if (aStr[i] == '-') { sign = -1.0; i++; }
    else if (aStr[i] == '+') i++;

    // Integer part
    while (aStr[i] >= '0' && aStr[i] <= '9')
    {
        result = result * 10.0 + (aStr[i] - '0');
        i++;
    }

    // Decimal part
    if (aStr[i] == '.')
    {
        i++;
        double scale = 0.1;
        while (aStr[i] >= '0' && aStr[i] <= '9')
        {
            result += (aStr[i] - '0') * scale;
            scale *= 0.1;
            i++;
        }
    }

    // Exponent (e.g. 1.5e2)
    if (aStr[i] == 'e' || aStr[i] == 'E')
    {
        i++;
        double expSign = 1.0;
        if (aStr[i] == '-') { expSign = -1.0; i++; }
        else if (aStr[i] == '+') i++;
        int exp = 0;
        while (aStr[i] >= '0' && aStr[i] <= '9')
        {
            exp = exp * 10 + (aStr[i] - '0');
            i++;
        }
        double mult = 1.0;
        for (int j = 0; j < exp; j++) mult *= 10.0;
        if (expSign < 0) result /= mult; else result *= mult;
    }

    return sign * result;
}

} // extern "C" (closes the block opened at line 19)
