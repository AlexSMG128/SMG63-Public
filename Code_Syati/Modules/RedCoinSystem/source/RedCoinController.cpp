#include "RedCoinController.h"
#include "RedCoinLayouts.h"
#include "RedCoinUtil.h"
#include "RedCoin.h"

RedCoinController::RedCoinController(const char* pName) : LiveActor(pName) {
    mNumCoins = 0;
    mElapsed = 0;
    mHasAllRedCoins = false;
    mRewardCoins = false;
    mIsValidCounterAppear = false;
    mRedCoinCounter = NULL;
    mRedCoinPlayerCounter = NULL;
    mRedCoinSwitch = NULL;
    mLinkedCoins = 0;
}

void RedCoinController::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::registerDemoSimpleCastAll(this);
    MR::invalidateClipping(this);
    MR::useStageSwitchWriteA(this, rIter);
    MR::joinToGroupArray(this, rIter, "RedCoinGroup", 32);

    s32 powerStarCheck = 0;
    s32 iconID = 0x37;
    s32 layoutMode = -1;
    bool isConstant = false;
    MR::getJMapInfoArg0NoInit(rIter, &mRewardCoins);
    MR::getJMapInfoArg1NoInit(rIter, &powerStarCheck);
    MR::getJMapInfoArg2NoInit(rIter, &iconID);
    MR::getJMapInfoArg3NoInit(rIter, &layoutMode);
    MR::getJMapInfoArg4NoInit(rIter, &isConstant);

    mRedCoinCounter = new RedCoinCounter("RedCoinCounter");
    mRedCoinCounter->initWithoutIter();
    mRedCoinCounter->setStarIcon(powerStarCheck, iconID, isConstant);

    mRedCoinPlayerCounter = new RedCoinCounterPlayer("RedCoinCounterPlayer");
    mRedCoinPlayerCounter->initWithoutIter();
    
    if (layoutMode == 0) {
        mRedCoinCounter->mLayoutMode = 0;
        mRedCoinCounter->appear();
    }

    initNerve(&NrvRedCoinController::NrvWait::sInstance, 1);

    makeActorAppeared();
}

void RedCoinController::initAfterPlacement() {
    mRedCoinSwitch = (RedCoinSwitch*)RedCoinUtil::getSpecificActorFromGroup(this, "RedCoinSwitch");
    
    #ifdef SMSS
    if (mRedCoinSwitch)
        mRedCoinCounter->mUseFollowPos = true;
    #endif

    LiveActorGroup* group = MR::getGroupFromArray(this);

    for (s32 i = 0; i < group->mNumObjs; i++) {
        if (MR::isEqualString(group->getActor(i)->mName, "RedCoin")) {
            mLinkedCoins++;

            if (mRedCoinSwitch) {
                MR::hideModel(group->getActor(i));
                MR::invalidateShadowAll(group->getActor(i));
                MR::invalidateHitSensors(group->getActor(i));
            }
        }
    }
}

void RedCoinController::control() {
    setCounterVisibility();
}

void RedCoinController::allCoinsCollectedDemo() {
    if (MR::isFirstStep(this)) {
        mRedCoinCounter->setNerve(&NrvRedCoinCounter::NrvComplete::sInstance);
        MR::tryStartDemo(this, "RedCoinDemo");
    }

    if (MR::isStep(this, 150)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    
        LiveActorGroup* group = MR::getGroupFromArray(this);
        for (s32 i = 0; i < group->mNumObjs; i++) {
            if (MR::isEqualString(group->getActor(i)->mName, "RedCoin"))
                group->getActor(i)->kill();
        }

        if (mRedCoinSwitch)
            mRedCoinSwitch->mTimeLimitLayout->kill();

        MR::endDemo(this, "RedCoinDemo");
    }
}

void RedCoinController::startCountUp(LiveActor* pRedCoin) {
    mNumCoins++;
    
    mHasAllRedCoins = mNumCoins == mLinkedCoins;

    mRedCoinCounter->startCountUp(mNumCoins);

    mRedCoinPlayerCounter->mLastRedCoin = pRedCoin;
    mRedCoinPlayerCounter->mNumCoins = mNumCoins;
    mRedCoinPlayerCounter->appear();

    if (mHasAllRedCoins)
        setNerve(&NrvRedCoinController::NrvAllRedCoinsDemo::sInstance);
}


void RedCoinController::setCounterVisibility() {
    bool layoutVisibility = true;

    if (MR::isStageStateScenarioOpeningCamera() || MR::isExecScenarioStarter())
        layoutVisibility = false;

    if (MR::isPowerStarGetDemoActive())
        layoutVisibility = false;

    if (MR::isDemoActive() && !MR::isDemoActive("RedCoinDemo"))
        layoutVisibility = false;

    if (MR::isPlayerDead())
        layoutVisibility = false;

    if (MR::isTimeKeepDemoActive() && !MR::isDemoActive("RedCoinDemo"))
        layoutVisibility = false;

    if (MR::isSystemTalking() || MR::isNormalTalking())
        layoutVisibility = false;

    if (layoutVisibility)
        MR::showLayout(mRedCoinCounter);
    else
        MR::hideLayout(mRedCoinCounter);
}

void RedCoinController::appearFromSwitch() {
    mRedCoinCounter->appear();
    LiveActorGroup* group = MR::getGroupFromArray(this);

    for (s32 i = 0; i < group->mNumObjs; i++) {
        if (MR::isEqualString(group->getActor(i)->mName, "RedCoin")) {
            group->getActor(i)->appear();
        }
    }
}

void RedCoinController::resetAllRedCoins() {
    LiveActorGroup* group = MR::getGroupFromArray(this);

    for (s32 i = 0; i < group->mNumObjs; i++) {
        if (MR::isEqualString(group->getActor(i)->mName, "RedCoin")) {
            RedCoin* coin = ((RedCoin*)group->getActor(i));
            coin->appear();

            if (coin->mIsInBubble)
                coin->mAirBubble->makeActorAppeared();
        }
    }

    mNumCoins = 0;
    mIsValidCounterAppear = false;
    MR::stopAnim(mRedCoinPlayerCounter, 0);
    MR::startSystemSE("SE_SY_TIMER_A_0", -1, -1);
    mRedCoinCounter->kill();
    MR::setTextBoxNumberRecursive(mRedCoinCounter, "Counter", 0);
}

namespace NrvRedCoinController {
    void NrvWait::execute(Spine* pSpine) const {}

    void NrvAllRedCoinsDemo::execute(Spine* pSpine) const {
        ((RedCoinController*)pSpine->mExecutor)->allCoinsCollectedDemo();
    }

    NrvWait(NrvWait::sInstance);
    NrvAllRedCoinsDemo(NrvAllRedCoinsDemo::sInstance);
};