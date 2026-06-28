// platform/symbian/PvZDocument.h
// C++03 — без override

#ifndef PVZDOCUMENT_H
#define PVZDOCUMENT_H

#include <akndoc.h>

class CPvZDocument : public CAknDocument
{
public:
    static CPvZDocument* NewL(CEikApplication& aApp);
    ~CPvZDocument();

private:
    CPvZDocument(CEikApplication& aApp);
    void ConstructL();
    CEikAppUi* CreateAppUiL();
};

#endif // PVZDOCUMENT_H
