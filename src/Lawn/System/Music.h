#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <e32def.h>

enum MusicTune
{
    MUSIC_TUNE_NONE = -1,
    MUSIC_TUNE_DAY_GRASSWALK = 0,
    MUSIC_TUNE_NIGHT_MOONGRAINS = 1,
    MUSIC_TUNE_POOL_WATERYGRAVES = 2,
    MUSIC_TUNE_FOG_RIGORMORMIST = 3,
    MUSIC_TUNE_ROOF_GRAZETHEROOF = 4,
    MUSIC_TUNE_ZEN_GARDEN = 5,
    MUSIC_TUNE_TITLE = 6,
    MUSIC_TUNE_CHALLENGE = 7,
    MUSIC_TUNE_BOSS = 8,
    NUM_MUSIC_TUNES
};

class Music
{
public:
    Music() : mCurrentTune(MUSIC_TUNE_NONE) {}
    void MusicUpdate() {}
    void StartMusic(MusicTune tune) { mCurrentTune = tune; }
    void StopMusic() {}
    void StopAllMusic() {}
    void SetVolume(float vol) { (void)vol; }
    void FadeIn(int ms) { (void)ms; }
    void FadeOut(int ms) { (void)ms; }
    TBool IsPlaying() { return EFalse; }
    void PlayFromOffset(float offset) { (void)offset; }
    void GameMusicPause(bool thePause) { (void)thePause; }
    void StartGameMusic() {}
    void StartGameMusic(int theMusicOverride) { (void)theMusicOverride; }
    MusicTune GetCurrentTune() { return mCurrentTune; }
    void UpdateMusic() { MusicUpdate(); }
    void GameMusicFadeInStop() {}
    int GetNumTune() { return 0; }
    void PlayMusic(int tune) { StartMusic((MusicTune)tune); }
    void PlayMusic(int& theTune, int theOffset, bool noLoop, bool forcePlay) { (void)theTune; (void)theOffset; (void)noLoop; (void)forcePlay; }
    void MusicInit() {}
    int GetNumLoadingTasks() { return 0; }

    MusicTune mCurrentTune;
    int mCurrentMusicTuneIndex;
};

#endif // __MUSIC_H__
