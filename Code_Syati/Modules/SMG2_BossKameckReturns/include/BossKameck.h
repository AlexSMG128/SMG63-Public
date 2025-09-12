#pragma once

#include "Game/LiveActor/ActorJointCtrl.h"
#include "Game/LiveActor/LiveActor.h"
#include "Game/Enemy/KameckBeam.h"
#include "Game/Enemy/KameckBeamHolder.h"
#include "BossKameckBattlePattern.h"
#include "BossKameckFollowerGroup.h"
#include "BossKameckMoveRail.h"
#include "BossKameckSequencer.h"

class BossKameckBeamEventListener;

class BossKameck : public LiveActor {
public:
    BossKameck(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);


    void initBattle(const JMapInfoIter&);
    void initFollowers(const JMapInfoIter&);
    void initMoveRail(const JMapInfoIter&);
    LiveActor* initBarrier(const JMapInfoIter&, const char*);
    void initStar();
    void initBeam();

    BossKameckFollowerGroup* getFollowerGroup(s32) const;
    BossKameckMoveRail* getMoveRail(s32);
    s32 getLivingKameckNum() const;
    KameckBeamHolder* tryGetBeamHolder(BossKameckBattlePattern*);

    void startSequence();
    // Returns false when no more patterns are available.
    bool tryGoNextPattern();
    const char* tryGetCurrentAnimName() const;
    bool isNextNeedPowerUpDemo() const;
    BossKameckFollowerGroup* tryGetCurrentFollowerGroup() const;
    KameckBeamHolder* tryGetCurrentBeamHolder();

    void startDemoAppearKameck();
    void endDemoAppearKameck();
    void appearKameck(s32);
    void deadKameck();
    void killAllBeam();

    void startDemo();
    void endDemo();

    void setPose(MtxPtr);
    void updatePose();
    void hitBeam(s32);
    void appearStarPieceToUp(s32 num);
    void appearStarPieceToPlayer(s32 num);


    BossKameckBattlePattern** mBattlePatterns;
    s32 mBattlePatternCount;
    s32 mCurrentBattlePattern;

    BossKameckFollowerGroup** mKameckFollowerGroupList;
    s32 mKameckFollowerGroupListCount;

    BossKameckMoveRail** mMoveRails;
    s32 mMoveRailCount;

    BossKameckBarrierEntry** mBarrierList;
    s32 mBarrierListCount;

    ActorJointCtrl* mJointCtrl;
    s32 mPowerStarId;
    s32 mCameraVectorId;

    // uhhhhhhhhh stolen from SMG1 lol
    TQuat4f _90;
    TVec3f _A0;
    TVec3f _C8;

    BossKameckSequencer* mSequencer;
    BossKameckBeamEventListener* mBeamListener;

    KameckBeamHolder** mMasterBeamList;
    s32 mMasterBeamListCount;

    bool mIsUseLv2Anim;
};

class BossKameckBeamEventListener : public KameckBeamEventListener {
public:
    inline BossKameckBeamEventListener(BossKameck* pBoss) : KameckBeamEventListener() {
        mBossKameck = pBoss;
    }

    virtual void hitBeam(s32 type) {
        mBossKameck->hitBeam(type);
    }

    BossKameck* mBossKameck;        // _4
};