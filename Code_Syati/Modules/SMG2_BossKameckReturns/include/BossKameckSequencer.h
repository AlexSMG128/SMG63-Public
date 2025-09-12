#pragma once

#include "Game/LiveActor/LiveActor.h"
#include "Game/System/NerveExecutor.h"

class BossKameck;
class BossKameckBattleDemo;
class BossKameckStateBattle;
class BossKameckBarrier;

class BossKameckSequencer : public NerveExecutor {
public:
    BossKameckSequencer(const char*);

    inline virtual ~BossKameckSequencer() {

    }
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void start();
    virtual void init(BossKameck*, const JMapInfoIter&);
    virtual void update();

    bool isBattle() const;

    void exeOpeningDemo();
    void exeBattle(bool);
    void exePowerUpDemo();
    void exeEndDemo();


    BossKameck* mBossKameck;
    BossKameckBattleDemo* mBattleDemo;
    BossKameckStateBattle* mBattleState;
    BossKameckBarrier* mBarrier;
};

namespace NrvBossKameckSequencer {
    NERVE(BossKameckSequencerNrvOpeningDemo);
    NERVE(BossKameckSequencerNrvBattleFirst);
    NERVE(BossKameckSequencerNrvBattleLoop);
    NERVE(BossKameckSequencerNrvPowerUpDemo);
    NERVE(BossKameckSequencerNrvEndDemo);
};