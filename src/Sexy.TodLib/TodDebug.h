#ifndef __TODDEBUG_H__
#define __TODDEBUG_H__

#include <e32def.h>

class TodHesitationBracket
{
public:
    char mMessage[256];
    int  mBracketStartTime;
public:
    TodHesitationBracket(const char*, ...) { }
    ~TodHesitationBracket() { }
    void EndBracket() { }
};

void TodLog(const char* theFormat, ...);
void TodLogString(const char* theMsg);
void TodTrace(const char* theFormat, ...);
void TodTraceMemory();
void TodTraceAndLog(const char* theFormat, ...);
void TodTraceWithoutSpamming(const char* theFormat, ...);
void TodHesitationTrace(...);
void TodAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg = "");
void TodErrorMessageBox(const char* theMessage, const char* theTitle);
void* TodMalloc(int theSize);
void TodFree(void* theBlock);
void TodAssertInitForApp();

// GCCE compatible variadic assert - ignores extra args
#ifdef _DEBUG
#define TOD_ASSERT(condition, args...) \
do { \
    if (!(condition)) { \
        TodAssertFailed(#condition, __FILE__, __LINE__); \
        TodTraceMemory(); \
    } \
} while (0)
#else
#define TOD_ASSERT(condition, args...)
#endif

#endif
