#ifndef __REANIMATIONLAWN_H__
#define __REANIMATIONLAWN_H__

#include <e32def.h>

// Lawn-specific reanimation types and helpers
enum ReanimationLawnType
{
    REANIM_LAWN_NONE = -1,
    REANIM_LAWN_ZOMBIE_HEAD = 0,
    REANIM_LAWN_SPROUT = 1,
    REANIM_LAWN_PEASHOOTER = 2,
    REANIM_LAWN_SUNFLOWER = 3,
    NUM_REANIM_LAWN_TYPES
};

#endif
