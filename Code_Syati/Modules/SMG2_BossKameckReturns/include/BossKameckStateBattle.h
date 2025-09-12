#pragma once

#include "Game/LiveActor/ActorStateBase.h"
#include "Game/System/NerveExecutor.h"
#include "Game/LiveActor/Spine.h"
#include "JSystem/JGeometry/TVec.h"

class BossKameck;
class BossKameckMoveRail;
class BossKameckBattlePattern;
class KameckBeamEventListener;
class HitSensor;
class KameckBeam;

class BossKameckStateBattle : public ActorStateBase<BossKameck> {
public:
    BossKameckStateBattle(BossKameck*);

    virtual ~BossKameckStateBattle();
    virtual void init();
    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void setMoveRail(BossKameckMoveRail*);
    void setBattlePattarn(BossKameckBattlePattern*);
    bool requestDamage(HitSensor*, HitSensor*);
    bool requestGuard(HitSensor*, HitSensor*);
    bool tryAttackWait();
    bool trySummonKameck();
    bool tryChargeBram();
    void startMove();
    void exeWait();
    void exeMove();
    void exeHideMoveStart();
    void exeHideMove();
    void exeHideMoveEnd();
    void exeSummonKameckWait();
    void exeSummonKameck();
    void exeAttackWait();
    void exeAttack();
    void exeDamage();
    void exeRecover();
    void exeGuard();
    void selectStoppablePosition();
    void selectPosition();
    bool isEnableDamage() const;
    bool isEnableGuard() const;

    BossKameck* mBossKameck;                      // _C
    BossKameckMoveRail* mMoveRail;                  // _10
    BossKameckBattlePattern* mBattlePattarn;        // _14
    KameckBeam* mBeam;                              // _18
    KameckBeamEventListener* mBeamListener;         // _1C
    TVec3f _20;
    s32 _2C;
    u32 _30;
    u32 _34;
    s32 _38;
    u8 _3C;
    u8 _3D;
};

namespace NrvBossKameckStateBattle {
    NERVE(BossKameckStateBattleNrvWait);
    NERVE(BossKameckStateBattleNrvMove);
    NERVE(BossKameckStateBattleNrvHideMoveStart);
    NERVE(BossKameckStateBattleNrvHideMove);
    NERVE(BossKameckStateBattleNrvHideMoveEnd);
    NERVE(BossKameckStateBattleNrvSummonKameckWait);
    NERVE(BossKameckStateBattleNrvSummonKameck);
    NERVE(BossKameckStateBattleNrvAttackWait);
    NERVE(BossKameckStateBattleNrvAttack);
    NERVE(BossKameckStateBattleNrvDamage);
    NERVE(BossKameckStateBattleNrvRecover);
    NERVE(BossKameckStateBattleNrvGuard);
};