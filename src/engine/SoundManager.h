#ifndef _SOUNDMANAGER_H_
#define _SOUNDMANAGER_H_

namespace Sexy {

class SoundInstance;

class SoundManager
{
public:
    SoundManager() {}
    void Update() {}
    SoundInstance* GetSoundInstance(int soundId) { (void)soundId; return NULL; }
};

} // namespace Sexy

#endif
