#pragma once

#include "syati.h"

typedef struct CoinInfo {
    void setDefault();

    bool mExistInDarkComet;
    bool mPlaySound;
    bool mUseSyncAppear;
    bool mUseReadSwitchB;
    bool mIgnoreSensorScaling;
    bool mInitFunction;
    s32 mCoinAddNo;
    char mMirrorActorName[32];
};

class CoinBase : public Coin {
public:
    CoinBase(const char* pName);
    virtual ~CoinBase();
    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);


    virtual bool vRequestGetCoin();
    void noticeGetCoin();
    void appearAndMove(f32 vel, const char* soundName);

    CoinInfo mCoinInfo;
};