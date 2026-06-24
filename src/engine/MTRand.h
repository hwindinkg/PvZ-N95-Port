#ifndef _MTRAND_H_
#define _MTRAND_H_

namespace Sexy {

class MTRand
{
public:
    MTRand() {}
    MTRand(unsigned long seed) { (void)seed; }
    void SRand(unsigned long seed) { (void)seed; }
    unsigned long Next() { return 0; }
    unsigned long Next(unsigned long max) { (void)max; return 0; }
};

} // namespace Sexy

#endif
