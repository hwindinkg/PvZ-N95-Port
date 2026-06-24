// platform/symbian/PvZDocument.cpp

#include "PvZDocument.h"
#include "PvZAppUi.h"

CPvZDocument* CPvZDocument::NewL(CEikApplication& aApp)
{
    CPvZDocument* self = new (ELeave) CPvZDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CPvZDocument::CPvZDocument(CEikApplication& aApp)
    : CAknDocument(aApp)
{
}

void CPvZDocument::ConstructL() {}

CPvZDocument::~CPvZDocument() {}

CEikAppUi* CPvZDocument::CreateAppUiL()
{
    return new (ELeave) CPvZAppUi;
}
