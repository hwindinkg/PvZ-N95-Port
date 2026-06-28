#ifndef __TYPINGCHECK_H__
#define __TYPINGCHECK_H__
#include <e32base.h>

namespace Sexy { class LawnApp; }

class TypingCheck
{
public:
    TypingCheck() : mApp(NULL) {}
    TypingCheck(const char* s) : mApp(NULL) { (void)s; }
    TypingCheck(Sexy::LawnApp* theApp) : mApp(theApp) {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    void TypingCheckAddKeyCheck(int key) { (void)key; }
    void TypingCheckAddKeyCheck(int key, const char* str) { (void)key; (void)str; }
    void AddKeyCode(int key) { (void)key; }
    void AddChar(char c) { (void)c; }
    Sexy::LawnApp* mApp;
};
#endif
