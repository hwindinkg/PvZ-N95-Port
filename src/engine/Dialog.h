#ifndef DIALOG_H
#define DIALOG_H

#include "Widget.h"
#include "DialogListener.h"
#include <string>

namespace Sexy { class LawnApp; }

namespace Sexy {

class ButtonWidget;

class Dialog : public Widget
{
public:
    Dialog() : Widget(), mImage(NULL), mReanimation(NULL), mSpaceAfterHeader(0), mYesButton(NULL), mNoButton(NULL), mId(0) {}
    Dialog(int theDialogId, bool isModal, const std::string& header, const std::string& lines, const std::string& footer, int buttonMode)
        : Widget(), mImage(NULL), mReanimation(NULL), mSpaceAfterHeader(0), mYesButton(NULL), mNoButton(NULL), mId(theDialogId)
    { (void)isModal; (void)header; (void)lines; (void)footer; (void)buttonMode; }
    virtual ~Dialog() {}

    virtual int WaitForResult(bool) { return 0; }
    virtual void CalcSize(int, int) {}
    virtual void SetButtonDelay(int) {}

    int mId;
    class Sexy::Image* mImage;
    ButtonWidget* mYesButton;
    ButtonWidget* mNoButton;
    class Reanimation* mReanimation;
    int mSpaceAfterHeader;

    // Button mode constants
    static const int BUTTONS_NONE = 0;
    static const int BUTTONS_FOOTER = 1;
    static const int BUTTONS_YES_NO = 2;
    static const int BUTTONS_OK_CANCEL = 3;
};

} // namespace Sexy

// LawnDialog: extended Dialog used by LawnApp (in global namespace, as used by LawnApp.cpp)
class LawnDialog : public Sexy::Dialog
{
public:
    LawnDialog(Sexy::LawnApp* theApp, int theDialogId, bool isModal, const std::string& header, const std::string& lines, const std::string& footer, int buttonMode)
        : Dialog(theDialogId, isModal, header, lines, footer, buttonMode), mLawnYesButton(NULL), mLawnNoButton(NULL) { (void)theApp; }
    virtual ~LawnDialog() {}

    Sexy::ButtonWidget* mLawnYesButton;
    Sexy::ButtonWidget* mLawnNoButton;
};

#endif // DIALOG_H
