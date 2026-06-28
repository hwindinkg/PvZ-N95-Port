/*
 * newdel_compat.cpp  (PvZ N95 port)
 * ------------------------------------------------------------------
 * Symbian^3+ SDKs move the global C++ operator new/delete into
 * scppnwdl.dll -- a DLL that does NOT exist on N95 (S60 3rd FP1)
 * firmware. Defining the operators here (semantics: User::Alloc/Free,
 * NULL on OOM, no throw) makes the linker use these instead, so the
 * scppnwdl import disappears. Pattern from the working Whisk3D port.
 *
 * IMPORTANT: e32cmn.h declares these with an empty exception-spec
 * "throw ()". The definitions MUST match it exactly or GCCE 3.4.3
 * errors with "declaration ... throws different exceptions".
 * ------------------------------------------------------------------
 */

#ifndef __WINS__

#include <e32std.h>

void* operator new(unsigned int aSize) throw()        { return User::Alloc(aSize); }
void* operator new[](unsigned int aSize) throw()      { return User::Alloc(aSize); }
void  operator delete(void* aPtr) throw()             { User::Free(aPtr); }
void  operator delete[](void* aPtr) throw()           { User::Free(aPtr); }

#endif // !__WINS__
