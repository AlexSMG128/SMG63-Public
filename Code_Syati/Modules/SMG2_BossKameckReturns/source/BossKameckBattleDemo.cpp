#include "BossKameckBattleDemo.h"
#include "BossKameck.h"

#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActorCameraUtil.h"
#include "Game/Util/ActorInitUtil.h"
#include "Game/Util/ActorShadowUtil.h"
#include "Game/Util/BgmUtil.h"
#include "Game/Util/CameraUtil.h"
#include "Game/Util/DemoUtil.h"
#include "Game/Util/JointUtil.h"
#include "Game/Util/LiveActorUtil.h"
#include "Game/Util/PlayerUtil.h"
// TODO: finish writing actual includes...

#include "syati.h"

BossKameckDemoPosition::BossKameckDemoPosition() : LiveActor("キャスト位") {
    makeActorDead();
}

void BossKameckDemoPosition::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    initModelManagerWithAnm("BossKameckBattleDemo", NULL, NULL, false);
    mCameraInfo = MR::createActorCameraInfo(rIter);

    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckStart");
    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckPowerUp");
    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckDown");
    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckStart2");
    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckPowerUp2");
    MR::initAnimCamera(this, mCameraInfo, "DemoBossKameckDown2");
    MR::invalidateClipping(this);
    makeActorDead();

    mHomePosition = mTranslation;
    mHomeRotation = mRotation;
}

// =======================================================================================================================================

BossKameckBattleDemo::BossKameckBattleDemo(BossKameck* pBoss, const JMapInfoIter& rIter) : ActorStateBase<BossKameck>("ボスカメック戦デモ") {
    mBossKameck = pBoss;
    mDemoPos = NULL;
    mCurDemoName = NULL;
    mDemoNerve = NULL;
    mDemoPos = new BossKameckDemoPosition();
    mDemoPos->init(rIter);
}

void BossKameckBattleDemo::init() {
    initNerve(&NrvBossKameckBattleDemo::BossKameckBattleDemoNrvWaitAppear::sInstance, 0);
}

void BossKameckBattleDemo::appear() {
    mIsDead = false;
    mDemoPos->appear();
}

void BossKameckBattleDemo::kill() {
    mIsDead = true;
    mDemoPos->kill();
}



void BossKameckBattleDemo::updateCastPose() {
    mDemoPos->movement();
    mDemoPos->calcAnim();
    MR::setPlayerBaseMtx(MR::getJointMtx(mDemoPos, "MarioPosition"));
    BossKameck* boss = mBossKameck;
    boss->setPose(MR::getJointMtx(mDemoPos, "KameckPosition"));
}

void BossKameckBattleDemo::startTryDemo(const char* pName, const Nerve* pNerve) {
    appear();
    if (MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(mBossKameck, pName)) { //TODO: Fix this to use normal tryStartDemoMarioPuppetable (located at 8001F950 ?)
        MR::overlayWithPreviousScreen(2);
        setNerve(pNerve);
    }
    else {
        mDemoNerve = pNerve;
        mCurDemoName = pName;
        setNerve(&NrvBossKameckBattleDemo::BossKameckBattleDemoNrvTryStartDemo::sInstance);
    }
}



void BossKameckBattleDemo::exeTryStartDemo() {
    if (MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(mBossKameck, mCurDemoName)) { //TODO: Fix this to use normal tryStartDemoMarioPuppetable (located at 8001F950 ?)
        MR::overlayWithPreviousScreen(2);
        setNerve(mDemoNerve);
        mCurDemoName = NULL;
        mDemoNerve = NULL;
    }
}

void BossKameckBattleDemo::exeWaitAppear() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BattleWait", (const char*)NULL);
        MR::stopStageBGM(0x5A);
        mBossKameck->startDemo();
    }

    if (MR::isGreaterStep(this, 120)) {
        setNerve(&NrvBossKameckBattleDemo::BossKameckBattleDemoNrvAppear::sInstance);
    }
}

void BossKameckBattleDemo::exeAppear() {

    if (mBossKameck->mIsUseLv2Anim)
    {
        if (MR::isFirstStep(this)) {
            MR::overlayWithPreviousScreen(1);
            MR::showModel(mBossKameck);
            BossKameckDemoPosition* pos = mDemoPos;
            const char* demoName = "DemoBossKameckStart2";
            pos->makeActorAppeared();
            MR::startAnimCameraTargetSelf(pos, pos->mCameraInfo, demoName, 0, false, 1.0f);
            MR::startBck(pos, "DemoBossKameckStart2", NULL);
            MR::startAction(mBossKameck, "DemoBossKameckStart2");
            MR::startStageBGM("MBGM_BOSS_04", false); // TODO: Change to use actual SMG1 song ID?
        }

        if (MR::isStep(this, 60)) {
            MR::startSound(mBossKameck, "SE_BV_KAMECK_APPEAR", -1, -1);
        }

        if (MR::isLessStep(this, 155)) {
            MR::startLevelSound(mBossKameck, "SE_BM_LV_KAMECK_DEMO_APPEAR", -1, -1, -1);
        }

        if (MR::isStep(this, 155)) {
            MR::startSound(mBossKameck, "SE_BM_KAMECK_DEMO_APPEA_END", -1, -1);
            MR::validateShadowAll(mBossKameck);
        }

        updateCastPose();
        if (MR::isActionEnd(mBossKameck)) {
            BossKameckDemoPosition* pos = mDemoPos;
            MR::endAnimCamera(pos, pos->mCameraInfo, "DemoBossKameckStart2", 0, true);
            pos->makeActorDead();
            MR::endDemo(mBossKameck, "カメック登場Vs2");
            mBossKameck->endDemo();
            kill();
        }
    }
    else
    {
        const char* demoName = "DemoBossKameckStart";
        if (MR::isFirstStep(this)) {
            MR::overlayWithPreviousScreen(1);
            MR::showModel(mBossKameck);
            BossKameckDemoPosition* pos = mDemoPos;
            pos->makeActorAppeared();
            MR::startAnimCameraTargetSelf(pos, pos->mCameraInfo, demoName, 0, false, 1.0f);
            MR::startBck(pos, demoName, NULL);
            MR::startAction(mBossKameck, demoName);
            MR::startStageBGM("MBGM_BOSS_04", false); // TODO: Change to use actual SMG1 song ID?
        }

        if (MR::isLessStep(this, 270)) {
            MR::startLevelSound(mBossKameck, "SE_BM_LV_KAMECK_DEMO_PRE_APPEAR", -1, -1, -1);
        }

        if (MR::isStep(this, 270)) {
            MR::startSound(mBossKameck, "SE_BM_KAMECK_DEMO_SMOKE", -1, -1);
            MR::startSound(mBossKameck, "SE_BV_KAMECK_APPEAR", -1, -1);
            MR::validateShadowAll(mBossKameck);
        }

        updateCastPose();
        if (MR::isActionEnd(mBossKameck)) {
            BossKameckDemoPosition* pos = mDemoPos;
            MR::endAnimCamera(pos, pos->mCameraInfo, demoName, 0, true);
            pos->makeActorDead();
            MR::endDemo(mBossKameck, "カメック登場Appear");
            mBossKameck->endDemo();
            kill();
        }
    }
}

void BossKameckBattleDemo::exePowerUp() {
    const char* powerUpName = mBossKameck->tryGetCurrentAnimName();
    if (powerUpName == NULL)
        powerUpName = "DemoBossKameckPowerUp";
    if (MR::isFirstStep(this)) {
        BossKameckDemoPosition* pos = mDemoPos;
        TVec3f NamePosition, NameRotation;
        if (MR::tryFindNamePos(powerUpName, &NamePosition, &NameRotation))
        {
            pos->mTranslation.set(NamePosition);
            pos->mRotation.set(NameRotation);
        }
        else
        {
            pos->mTranslation.set(pos->mHomePosition);
            pos->mRotation.set(pos->mHomeRotation);
        }
        pos->makeActorAppeared();
        MR::startAnimCameraTargetSelf(pos, pos->mCameraInfo, powerUpName, 0, false, 1.0f);
        MR::startBck(pos, powerUpName, NULL);
        MR::startAction(mBossKameck, powerUpName);
        MR::startBckPlayer("BattleWait", (const char*)0);
        mBossKameck->startDemo();
    }

    if (MR::isStep(this, 200)) {
        mBossKameck->startDemoAppearKameck();
    }

    updateCastPose();

    if (MR::isActionEnd(mBossKameck)) {
        MR::endDemo(mBossKameck, "カメックパワーアップPowerUp");
        BossKameckDemoPosition* pos = mDemoPos;
        MR::endAnimCamera(pos, pos->mCameraInfo, powerUpName, 0, true);
        pos->makeActorDead();
        mBossKameck->endDemoAppearKameck();
        mBossKameck->endDemo();
        kill();
    }
}

void BossKameckBattleDemo::exeDown() {
    const char* powerUpName = mBossKameck->mIsUseLv2Anim ? "DemoBossKameckDown2":"DemoBossKameckDown";
    if (MR::isFirstStep(this)) {
        BossKameckDemoPosition* pos = mDemoPos;
        TVec3f NamePosition, NameRotation;
        if (MR::tryFindNamePos(powerUpName, &NamePosition, &NameRotation))
        {
            pos->mTranslation.set(NamePosition);
            pos->mRotation.set(NameRotation);
        }
        else
        {
            pos->mTranslation.set(pos->mHomePosition);
            pos->mRotation.set(pos->mHomeRotation);
        }
        pos->makeActorAppeared();
        MR::startAnimCameraTargetSelf(pos, pos->mCameraInfo, powerUpName, 0, false, 1.0f);
        MR::startBck(pos, powerUpName, NULL);
        MR::startAction(mBossKameck, powerUpName);
        mBossKameck->deadKameck();
        mBossKameck->startDemo();
        MR::startBckPlayer("BattleWait", (const char*)0);
        MR::stopStageBGM(30);
    }

    updateCastPose();

    if (MR::isStep(this, 18)) {
        MR::startSound(mBossKameck, "SE_BV_KAMECK_DEAD", -1, -1);
    }

    if (MR::isStep(this, 150)) {
        MR::startSound(mBossKameck, "SE_BM_KAMECK_DEAD", -1, -1);
    }

    if (MR::isStep(this, 285)) {
        //MR::startAfterBossBGM();
    }

    if (MR::isStep(this, 345)) {
        MR::startSound(mBossKameck, "SE_BM_KAMECK_DEMO_STAFF_BREAK", -1, -1);
        MR::startSystemSE("SE_SY_POW_STAR_APPEAR", -1, -1);
    }

    if (MR::isActionEnd(mBossKameck)) {
        MR::endDemo(mBossKameck, "ボスカメックダウンDown");
        BossKameckDemoPosition* pos = mDemoPos;
        MR::endAnimCamera(pos, pos->mCameraInfo, powerUpName, 0, true);
        pos->makeActorDead();
        if (mBossKameck->mPowerStarId > 0)
            MR::appearPowerStarWithoutDemo(mBossKameck);
        mBossKameck->endDemo();
        mBossKameck->kill();
        kill();
    }
}


void BossKameckBattleDemo::startDemoAppear() {
    startTryDemo("カメック登場Appear", &NrvBossKameckBattleDemo::BossKameckBattleDemoNrvWaitAppear::sInstance);
    MR::hideModelAndOnCalcAnim(mBossKameck);
    MR::invalidateShadowAll(mBossKameck);
}

void BossKameckBattleDemo::startDemoPowerUp() {
    startTryDemo("カメックパワーアップPowerUp", &NrvBossKameckBattleDemo::BossKameckBattleDemoNrvPowerUp::sInstance);
}

void BossKameckBattleDemo::startDemoDown() {
    startTryDemo("ボスカメックダウンDown", &NrvBossKameckBattleDemo::BossKameckBattleDemoNrvDown::sInstance);
}


BossKameckBattleDemo::~BossKameckBattleDemo() {

}
BossKameckDemoPosition::~BossKameckDemoPosition() {

}

namespace NrvBossKameckBattleDemo {
    void BossKameckBattleDemoNrvTryStartDemo::execute(Spine* pSpine) const {
        ((BossKameckBattleDemo*)pSpine->mExecutor)->exeTryStartDemo();
    }
    BossKameckBattleDemoNrvTryStartDemo(BossKameckBattleDemoNrvTryStartDemo::sInstance);


    void BossKameckBattleDemoNrvWaitAppear::execute(Spine* pSpine) const {
        ((BossKameckBattleDemo*)pSpine->mExecutor)->exeWaitAppear();
    }
    BossKameckBattleDemoNrvWaitAppear(BossKameckBattleDemoNrvWaitAppear::sInstance);


    void BossKameckBattleDemoNrvAppear::execute(Spine* pSpine) const {
        ((BossKameckBattleDemo*)pSpine->mExecutor)->exeAppear();
    }
    BossKameckBattleDemoNrvAppear(BossKameckBattleDemoNrvAppear::sInstance);


    void BossKameckBattleDemoNrvPowerUp::execute(Spine* pSpine) const {
        ((BossKameckBattleDemo*)pSpine->mExecutor)->exePowerUp();
    }
    BossKameckBattleDemoNrvPowerUp(BossKameckBattleDemoNrvPowerUp::sInstance);


    void BossKameckBattleDemoNrvDown::execute(Spine* pSpine) const {
        ((BossKameckBattleDemo*)pSpine->mExecutor)->exeDown();
    }
    BossKameckBattleDemoNrvDown(BossKameckBattleDemoNrvDown::sInstance);
};