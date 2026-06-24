// ModVal.cpp - stub implementation for Symbian port
// (ModVal runtime modification is disabled in RELEASEFINAL builds;
//  M(val) macro just returns val, so functions are never called.)
#include "ModVal.h"

namespace Sexy {

int ModVal(const char* theFileName, int theInt) { (void)theFileName; return theInt; }
double ModVal(const char* theFileName, double theDouble) { (void)theFileName; return theDouble; }
float ModVal(const char* theFileName, float theFloat) { (void)theFileName; return theFloat; }
const char* ModVal(const char* theFileName, const char* theStr) { (void)theFileName; return theStr; }
bool ReparseModValues() { return false; }
void AddModValEnum(const std::string& theEnumName, int theVal) { (void)theEnumName; (void)theVal; }

} // namespace Sexy
