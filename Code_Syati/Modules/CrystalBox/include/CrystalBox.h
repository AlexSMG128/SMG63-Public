#pragma once

#include "syati.h"

class CrystalBox : public LiveActor {
public:
    CrystalBox(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    void exeBreak();

    ModelObj* mBreakModel;
    DummyDisplayModel* mDisplayModel;
    Coin* mPurpleCoin;
    TMtx34f mNewBaseMtx;
    s32 mDisplayModelID;
    f32 mDisplayModelOffset;
};

namespace NrvCrystalBox {
    NERVE(NrvWait);
    NERVE(NrvBreak);
};