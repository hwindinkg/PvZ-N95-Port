// main_symbian.cpp -- Symbian startup with boot logging
#include <e32std.h>
#include <e32base.h>
#include <eikstart.h>
#include "platform/symbian/PvZApplication.h"

static void WriteLog(const char* msg) {
    RFs fs; RFile f;
    if (fs.Connect() != KErrNone) return;
    fs.MkDirAll(_L("C:\\Data\\PvZ\\")); // ensure log dir exists
    TBuf8<128> b; b.Copy((const TUint8*)msg); b.Append('\n');
    if (f.Open(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite|EFileShareAny) != KErrNone)
        f.Create(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite);
    else { TInt p=0; f.Seek(ESeekEnd,p); }
    f.Write(b); f.Close(); fs.Close();
}

// Application entry point required by EikStart.
LOCAL_C CApaApplication* NewApplication() {
    return CPvZApplication::NewApplication();
}

// Process entry point. Under the standard abld toolchain the SDK startup
// (eexe.lib -> callfirstprocessfn.cpp) calls E32Main() with C++ LINKAGE, i.e.
// the mangled symbol _Z7E32Mainv. The previous 'extern "C"' produced the
// unmangled "E32Main" (needed only by the old hand-rolled GCCE link) and made
// the abld link fail with: undefined reference to 'E32Main()'.
GLDEF_C TInt E32Main()
{
    WriteLog("E32Main START");
    TInt err = EikStart::RunApplication(NewApplication);
    WriteLog("E32Main END");
    return err;
}
