/* newop.c - C file for operator new/delete */
#define _UNICODE
#include <e32std.h>

/* operator new/delete with explicit mangled names */
void* _Znwj(unsigned int sz) { return User::Alloc(sz); }
void* _Znaj(unsigned int sz) { return User::Alloc(sz); }
void _ZdlPv(void* p) { if (p) User::Free(p); }
void _ZdaPv(void* p) { if (p) User::Free(p); }
