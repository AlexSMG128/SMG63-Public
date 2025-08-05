#include "BlueCoinStageProgress.h"

BlueCoinStageProgress::BlueCoinStageProgress(const char* pName) : LayoutActor(pName, false) {
    mTargetID = -1;
    mAppearer = 0;
    mPaneRumbler = 0;
}

void BlueCoinStageProgress::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinStageProgress", 1);
    initNerve(&NrvBlueCoinStageProgress::NrvHide::sInstance);
    initEffectKeeper(0, 0, 0);
    MR::connectToSceneLayout(this);
    MR::registerDemoSimpleCastAll(this);
    MR::createAndAddPaneCtrl(this, "Text00", 1);
    MR::createAndAddPaneCtrl(this, "TextAnm", 1);
    MR::setTextBoxFormatRecursive(this, "ShaText", L"TEST");
    MR::setTextBoxFormatRecursive(this, "TextAnm", L"");
    MR::registerDemoSimpleCastAll(this);

    updateTextBox();

    mAppearer = new CounterLayoutAppearer(this, TVec2f(0.0f, 50.0f));

    mPaneRumbler = new CountUpPaneRumbler(this, "TextAnm");
    mPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;
}

void BlueCoinStageProgress::appear() {
    if (mAppearer->isAppeared())
        setNerve(&NrvBlueCoinStageProgress::NrvUpdate::sInstance);
    else
        setNerve(&NrvBlueCoinStageProgress::NrvAppear::sInstance);
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
        updateTextBox();
        updateAnimBox();
        mPaneRumbler->start();
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

    if (mAppearer->isDisappeared())
        setNerve(&NrvBlueCoinStageProgress::NrvHide::sInstance);
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
    
        for (s32 i = 0; i < coinNum; i++) {
            if (mTargetID == coinMin+i) {
                MR::addPictureFontCode(&strBuf[i], 0x52);
            }
            else {
                if (BlueCoinUtil::isBlueCoinGotCurrentFile(coinMin+i)) {
                    MR::addPictureFontCode(&strBuf[i], 0x8A);
                }
                else {
                    MR::addPictureFontCode(&strBuf[i], 0xC0);
                }
            }
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
        
        for (s32 i = 0; i < coinNum; i++) {
            if (coinMin+i == mTargetID)
                MR::addPictureFontCode(&anmBuf[i], 0x8A);
            else
                MR::addPictureFontCode(&anmBuf[i], 0x52);
        }

        anmBuf[coinNum] = L'\0';
        MR::setTextBoxFormatRecursive(this, "ShaAnm", anmBuf);
    }
}

void BlueCoinStageProgress::setTargetIDAndAppear(s32 id) {
    mTargetID = id;
    appear();
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