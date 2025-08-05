#include "BlueCoinCounter.h"
#include "BlueCoinUtil.h"
#include "Game/Screen/CounterLayoutControllerExt.h"
#include "Game/Screen/GameSceneLayoutHolder.h"
#include "BlueCoinStageProgress.h"


#if defined TWN || defined KOR
#define REGIONOFF 0x10
#else
#define REGIONOFF 0
#endif


BlueCoinCounter::BlueCoinCounter(const char* pName) : LayoutActor(pName, 0) {
    mAppearer = NULL;
    mPaneRumbler = NULL;
    mSysInfoWindow = NULL;
    mBlueCoinCount = 0;
    mBlueCoinDisplayNum = 0;
    _3C = 0;
    mIsAppear = 0;
}

void BlueCoinCounter::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinCounterInStage", 2);
    initEffectKeeper(0, 0, 0);
    MR::connectToSceneLayout(this);
    MR::registerDemoSimpleCastAll(this);
    MR::createAndAddPaneCtrl(this, "Counter", 1);
    s32 blueCoinNum = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true);
    MR::setTextBoxNumberRecursive(this, "Counter", blueCoinNum);
    mBlueCoinDisplayNum = blueCoinNum;

    #ifdef SMSS
        mAppearer = new CounterLayoutAppearer(this, TVec2f(0.0f, 0.0f));
    #else
        mAppearer = new CounterLayoutAppearer(this, TVec2f(-50.0f, 0.0f));
    #endif

    mPaneRumbler = new CountUpPaneRumbler(this, "Counter");
    mPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;

    initNerve(&NrvBlueCoinCounter::NrvHide::sInstance);   

    if (!BlueCoinUtil::hasSeenBlueCoinTextBoxCurrentFile()) {
        mSysInfoWindow = MR::createSysInfoWindowMiniExecuteWithChildren();
        MR::connectToSceneLayout(mSysInfoWindow);
        MR::registerDemoSimpleCastAll(mSysInfoWindow);
    }

    appear();
}

void BlueCoinCounter::appear() {
    mAppearer->reset();
    mPaneRumbler->reset();
    _3C = 0;
    mIsAppear = 0;
    MR::hideLayout(this);
    setNerve(&NrvBlueCoinCounter::NrvHide::sInstance);
    LayoutActor::appear();
}

void BlueCoinCounter::forceAppear() {
    if (!isNerve(&NrvBlueCoinCounter::NrvWait::sInstance)) {
        appear();
        setNerve(&NrvBlueCoinCounter::NrvAppear::sInstance);
    }

    mIsAppear = true;
}

void BlueCoinCounter::disappear() {
    mIsAppear = false;
    setNerve(&NrvBlueCoinCounter::NrvDisappear::sInstance);
}

void BlueCoinCounter::control() {
    updateCounter();
    mAppearer->updateNerve();
    mPaneRumbler->update();
}

void BlueCoinCounter::updateCounter() {
    s32 blueCoinNum = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true);
    s32 var = _3C;
    mBlueCoinCount = blueCoinNum;

    if (var > 0) {
        _3C = var - 1;
    }
    else {
        if (mBlueCoinDisplayNum != blueCoinNum && !isNerve(&NrvBlueCoinCounter::NrvShowTextBox::sInstance)) {
            if (isNerve(&NrvBlueCoinCounter::NrvWait::sInstance)) {
                u32 v4 = mBlueCoinDisplayNum;
                _3C = 3;

                if (mBlueCoinDisplayNum < blueCoinNum)
                    mBlueCoinDisplayNum = v4 + 1;
                else 
                    mBlueCoinDisplayNum = v4 - 1;

                MR::startAnim(this, "Flash", 0);
                MR::emitEffect(this, "BlueCoinCounterInStageLight");
                mPaneRumbler->start();
            }

            if (!isNerve(&NrvBlueCoinCounter::NrvAppear::sInstance)) {
                if (!isNerve(&NrvBlueCoinCounter::NrvWait::sInstance))
                    setNerve(&NrvBlueCoinCounter::NrvAppear::sInstance);
                else
                    setNerve(&NrvBlueCoinCounter::NrvWait::sInstance);
            }
        }
    }

    MR::setTextBoxNumberRecursive(this, "Counter", mBlueCoinDisplayNum);
}

void BlueCoinCounter::setCounter() {
    s32 blueCoinNum = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true);
    mBlueCoinDisplayNum = blueCoinNum;
    mBlueCoinCount = blueCoinNum;
}

void BlueCoinCounter::exeHide() {
    if (MR::isFirstStep(this)) {
        _3C = 0;
        MR::hideLayout(this);
    }
}

void BlueCoinCounter::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        mAppearer->appear(TVec2f(0.0f, 0.0f));
        MR::startAnim(this, "Wait", 1);
    }

    if (mAppearer->isAppeared()) {
        setNerve(&NrvBlueCoinCounter::NrvWait::sInstance);
    }
}

void BlueCoinCounter::exeWait() {
    if (!mIsAppear && mBlueCoinDisplayNum == mBlueCoinCount) {
        if (CounterLayoutController::isWaitToDisappearCounter(this)) {
            setNerve(&NrvBlueCoinCounter::NrvDisappear::sInstance);
        }
    }
}

void BlueCoinCounter::exeDisappear() {
    if (MR::isFirstStep(this)) {
        mAppearer->disappear();
    }

    if (mAppearer->isDisappeared()) {
        setNerve(&NrvBlueCoinCounter::NrvHide::sInstance);
    }
}

void BlueCoinCounter::exeShowTextBox() {
    if (MR::isFirstStep(this)) {
        MR::hideLayout(this);
        
        if (MR::canStartDemo()) {
            DemoStartRequestUtil::startDemoSystem((LiveActor*)this, "BlueCoinFirstTimeText", 1, DemoStartInfo::DEMOTYPE_0, DemoStartInfo::CINEMAFRAMETYPE_1, DemoStartInfo::STARPOINTERTYPE_0, DemoStartInfo::DELETEEFFECTYPE_0, 0);
            MR::requestMovementOn(this);
        }
    }

    if (MR::isStep(this, 3))
        mSysInfoWindow->appear("BlueCoinCounter_OnFirstBlueCoin", SysInfoWindow::SysInfoType_0, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);

    if (mSysInfoWindow->isDisappear() && MR::isDead(mSysInfoWindow)) {
        MR::endDemo(this, "BlueCoinFirstTimeText");
        BlueCoinUtil::setSeenBlueCoinTextBoxCurrentFile();
        setNerve(&NrvBlueCoinCounter::NrvAppear::sInstance);
    }
}

BlueCoinCounter::~BlueCoinCounter() {

}

CounterLayoutControllerExt::CounterLayoutControllerExt() : CounterLayoutController() {
    mBlueCoinCounter = 0;
    mBlueCoinStageProgress = 0;
}

CounterLayoutControllerExt* createCounterLayoutControllerExt() {
    return new CounterLayoutControllerExt();
}

kmCall(0x80471784, createCounterLayoutControllerExt); // bl createCounterLayoutControllerExt
kmWrite32(0x80471788, 0x4800000C); // b 0xC

namespace NrvBlueCoinCounter {
	void NrvHide::execute(Spine* pSpine) const {
        ((BlueCoinCounter*)pSpine->mExecutor)->exeHide();
    }

	void NrvAppear::execute(Spine* pSpine) const {
        ((BlueCoinCounter*)pSpine->mExecutor)->exeAppear();
    }

	void NrvWait::execute(Spine* pSpine) const {
        ((BlueCoinCounter*)pSpine->mExecutor)->exeWait();
    }

	void NrvDisappear::execute(Spine* pSpine) const {
        ((BlueCoinCounter*)pSpine->mExecutor)->exeDisappear();
    }

	void NrvShowTextBox::execute(Spine* pSpine) const {
        ((BlueCoinCounter*)pSpine->mExecutor)->exeShowTextBox();
    }

	NrvHide(NrvHide::sInstance);
    NrvAppear(NrvAppear::sInstance);
    NrvWait(NrvWait::sInstance);
    NrvDisappear(NrvDisappear::sInstance);
    NrvShowTextBox(NrvShowTextBox::sInstance);
}

void createBlueCoinCounter(CounterLayoutControllerExt* pController, const Nerve* pNerve) { 
    BlueCoinCounter* pCounter = new BlueCoinCounter("BlueCoinCounter");
    pCounter->initWithoutIter();

    pController->mBlueCoinCounter = pCounter;

    BlueCoinStageProgress* pStageProgress = new BlueCoinStageProgress("BlueCoinStageProgress");
    pStageProgress->initWithoutIter();

    pController->mBlueCoinStageProgress = pStageProgress;

    pController->initNerve(pNerve);
}

kmCall(0x804657AC, createBlueCoinCounter);

bool isBlueCoinCounterDead(CounterLayoutControllerExt* pController) {
    return (MR::isDead(pController->mCoinCounter) || MR::isDead(pController->mBlueCoinCounter));
}

kmWrite32(0x80465C00, 0x7F23CB78); // mr r3, r25
kmCall(0x80465C04, isBlueCoinCounterDead);

bool isBlueCoinCounterHidden(CounterLayoutControllerExt* pController) {
    return (MR::isHiddenLayout(pController->mCoinCounter) || MR::isHiddenLayout(pController->mBlueCoinCounter));
}

kmWrite32(0x80465C10, 0x7F23CB78); // mr r3, r25
kmCall(0x80465C14, isBlueCoinCounterHidden);


bool isBlueCoinCounterWait(CounterLayoutControllerExt* pController) {
    return (pController->mCoinCounter->isWait() || pController->mBlueCoinCounter->isNerve(&NrvBlueCoinCounter::NrvWait::sInstance));
}

kmWrite32(0x80465C20, 0x7F23CB78); // mr r3, r25
kmCall(0x80465C24, isBlueCoinCounterWait); // bl isBlueCoinCounterWait


void appearBlueCoinLayout(CounterLayoutControllerExt* pController) {
    if (pController->mBlueCoinCounter) {
        pController->mBlueCoinCounter->forceAppear();
    }

    pController->showAllLayout();
}


kmCall(0x80466128, appearBlueCoinLayout);


void appearBlueCoinLayoutWithoutStar(CounterLayoutControllerExt* pController) {
    pController->mCoinCounter->appear();

    if (pController->mBlueCoinCounter) {
        pController->mBlueCoinCounter->appear();
    }
}

kmCall(0x80465EC4, appearBlueCoinLayoutWithoutStar); // bl appearBlueCoinLayoutWithoutStar
kmWrite32(0x80465EC8, 0x48000010); // b 0x10

void disappearBlueCoinLayout(CounterLayoutControllerExt* pController) {
    if (pController->mBlueCoinCounter) {
        pController->mBlueCoinCounter->disappear();
    }
        
    pController->hideAllLayout();
}

kmCall(0x80466198, disappearBlueCoinLayout);

void killBlueCoinCounter(CounterLayoutControllerExt* pController) {
    if (pController->mBlueCoinCounter && !MR::isDemoActive("BlueCoinFirstTimeText")) {
        pController->mBlueCoinCounter->kill();
    }

    if (pController->mBlueCoinStageProgress && !MR::isDemoActive("BlueCoinFirstTimeText")) {
        pController->mBlueCoinStageProgress->kill();
    }

    pController->killAllCoounter();
}

kmCall(0x8046590C, killBlueCoinCounter); // bl killBlueCoinCounter