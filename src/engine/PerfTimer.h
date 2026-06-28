#ifndef _PERFTIMER_H_
#define _PERFTIMER_H_

namespace Sexy {

class PerfTimer
{
public:
    PerfTimer() {}
    void Start() {}
    void Stop() {}
    double GetDuration() const { return 0.0; }
};

} // namespace Sexy

#endif
