#include "PvZApplication.h"
#include "PvZDocument.h"
#include <f32file.h>

static void BootLog(const char* msg) {
    RFs fs; RFile f;
    if (fs.Connect() != KErrNone) return;
    fs.MkDirAll(_L("C:\\Data\\PvZ\\")); // ensure log dir exists
    TBuf8<128> b; b.Copy((const TUint8*)msg); b.Append('\n');
    if (f.Open(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite|EFileShareAny) != KErrNone)
        f.Create(fs, _L("C:\\Data\\PvZ\\boot.log"), EFileWrite);
    else { TInt p=0; f.Seek(ESeekEnd,p); }
    f.Write(b); f.Close(); fs.Close();
}

CApaApplication* CPvZApplication::NewApplication() {
    BootLog("CPvZApplication::NewApplication");
    return new CPvZApplication;
}

TUid CPvZApplication::AppDllUid() const {
    BootLog("AppDllUid");
    return KUidPvZApp;
}

CApaDocument* CPvZApplication::CreateDocumentL() {
    BootLog("CreateDocumentL");
    return CPvZDocument::NewL(*this);
}