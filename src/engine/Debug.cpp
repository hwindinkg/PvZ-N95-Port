// Debug.cpp - stub implementation matching Debug.h
// (PopCap framework full implementation removed for Symbian port;
//  assert macros in Debug.h use standard assert() directly)
#include "Common.h"
#include "Debug.h"

bool gInAssert = false;

void SexyTraceFmt(const char* fmt ...) { (void)fmt; }
void OutputDebug(const char* fmt ...) { (void)fmt; }
