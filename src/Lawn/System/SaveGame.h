#ifndef __SAVEGAME_H__
#define __SAVEGAME_H__
#include <e32base.h>
#include <string>

class Board;
class PlayerInfo;

class SaveGame
{
public:
    SaveGame() {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
};

// Global save/load functions used by Board.cpp
inline bool LawnSaveGame(Board* theBoard, const std::string& theFileName) { (void)theBoard; (void)theFileName; return true; }
inline bool LawnLoadGame(Board* theBoard, const std::string& theFileName) { (void)theBoard; (void)theFileName; return true; }
inline std::string GetSavedGameName(int theGameMode, int theSlot)
{
    (void)theGameMode;
    char buf[32];
    sprintf(buf, "save%d.dat", theSlot);
    return std::string(buf);
}
inline std::string GetAppDataPath(const char* subDir)
{
    if (subDir == NULL) return std::string("c:\\private\\e0000000\\");
    return std::string("c:\\private\\e0000000\\") + std::string(subDir) + "\\";
}
inline void MkDir(const std::string& thePath) { (void)thePath; }
#endif
