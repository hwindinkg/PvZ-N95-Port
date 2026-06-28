#ifndef _SOUNDINSTANCE_H_
#define _SOUNDINSTANCE_H_

namespace Sexy {

class SoundInstance
{
public:
    SoundInstance() {}
    void Play(bool looping = false, bool autoRelease = false) { (void)looping; (void)autoRelease; }
    void Stop() {}
    void SetVolume(double vol) { (void)vol; }
    void SetVolume(float vol) { (void)vol; }
    void SetPan(int pan) { (void)pan; }
    void AdjustPitch(float pitch) { (void)pitch; }
    bool IsPlaying() const { return false; }
    void Release() {}
};

} // namespace Sexy

#endif
