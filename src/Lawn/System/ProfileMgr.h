#ifndef __PROFILEMGR_H__
#define __PROFILEMGR_H__
#include <e32base.h>
#include <string>

class PlayerInfo;

class ProfileMgr
{
public:
    ProfileMgr() {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    PlayerInfo* GetProfile(const std::string& name) { (void)name; return NULL; }
    PlayerInfo* AddProfile(const std::string& name) { (void)name; return NULL; }
    void DeleteProfile(const std::string& name) { (void)name; }
    bool RenameProfile(const std::string& oldName, const std::string& newName) { (void)oldName; (void)newName; return false; }
    PlayerInfo* GetAnyProfile() { return NULL; }
    void Load() {}
    void Save() {}
};
#endif
