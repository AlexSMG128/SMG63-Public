#pragma once

#include "Game/LiveActor/LiveActor.h"

class BossKameckBarrier : public LiveActor {
public:
    BossKameckBarrier(const char*);

    virtual ~BossKameckBarrier();
    virtual void init(const JMapInfoIter&);

    void exeWait();
};

namespace NrvBossKameckBarrier {
    NERVE(BossKameckBarrierNrvWait);
};