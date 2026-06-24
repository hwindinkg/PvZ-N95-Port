#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__

#include <e32def.h>

class CutScene
{
public:
    CutScene() : mInShovelTutorial(EFalse) {}
    void BeginCutScene() {}
    void EndCutScene() {}
    void Update() {}
    void Draw(class Sexy::Graphics* g) { (void)g; }
    TBool IsInCutScene() { return EFalse; }
    TBool IsInShovelTutorial() { return mInShovelTutorial; }
    void StartFromIdent() {}
    void StartLevelIntro() {}
    void StartZombieWalkIn() {}
    void ZombieWonClick() {}
    void MouseDown(int x, int y, int theClickCount) { (void)x; (void)y; (void)theClickCount; }
    void MouseDown(int& x, int& y) { (void)x; (void)y; }
    bool ShouldRunUpsellBoard() { return EFalse; }
    void ZombieWonStart() {}
    bool ShowZombieWalking() { return EFalse; }

    TBool mInShovelTutorial;
};

#endif // __CUTSCENE_H__
