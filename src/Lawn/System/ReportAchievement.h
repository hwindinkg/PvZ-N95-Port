#ifndef __REPORTACHIEVEMENT_H__
#define __REPORTACHIEVEMENT_H__

#include <e32def.h>

enum AchievementId
{
    Immortal = 0,
    Zombologist = 1,
    PopcornParty = 2,
    PennyPincher = 3,
    // Achievement IDs used in LawnApp.cpp
    HomeSecurity = 4,
    BeyondTheGrave = 5,
    DontPea = 6,
    Grounded = 7,
    GoodMorning = 8,
    NoFungusAmongUs = 9,
    NUM_ACHIEVEMENTS
};

namespace Sexy { class LawnApp; }

class ReportAchievement
{
public:
    static void GiveAchievement(Sexy::LawnApp* theApp, AchievementId theId, bool theForce)
    {
        (void)theApp;
        (void)theId;
        (void)theForce;
    }
};

#endif // __REPORTACHIEVEMENT_H__
