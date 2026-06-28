#ifndef __TODFOLEY_H__
#define __TODFOLEY_H__

#include <e32def.h>

enum FoleyType
{
    FOLEY_NONE = -1,
    FOLEY_PLANT = 0,
    FOLEY_SPLAT = 1,
    FOLEY_GROAN = 2,
    FOLEY_CHOMP = 3,
    FOLEY_EXPLOSION = 4,
    FOLEY_SHOOT = 5,
    FOLEY_FROZEN = 6,
    FOLEY_JACK = 7,
    FOLEY_RAIN = 8,
    FOLEY_WINCHIME = 9,
    FOLEY_BLEEP = 10,
    FOLEY_FALL = 11,
    FOLEY_SPAWN_SUN = 12,
    FOLEY_FINAL_FANFARE = 13,
    FOLEY_WINMUSIC = 14,
    FOLEY_CERAMIC = 15,
    FOLEY_PLANT_WATER = 16,
    FOLEY_OUT_OF_SUN = 17,
    FOLEY_GRAVE = 18,
    FOLEY_HUGE_WAVE = 19,
    FOLEY_SHOVEL = 20,
    // Extended foley types for plant effects
    FOLEY_REVERSE_EXPLOSION,
    FOLEY_GRAVEBUSTERCHOMP,
    FOLEY_SQUISH,
    FOLEY_THROW,
    FOLEY_PLANTGROW,
    FOLEY_DIRT_RISE,
    FOLEY_FLOOP,
    FOLEY_ZOMBIE_ENTERING_WATER,
    FOLEY_SQUASH_HMM,
    FOLEY_THUMP,
    FOLEY_SHOOP,
    FOLEY_COB_LAUNCH,
    FOLEY_BIGCHOMP,
    FOLEY_MAGNETSHROOM,
    FOLEY_COIN,
    FOLEY_CHERRYBOMB,
    FOLEY_BOWLINGIMPACT,
    FOLEY_SHIELD_HIT,
    FOLEY_PLASTIC_HIT,
    FOLEY_WAKEUP,
    FOLEY_JUICY,
    FOLEY_JALAPENO_IGNITE,
    FOLEY_COFFEE,
    FOLEY_FUME,
    FOLEY_SNOW_PEA_SPARKLES,
    FOLEY_PUFF,
    FOLEY_BUNGEE_SCREAM,
    FOLEY_UMBRELLA,
    FOLEY_GRASSSTEP,
    FOLEY_POGO_ZOMBIE,
    FOLEY_BONK,
    FOLEY_BASKETBALL,
    FOLEY_NEWSPAPER_RARRGH,
    FOLEY_POLEVAULT,
    FOLEY_DOLPHIN_BEFORE_JUMPING,
    FOLEY_JACK_SURPRISE,
    FOLEY_SWING,
    FOLEY_IMP,
    FOLEY_LOW_GROAN,
    FOLEY_GRAVESTONE_RUMBLE,
    FOLEY_DANCER,
    FOLEY_SLURP,
    FOLEY_LIMBS_POP,
    FOLEY_KERNEL_SPLAT,
    FOLEY_BUTTER,
    FOLEY_IGNITE,
    FOLEY_MELONIMPACT,
    FOLEY_FIREPEA,
    FOLEY_CHIME,
    FOLEY_MONEYFALLS,
    FOLEY_SUN,
    FOLEY_PRIZE,
    FOLEY_ZOMBIESPLASH,
    FOLEY_LAWNMOWER,
    FOLEY_POOL_CLEANER,
    FOLEY_PORTAL,
    FOLEY_ART_CHALLENGE,
    NUM_FOLEY_TYPES
};

class TodFoley
{
public:
    TodFoley() {}
    void StartFoley(FoleyType type) { (void)type; }
    void StopFoley(int type) { (void)type; }
    void PlayFoley(int type) { (void)type; }
    void PlayFoleyPitch(int type, float pitch) { (void)type; (void)pitch; }
    void SetFoleyVolume(FoleyType type, float vol) { (void)type; (void)vol; }
    void SetMusicVolume(float vol) { (void)vol; }
    void SetSfxVolume(float vol) { (void)vol; }
    void GamePause(bool thePause) { (void)thePause; }
    void UpdateFoley() {}
    void StopAllSounds() {}
    void CancelPausedFoley() {}
};

#endif // __TODFOLEY_H__
