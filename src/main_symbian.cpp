// main_symbian.cpp -- Symbian startup with boot logging
#include <e32std.h>
#include <e32base.h>
#include <eikstart.h>
#include "platform/symbian/PvZApplication.h"

static void WriteLog(const char* msg) {
    RFs fs; RFile f;
    if (fs.Connect() != KErrNone) return;
    TBuf8<128> b; b.Copy((const TUint8*)msg); b.Append('\n');
    if (f.Open(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite|EFileShareAny) != KErrNone)
        f.Create(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite);
    else { TInt p=0; f.Seek(ESeekEnd,p); }
    f.Write(b); f.Close(); fs.Close();
}

// Application entry point required by EikStart
extern "C" CApaApplication* NewApplication() {
    return CPvZApplication::NewApplication();
}

// usrt2_2.lib's process startup (callfirstprocessfn) references E32Main with
// C linkage, so elf2e32 looks for the unmangled symbol "E32Main". Define it
// extern "C" so the name matches exactly. A weak C++-mangled alias is also
// provided in case any startup variant expects _Z7E32Mainv instead.
extern "C" TInt E32Main()
{
    WriteLog("E32Main START");
    TInt err = EikStart::RunApplication(NewApplication);
    WriteLog("E32Main END");
    return err;
}

// Weak C++-linkage alias -> same entry, harmless if unused / overridden.
__attribute__((weak)) TInt E32Main_cpp_alias()
{
    return E32Main();
}