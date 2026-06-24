#ifndef __PLAYERINFO_H__
#define __PLAYERINFO_H__

#include <e32def.h>
#include <string>
#include "../../ConstEnums.h"
#include "../../GameConstants.h"

// Forward declare PottedPlant (defined in GameConstants.h)
struct PottedPlant;

class PlayerInfo
{
public:
    PlayerInfo() : mId(0), mLevel(0), mFinishedAdventure(false),
                   mShovelUnlocked(false), mGloveUnlocked(false),
                   mMoneySignUnlocked(false), mWheelbarrowUnlocked(false),
                   mZombiesKilled(0), mPlantsPlanted(0), mSunUsed(0),
                   mMoney(0), mSurvivalNormalFlags(0), mSurvivalHardFlags(0),
                   mSurvivalEndlessFlags(0), mCoins(0), mSunMoney(0),
                   mHasUsedGardeningGlove(false), mIZombieEndlessStreak(0),
                   mScaryPotterEndlessStreak(0),
                   mHasUsedCheatKeys(false), mNeedsMessageOnGameSelector(false),
                   mHasNewSurvival(false), mHasNewScaryPotter(false),
                   mHasNewIZombie(false), mHasNewMiniGame(false),
                   mNeedsMagicTacoReward(false), mHasSeenUpsell(false),
                   mPlayTimeActivePlayer(0), mPlayTimeInactivePlayer(0)
    {
        for (int i = 0; i < NUM_STORE_ITEMS; ++i)
            mPurchases[i] = 0;
        for (int i = 0; i < 50; ++i)
            mPottedPlant[i].mSeedType = -1;
        for (int i = 0; i < MAX_ACHIEVEMENTS; ++i)
        {
            mEarnedAchievements[i] = false;
            mShownAchievements[i] = false;
        }
        for (int i = 0; i < MAX_CHALLENGE_RECORDS; ++i)
            mChallengeRecords[i] = 0;
    }

    void AddCoins(int amount) { mMoney += amount; }
    void SaveDetails() {}
    void SetLevel(int lvl) { mLevel = lvl; }
    int GetLevel() { return mLevel; }

    int mId;
    int mLevel;
    bool mFinishedAdventure;
    int mPurchases[NUM_STORE_ITEMS];
    bool mShovelUnlocked;
    bool mGloveUnlocked;
    bool mMoneySignUnlocked;
    bool mWheelbarrowUnlocked;
    int mZombiesKilled;
    int mPlantsPlanted;
    int mSunUsed;
    int mMoney;
    int mCoins;
    int mSunMoney;
    int mSurvivalNormalFlags;
    int mSurvivalHardFlags;
    int mSurvivalEndlessFlags;
    int mIZombieEndlessStreak;
    int mScaryPotterEndlessStreak;
    bool mHasUsedGardeningGlove;
    int mHasUnlockedMinigames;
    int mHasUnlockedPuzzleMode;
    int mHasUnlockedSurvivalMode;
    PottedPlant mPottedPlant[50];

    // Additional members used by LawnApp.cpp
    std::string mName;
    bool mEarnedAchievements[MAX_ACHIEVEMENTS];
    bool mShownAchievements[MAX_ACHIEVEMENTS];
    bool mHasUsedCheatKeys;
    bool mNeedsMessageOnGameSelector;
    bool mHasNewSurvival;
    bool mHasNewScaryPotter;
    bool mHasNewIZombie;
    bool mHasNewMiniGame;
    bool mNeedsMagicTacoReward;
    bool mHasSeenUpsell;
    int  mPlayTimeActivePlayer;
    int  mPlayTimeInactivePlayer;
    int  mChallengeRecords[MAX_CHALLENGE_RECORDS];
};

#endif
