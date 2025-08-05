#include "RedCoinSwitch.h"
#include "RedCoinController.h"
#include "RedCoinUtil.h"

RedCoinSwitch::RedCoinSwitch(const char* pName) : LiveActor(pName) {
    mRedCoinController = 0;
    mTime = 0;
    mMode = 0;
}

void RedCoinSwitch::init(const JMapInfoIter& rIter) {
    MR::processInitFunction(this, rIter, false);
    MR::joinToGroupArray(this, rIter, "RedCoinGroup", 32);
    initNerve(&NrvRedCoinSwitch::NrvWait::sInstance, 0);
    initHitSensor(1);
	MR::addHitSensorMapObj(this, "Switch", 1, 75.0f, TVec3f(0.0f, 150.0f, 0.0f));
    MR::initCollisionParts(this, "RedCoinSwitch", getSensor("Switch"), (MtxPtr)getBaseMtx());
    MR::getJMapInfoArg0NoInit(rIter, &mTime);
    MR::getJMapInfoArg1NoInit(rIter, &mMode);

    mTimeLimitLayout = new TimeLimitLayout();

    MR::connectToSceneLayout(mTimeLimitLayout);
    mTimeLimitLayout->setDisplayModeOnNormal(1);
    mTimeLimitLayout->initWithoutIter();
  
    makeActorAppeared();
}

void RedCoinSwitch::initAfterPlacement() {
    mRedCoinController = (RedCoinController*)RedCoinUtil::getSpecificActorFromGroup(this, "RedCoinController");
}

void RedCoinSwitch::exePress() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::invalidateCollisionParts(this);
        MR::invalidateClipping(this);

        mTimeLimitLayout->setTimeLimit(mTime * 60);
        mTimeLimitLayout->appear();

        if (MR::isValidSwitchA(this))
            MR::onSwitchA(this);

        if (mRedCoinController) {
            mRedCoinController->appearFromSwitch();
        }
    }
}

void RedCoinSwitch::exeRelease() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Release");
        MR::validateCollisionParts(this);
        MR::invalidateClipping(this);
        setNerve(&NrvRedCoinSwitch::NrvWait::sInstance);
    }
}

void RedCoinSwitch::control() {
    if (mTimeLimitLayout->isReadyToTimeUp()) {
        if (mMode == 0) {
            MR::forceKillPlayerByGroundRace();
        }
        if (mMode == 1 && mRedCoinController) {
            mRedCoinController->resetAllRedCoins();
            setNerve(&NrvRedCoinSwitch::NrvRelease::sInstance);
        }

        mTimeLimitLayout->kill();
    }

    if (MR::isPowerStarGetDemoActive())
        mTimeLimitLayout->kill();
}

bool RedCoinSwitch::receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
	if (MR::isMsgPlayerHipDropFloor(msg) && !isNerve(&NrvRedCoinSwitch::NrvPress::sInstance))
        setNerve(&NrvRedCoinSwitch::NrvPress::sInstance);

	return false;
}

namespace NrvRedCoinSwitch {
    void NrvWait::execute(Spine* pSpine) const {
    }

    void NrvPress::execute(Spine* pSpine) const {
        ((RedCoinSwitch*)pSpine->mExecutor)->exePress();  
    }

    void NrvRelease::execute(Spine* pSpine) const {
        ((RedCoinSwitch*)pSpine->mExecutor)->exeRelease(); 
    }

    NrvWait(NrvWait::sInstance);
    NrvPress(NrvPress::sInstance);
    NrvRelease(NrvRelease::sInstance);
}
