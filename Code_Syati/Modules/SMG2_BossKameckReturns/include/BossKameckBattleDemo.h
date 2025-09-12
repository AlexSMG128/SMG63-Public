#pragma once

#include "Game/LiveActor/ActorStateBase.h"
#include "Game/LiveActor/LiveActor.h"
#include "Game/LiveActor/ActorCameraInfo.h"

class BossKameck;

class BossKameckDemoPosition : public LiveActor {
public:
    BossKameckDemoPosition();

    virtual ~BossKameckDemoPosition();
    virtual void init(const JMapInfoIter&);

    ActorCameraInfo* mCameraInfo;           // _8C
    TVec3f mHomePosition;
    TVec3f mHomeRotation;
};

class BossKameckBattleDemo : public ActorStateBase<BossKameck> {
public:
    BossKameckBattleDemo(BossKameck*, const JMapInfoIter&);

    virtual ~BossKameckBattleDemo();
    virtual void init();
    virtual void appear();
    virtual void kill();

    void updateCastPose();
    void startTryDemo(const char*, const Nerve*);
    void startDemoAppear();
    void startDemoPowerUp();
    void startDemoDown();

    void exeTryStartDemo();
    void exeWaitAppear();
    void exeAppear();
    void exePowerUp();
    void exeDown();


    BossKameck* mBossKameck;
    BossKameckDemoPosition* mDemoPos;
    const Nerve* mDemoNerve;
    const char* mCurDemoName;
};

namespace NrvBossKameckBattleDemo {
    NERVE(BossKameckBattleDemoNrvTryStartDemo);
    NERVE(BossKameckBattleDemoNrvWaitAppear);
    NERVE(BossKameckBattleDemoNrvAppear);
    NERVE(BossKameckBattleDemoNrvPowerUp);
    NERVE(BossKameckBattleDemoNrvDown);
};