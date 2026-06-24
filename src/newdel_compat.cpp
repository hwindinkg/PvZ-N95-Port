/*
 * newdel_compat.cpp  (PvZ N95 port)
 * ------------------------------------------------------------------
 * Symbian^3 SDKs move the global C++ operator new/delete into
 * scppnwdl.dll -- a DLL that does NOT exist on N95 (S60 3rd FP1)
 * firmware. If the EXE imports it, the loader refuses to start the
 * app ("function not supported"). Defining the operators here (same
 * semantics as the old euser: Alloc/Free, NULL on OOM, no throw)
 * makes the linker use these instead of the scppnwdl stubs, so the
 * import disappears. new (ELeave) on CBase lives in euser and is
 * unaffected.  Pattern adopted from the working Whisk3D N95 port.
 * ------------------------------------------------------------------
 */

#ifndef __WINS__

#include <e32std.h>

void* operator new(unsigned int aSize)        { return User::Alloc(aSize); }
void* operator new[](unsigned int aSize)      { return User::Alloc(aSize); }
void  operator delete(void* aPtr)             { User::Free(aPtr); }
void  operator delete[](void* aPtr)           { User::Free(aPtr); }

#endif // !__WINS__
