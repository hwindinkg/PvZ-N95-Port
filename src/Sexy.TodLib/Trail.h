#ifndef __TRAIL_H__
#define __TRAIL_H__
#include <e32base.h>

namespace Sexy { class Graphics; }

namespace Sexy {

class Trail
{
public:
    Trail() {}
    void Update() {}
    void Draw(Graphics* g) { (void)g; }
};

} // namespace Sexy

#endif
