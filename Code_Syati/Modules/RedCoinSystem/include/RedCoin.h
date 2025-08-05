#pragma once

#include "syati.h"
#include "Game/MapObj/Coin.h"
#include "Game/Screen/CountUpPaneRumbler.h"
#include "CoinBase.h"

class RedCoin : public CoinBase {
public:
    RedCoin(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual bool vRequestGetCoin();


    LiveActorGroup* mGroup;
    f32 mLaunchVelocity;
    bool mHasRewardedCoins;
    bool mRedCoinCounterPlayerPos;
};