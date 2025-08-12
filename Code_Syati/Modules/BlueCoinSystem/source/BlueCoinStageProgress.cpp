#include "BlueCoinStageProgress.h"
#include "GeneralUtil.h"

BlueCoinStageProgress::BlueCoinStageProgress(const char* pName) : LayoutActor(pName, false) {
    mTargetID = -1;
    mSlideFrame = 0.0f;
    mAppearer = 0;
    mPaneRumbler = 0;
    mIsTargetIDGot = false;
}

void BlueCoinStageProgress::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinStageProgress", 2);
    initNerve(&NrvBlueCoinStageProgress::NrvHide::sInstance);
    initEffectKeeper(0, 0, 0);
    MR::connectToSceneLayout(this);
    MR::registerDemoSimpleCastAll(this);
    MR::createAndAddPaneCtrl(this, "Text00", 1);
    MR::createAndAddPaneCtrl(this, "Rumble", 1);
    MR::createAndAddPaneCtrl(this, "Particle", 1);
    MR::setTextBoxFormatRecursive(this, "ShaText", L"TEST");
    MR::setTextBoxFormatRecursive(this, "TextAnm", L"");
    MR::setTextBoxFormatRecursive(this, "ShaNum", L"");
    MR::registerDemoSimpleCastAll(this);
    updateTextBox();

    mAppearer = new CounterLayoutAppearer(this, TVec2f(0.0f, 50.0f));

    mPaneRumbler = new CountUpPaneRumbler(this, "Rumble");
    mPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;
}

void BlueCoinStageProgress::appear() {
    if (mAppearer->isAppeared())
        setNerve(&NrvBlueCoinStageProgress::NrvUpdate::sInstance);
    else {
        if (!isNerve(&NrvBlueCoinStageProgress::NrvAppear::sInstance))
            setNerve(&NrvBlueCoinStageProgress::NrvAppear::sInstance);
    }
    LayoutActor::appear();
}

void BlueCoinStageProgress::control() {
    mAppearer->updateNerve();
    mPaneRumbler->update();
}

void BlueCoinStageProgress::exeAppear() {
    if (MR::isFirstStep(this)) {
        mAppearer->reset();
        mPaneRumbler->reset();
        
        mAppearer->appear(TVec2f(0.0f, 0.0f));
    }
    
    if (mAppearer->isAppeared())
        setNerve(&NrvBlueCoinStageProgress::NrvUpdate::sInstance);
}

void BlueCoinStageProgress::exeWait() {
    if (mAppearer->isAppeared() && CounterLayoutController::isWaitToDisappearCounter(this) && !MR::isDemoActive("BlueCoinFirstTimeText"))
        setNerve(&NrvBlueCoinStageProgress::NrvDisappear::sInstance);
}

void BlueCoinStageProgress::exeUpdate() {
    if (MR::isFirstStep(this)) {
        calcSlidePaneFrame();
        MR::startAnimAndSetFrameAndStop(this, "SlidePane", mSlideFrame, 1);
        updateTextBox();
        updateAnimBox();
        updateNumBox();
        mPaneRumbler->start();

        if (!mIsTargetIDGot)
            MR::emitEffect(this, "BlueCoinStageProgressLight");

        setNerve(&NrvBlueCoinStageProgress::NrvWait::sInstance);
    }

    if (!mPaneRumbler->isRumbling()) {
        mTargetID = -1;
        updateTextBox();
        updateAnimBox();
        setNerve(&NrvBlueCoinStageProgress::NrvWait::sInstance);
    }
}

void BlueCoinStageProgress::exeDisappear() {
    if (MR::isFirstStep(this))
        mAppearer->disappear();

    if (mAppearer->isDisappeared()) {
        MR::setTextBoxFormatRecursive(this, "ShaNum", L"");
        setNerve(&NrvBlueCoinStageProgress::NrvHide::sInstance);
    }
}

void BlueCoinStageProgress::disappear() {
    setNerve(&NrvBlueCoinStageProgress::NrvDisappear::sInstance);
}

void BlueCoinStageProgress::updateTextBox() {
    wchar_t strBuf[16];

    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    
    if (coinMin != -1) {
        s32 coinMax = BlueCoinUtil::getBlueCoinRange(0, true);
        s32 coinNum = (coinMax+1)-coinMin;

        int icon;
        for (s32 i = 0; i < coinNum; i++) {

            if (mTargetID == coinMin+i)
                icon = 0x52;
            else {
                if (BlueCoinUtil::isBlueCoinGotCurrentFile(coinMin+i))
                    icon = 0x8A;
                else
                    icon = 0xC0;
            }
            MR::addPictureFontCode(&strBuf[i], icon);
        }
        
        strBuf[coinNum] = L'\0';
        MR::setTextBoxFormatRecursive(this, "ShaText", strBuf);
    }
}

void BlueCoinStageProgress::updateAnimBox() {
    wchar_t anmBuf[16];

    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    
    if (coinMin != -1) {
        s32 coinMax = BlueCoinUtil::getBlueCoinRange(0, true);
        s32 coinNum = (coinMax+1)-coinMin;
        
        int icon;
        for (s32 i = 0; i < coinNum; i++) {
            icon = 0x52;

            if (coinMin+i == mTargetID)
                icon = 0x8A;

            MR::addPictureFontCode(&anmBuf[i], icon);
        }

        anmBuf[coinNum] = L'\0';
        MR::setTextBoxFormatRecursive(this, "ShaAnm", anmBuf);
    }
}

void BlueCoinStageProgress::updateNumBox() { 
    wchar_t numBuf[3];
    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    
    s32 targetNum = (mTargetID-coinMin)+1;
    
    if (mIsTargetIDGot)
        pt::str2wcsfullwidth(numBuf, targetNum);
    else
        swprintf(numBuf, 3, L"%d", targetNum);

    MR::setTextBoxFormatRecursive(this, "ShaNum", numBuf);
}

void BlueCoinStageProgress::setTargetIDAndAppear(s32 id) {
    mTargetID = id;
    mIsTargetIDGot = BlueCoinUtil::isBlueCoinGotCurrentFile(id);
    appear();
}

void BlueCoinStageProgress::calcSlidePaneFrame() {
    s32 totalInRange = BlueCoinUtil::calcBlueCoinTotalInRange(0, false);
    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    mSlideFrame = 200.0f + 32.0f*(mTargetID-coinMin) - 32.0f*(f32)((totalInRange/2)-1) - 18.0f*(!(totalInRange%2));
}

BlueCoinStageProgress::~BlueCoinStageProgress() {

}

namespace NrvBlueCoinStageProgress {
    void NrvAppear::execute(Spine* pSpine) const {
        ((BlueCoinStageProgress*)pSpine->mExecutor)->exeAppear();
    }

    void NrvWait::execute(Spine* pSpine) const {
        ((BlueCoinStageProgress*)pSpine->mExecutor)->exeWait();
    }

    void NrvUpdate::execute(Spine* pSpine) const {
        ((BlueCoinStageProgress*)pSpine->mExecutor)->exeUpdate();
    }

    void NrvDisappear::execute(Spine* pSpine) const {
        ((BlueCoinStageProgress*)pSpine->mExecutor)->exeDisappear();
    }

    void NrvHide::execute(Spine* pSpine) const {

    }

    NrvAppear(NrvAppear::sInstance);
    NrvWait(NrvWait::sInstance);
    NrvUpdate(NrvUpdate::sInstance);
    NrvDisappear(NrvDisappear::sInstance);
    NrvHide(NrvHide::sInstance);
}