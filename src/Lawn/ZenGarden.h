#ifndef __ZENGARDEN_H__
#define __ZENGARDEN_H__

#include <e32def.h>

namespace Sexy { class Graphics; }

class Board;
struct PottedPlant;

class ZenGarden
{
public:
    ZenGarden() : mBoard(NULL), mGardenChoice(0), mWheelbarrowPlant(-1), mGardenType(0) {}
    void Update() {}
    void Draw(Sexy::Graphics* g) { (void)g; }
    void LeaveGarden() {}
    int GetPottedPlantInWheelbarrow() { return mWheelbarrowPlant; }
    void WaterPlant(int x, int y) { (void)x; (void)y; }
    void WaterPlant(class Plant* thePlant) { (void)thePlant; }
    void FertilizePlant(int x, int y) { (void)x; (void)y; }
    void FertilizePlant(class Plant* thePlant) { (void)thePlant; }
    void ApplyBugSpray(int x, int y) { (void)x; (void)y; }
    void ApplyBugSpray(class Plant* thePlant) { (void)thePlant; }
    void ApplyPhonograph(int x, int y) { (void)x; (void)y; }
    void ApplyPhonograph(class Plant* thePlant) { (void)thePlant; }
    void ApplyChocolate(int x, int y) { (void)x; (void)y; }
    void ApplyChocolate(class Plant* thePlant) { (void)thePlant; }
    void PlantInWheelbarrow(int x, int y) { (void)x; (void)y; }
    void PlantInWheelbarrow(class Plant* thePlant) { (void)thePlant; }
    void SellPlant(int x, int y) { (void)x; (void)y; }
    void SellPlant(class Plant* thePlant) { (void)thePlant; }
    void DropPlantFromWheelbarrow(int x, int y) { (void)x; (void)y; }
    void DropPlantFromWheelbarrow(class Plant* thePlant) { (void)thePlant; }
    void DropPlantFromWheelbarrow(long) { }
    void GardenNext() {}
    int PickRandomSeedType() { return 0; }
    bool IsZenGardenFull(bool) { return false; }
    void AddPottedPlant(struct PottedPlant*) {}
    void DrawPottedPlantIcon(Sexy::Graphics*, float, float, struct PottedPlant*) {}
    bool IsStinkyHighOnChocolate() { return false; }

    // Stub methods for Challenge.cpp compatibility
    void ZenGardenInitLevel() {}
    int MouseDownZenGarden(int x, int y, int theClickCount, class HitResult* theHitResult) { (void)x; (void)y; (void)theClickCount; (void)theHitResult; return 0; }

    Board* mBoard;
    int mGardenChoice;
    int mWheelbarrowPlant;
    int mGardenType;

    int ZenPlantOffsetX(int thePottedPlantIndex) { (void)thePottedPlantIndex; return 0; }
    int PlantPottedDrawHeightOffset(int thePottedPlantIndex) { (void)thePottedPlantIndex; return 0; }
};

#endif
