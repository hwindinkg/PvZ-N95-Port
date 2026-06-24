// platform/symbian/PvZApplication.h
// C++03 — без override

#ifndef PVZAPPLICATION_H
#define PVZAPPLICATION_H

#include <aknapp.h>

const TUid KUidPvZApp = { 0xE1234567 };

class CPvZApplication : public CAknApplication
{
public:
    static CApaApplication* NewApplication();

private:
    TUid AppDllUid() const;
    CApaDocument* CreateDocumentL();
};

#endif // PVZAPPLICATION_H
