// PropertiesParser.cpp - stub implementation for Symbian port
// (XML config parsing is disabled in Symbian port — properties are not loaded.)
#include "PropertiesParser.h"

namespace Sexy {

PropertiesParser::PropertiesParser(SexyAppBase* theApp) : mApp(theApp), mXMLParser(NULL), mHasFailed(false) {}
PropertiesParser::~PropertiesParser() {}
void PropertiesParser::Fail(const std::string& theErrorText) { (void)theErrorText; mHasFailed = true; }
bool PropertiesParser::ParseSingleElement(std::string* theString) { (void)theString; return false; }
bool PropertiesParser::ParseStringArray(StringVector* theStringVector) { (void)theStringVector; return false; }
bool PropertiesParser::ParseProperties() { return false; }
bool PropertiesParser::DoParseProperties() { return false; }
bool PropertiesParser::ParsePropertiesFile(const std::string& theFilename) { (void)theFilename; return false; }
bool PropertiesParser::ParsePropertiesBuffer(const Buffer& theBuffer) { (void)theBuffer; return false; }
std::string PropertiesParser::GetErrorText() { return mError; }

} // namespace Sexy
