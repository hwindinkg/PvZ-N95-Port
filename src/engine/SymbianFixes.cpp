// engine/SymbianFixes.cpp
#include "SymbianFixes.h"
#include <e32std.h>

// ---------------------------------------------------------------------------
// Legacy EKA1 process-entry trap-harness shim (weak).
//
// S60 3rd FP1's precompiled usrt2_2.lib(callfirstprocessfn.o) wraps the call to
// E32Main() in the old TTrap harness:  if (t.Trap(r)==0) { r=E32Main(); UnTrap(); }
// But on EKA2 leaves are C++ exceptions (__LEAVE_EQUALS_THROW__) and euser.dso
// does not export TTrap::Trap, so elf2e32 reports E1035 for _ZN5TTrap4TrapERi.
//
// We provide a trivial pass-through: Trap() installs nothing and returns 0, so
// the entry simply runs E32Main(); the matching UnTrap() is a no-op. The symbols
// are WEAK, so if any euser.dso variant actually exports them the real ones win
// and these are ignored -- no multiple-definition conflict either way.
//
// This is safe because EikStart::RunApplication() TRAPs the whole application
// internally (compiled with exceptions), so no leave ever escapes E32Main().
//
// 'this' is the implicit first argument of the non-static member in the C++ ABI.
// ---------------------------------------------------------------------------
extern "C" __attribute__((weak)) TInt _ZN5TTrap4TrapERi(TAny* /*aThis*/, TInt& aResult)
    {
    aResult = KErrNone;
    return KErrNone;   // 0 => run the protected block (call E32Main)
    }

// User::UnTrap()  (static)  and  TTrap::UnTrap()  (if referenced) -- no-op pairs.
extern "C" __attribute__((weak)) void _ZN4User6UnTrapEv(void) {}
extern "C" __attribute__((weak)) void _ZN5TTrap6UnTrapEv(TAny* /*aThis*/) {}
