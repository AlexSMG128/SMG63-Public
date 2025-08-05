#include "RedCoinLayouts.h"
#include "RedCoin.h"

RedCoinCounter::RedCoinCounter(const char* pName) : LayoutActor(pName, false) {
    mAppearer = NULL;
    mPaneRumbler = 0;
    mRedCoinCount = 0;
    mLayoutMode = -1;
    mFollowPos = TVec2f (0.0f, 0.0f);
    mUseFollowPos = false;
    mUpdate = false;
}

void RedCoinCounter::init(const JMapInfoIter& rIter) {
    initLayoutManager("RedCoinCounter", 2);
    initEffectKeeper(0, 0, 0);

    MR::registerDemoSimpleCastAll(this);
    MR::connectToSceneLayout(this);
    MR::createAndAddPaneCtrl(this, "Counter", 1);
    MR::setTextBoxNumberRecursive(this, "Counter", 0);

    mAppearer = new CounterLayoutAppearer(this, TVec2f(-50.0f, 0.0f));

    mPaneRumbler = new CountUpPaneRumbler(this, "Counter");
    mPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;

    initNerve(&NrvRedCoinCounter::NrvHide::sInstance);

    #ifdef SMSS
        MR::createAndAddPaneCtrl(this, "CoinCounter", 1);
        MR::setFollowTypeReplace(this, "CoinCounter");
        MR::setFollowPos(&mFollowPos, this, "CoinCounter");
    #endif
}

void RedCoinCounter::control() {
    mAppearer->updateNerve();
    mPaneRumbler->update();

    #ifdef SMSS
    if (mUseFollowPos)
        MR::copyPaneTrans(&mFollowPos, this, "PositionTimer");
    else 
        MR::copyPaneTrans(&mFollowPos, this, "PositionNoTimer");
    #endif
}

void RedCoinCounter::appear() {
    mAppearer->reset();
    mPaneRumbler->reset();

    setNerve(&NrvRedCoinCounter::NrvAppear::sInstance);
    LayoutActor::appear();
}

void RedCoinCounter::setStarIcon(s32 starID, s32 iconID, bool constant) {
    if (!constant && (starID == 0 || !MR::hasPowerStarInCurrentStage(starID)))
        iconID = 0x52;

    wchar_t str;
    MR::addPictureFontCode(&str, iconID);
    MR::setTextBoxFormatRecursive(this, "TxtStar", &str);
}

void RedCoinCounter::startCountUp(s32 count) {
    mRedCoinCount = count;

    if (mAppearer->isDisappeared() && mLayoutMode == -1) {
        mUpdate = true;
        appear();
    }
    else 
        setNerve(&NrvRedCoinCounter::NrvCountUp::sInstance);
}

void RedCoinCounter::exeAppear() {
    if (MR::isFirstStep(this)) {
        mAppearer->appear(TVec2f(0.0f, 0.0f));
        MR::startAnim(this, "Wait", 1);
    }

    if (mAppearer->isAppeared()) {
        if (mUpdate)
            setNerve(&NrvRedCoinCounter::NrvCountUp::sInstance);
        else
            setNerve(&NrvRedCoinCounter::NrvWait::sInstance);
    }
}

void RedCoinCounter::exeDisappear() {
    if (MR::isFirstStep(this))
        mAppearer->disappear();

    if (MR::isStep(this, 60))
        kill();
}

void RedCoinCounter::exeCountUp() {
    if (MR::isFirstStep(this)) {
        MR::setTextBoxNumberRecursive(this, "Counter", mRedCoinCount);
        MR::emitEffect(this, "RedCoinCounterLight");
        mPaneRumbler->start();
        MR::startPaneAnim(this, "Counter", "Flash", 0);
    }

    setNerve(&NrvRedCoinCounter::NrvWait::sInstance);
}

void RedCoinCounter::exeComplete() {
    if (MR::isFirstStep(this))
        MR::startPaneAnim(this, "Counter", "FlashLoop", 0);

    if (MR::isStep(this, 120))
        setNerve(&NrvRedCoinCounter::NrvDisappear::sInstance);
}

namespace NrvRedCoinCounter {
    void NrvAppear::execute(Spine* pSpine) const {
        ((RedCoinCounter*)pSpine->mExecutor)->exeAppear();
    }

    void NrvWait::execute(Spine* pSpine) const {}

    void NrvDisappear::execute(Spine* pSpine) const {
        ((RedCoinCounter*)pSpine->mExecutor)->exeDisappear();
    }

    void NrvCountUp::execute(Spine* pSpine) const {
        ((RedCoinCounter*)pSpine->mExecutor)->exeCountUp();
    }

    void NrvComplete::execute(Spine* pSpine) const {
        ((RedCoinCounter*)pSpine->mExecutor)->exeComplete();
    }

    void NrvHide::execute(Spine* pSpine) const {}

    NrvAppear(NrvAppear::sInstance);
    NrvWait(NrvWait::sInstance);
    NrvDisappear(NrvDisappear::sInstance);
    NrvCountUp(NrvCountUp::sInstance);
    NrvComplete(NrvComplete::sInstance);
    NrvHide(NrvHide::sInstance);
}

RedCoinCounterPlayer::RedCoinCounterPlayer(const char* pName) : LayoutActor(pName, false) {
    mLastRedCoin = 0;
    mNumCoins = 0;
}

void RedCoinCounterPlayer::init(const JMapInfoIter& rIter) {
    initLayoutManager("RedCoinCounterPlayer", 2);
    MR::registerDemoSimpleCastAll(this);
    MR::connectToSceneLayout(this);
    MR::createAndAddPaneCtrl(this, "Text00", 1);
    MR::setTextBoxNumberRecursive(this, "Text00", 0);
    initNerve(&NrvRedCoinCounterPlayer::NrvHide::sInstance);
}

void RedCoinCounterPlayer::appear() {
    setNerve(&NrvRedCoinCounterPlayer::NrvAppear::sInstance);
    LayoutActor::appear();
}

void RedCoinCounterPlayer::kill() {
    setNerve(&NrvRedCoinCounterPlayer::NrvHide::sInstance);
    LayoutActor::kill();
}

void RedCoinCounterPlayer::exeAppear() {
    calcScreenPos();

    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
        MR::setTextBoxNumberRecursive(this, "TxtText", mNumCoins);
    }
    
    MR::killAtAnimStopped(this, 0);
}
 
void RedCoinCounterPlayer::calcScreenPos() {
    TVec3f gravity;
    TVec3f pos;
    f32 heightAdd;

    if (((RedCoin*)mLastRedCoin)->mRedCoinCounterPlayerPos) {
        gravity = *MarioAccess::getPlayerActor()->getGravityVec();
        pos = *MR::getPlayerPos();
        heightAdd = 200.0f;
    }
    else {
        gravity = mLastRedCoin->mGravity;
        pos = mLastRedCoin->mTranslation;
        heightAdd = 150.0f;
    }

    TVec2f screenPos;
    TVec3f newPos;

    JMAVECScaleAdd((Vec*)&gravity, (Vec*)&pos, (Vec*)&newPos, -heightAdd);
    MR::calcScreenPosition(&screenPos, newPos);
    MR::clamp(screenPos.x, 0, (f32)MR::getScreenWidth());
    MR::clamp(screenPos.y, 0, (f32)MR::getScreenHeight());

    setTrans(screenPos);
}

namespace NrvRedCoinCounterPlayer {
    void NrvRedCoinCounterPlayer::NrvHide::execute(Spine* pSpine) const {}

    void NrvRedCoinCounterPlayer::NrvAppear::execute(Spine* pSpine) const {
        ((RedCoinCounterPlayer*)pSpine->mExecutor)->exeAppear();
    }

    NrvHide(NrvHide::sInstance);
    NrvAppear(NrvAppear::sInstance);
}