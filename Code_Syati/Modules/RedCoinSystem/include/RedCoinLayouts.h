#pragma once

#include "syati.h"
#include "Game/Screen/CountUpPaneRumbler.h"

class RedCoinCounter : public LayoutActor {
public:
    RedCoinCounter(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void control();
    void setStarIcon(s32 starID, s32 iconID, bool constant);
    void startCountUp(s32 count);
    void exeAppear();
    void exeDisappear();
    void exeCountUp();
    void exeComplete();

    CounterLayoutAppearer* mAppearer;
    CountUpPaneRumbler* mPaneRumbler;
    TVec2f mFollowPos;
    s32 mRedCoinCount;
    s32 mLayoutMode;
    bool mUseFollowPos;
    bool mUpdate;
};

namespace NrvRedCoinCounter {
    NERVE(NrvAppear);
    NERVE(NrvWait);
    NERVE(NrvDisappear);
    NERVE(NrvCountUp);
    NERVE(NrvComplete);
    NERVE(NrvHide); 
};

class RedCoinCounterPlayer : public LayoutActor {
public:
    RedCoinCounterPlayer(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    void exeAppear();
    void calcScreenPos();

    LiveActor* mLastRedCoin;
    s32 mNumCoins;
};

namespace NrvRedCoinCounterPlayer {
    NERVE(NrvHide);
    NERVE(NrvAppear);
}