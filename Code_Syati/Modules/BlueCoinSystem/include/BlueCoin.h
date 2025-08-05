#pragma once

#include "syati.h"
#include "BlueCoinUtil.h"
#include "CoinBase.h"

class BlueCoin : public CoinBase {
    public:
    BlueCoin(const char*);
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual bool vRequestGetCoin();

    void onSwitchAppear();

    s32 mID;
    f32 mLaunchVelocity;
    bool mIsCollected;
};
