// RegEmu.cpp - stub implementation for Symbian port
// (No Windows registry on Symbian — all reads return false, writes are no-ops.)
#include "RegEmu.h"

namespace regemu {

void SetRegFile(const std::string& fileName) { (void)fileName; }
bool RegistryRead(const std::string& keyName, const std::string& valueName, uint32_t* type, uint8_t* value, uint32_t* length) {
    (void)keyName; (void)valueName; (void)type; (void)value; (void)length;
    return false;
}
bool RegistryWrite(const std::string& keyName, const std::string& valueName, uint32_t type, const uint8_t* value, uint32_t length) {
    (void)keyName; (void)valueName; (void)type; (void)value; (void)length;
    return false;
}
bool RegistryEraseKey(const std::string& keyName) { (void)keyName; return false; }
bool RegistryEraseValue(const std::string& keyName, const std::string& valueName) {
    (void)keyName; (void)valueName;
    return false;
}

} // namespace regemu
