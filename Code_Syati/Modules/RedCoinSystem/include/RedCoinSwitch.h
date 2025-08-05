#pragma once

#include "syati.h"
#include "Game/Screen/TimeLimitLayout.h"

class RedCoinController;

class RedCoinSwitch : public LiveActor {
public:
    RedCoinSwitch(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual bool receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual void control();
    void exePress();
    void exeRelease();

    TimeLimitLayout* mTimeLimitLayout;
    RedCoinController* mRedCoinController;
    s32 mTime;
    s32 mMode;
};

namespace NrvRedCoinSwitch {
    NERVE(NrvWait);
    NERVE(NrvPress);
    NERVE(NrvRelease);
}