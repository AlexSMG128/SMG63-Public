#include "BlueCoinList.h"
#include "GeneralUtil.h"

BlueCoinList::BlueCoinList(const char* pName) : LayoutActor(pName, false) {
    for (s32 i = 0; i < 7; i++) {

        mListEntries[i] = new ListEntry;
        memset(mListEntries[i]->pStageName, 0, 48);
        mListEntries[i]->coinNum = 0;
        mListEntries[i]->rangeMin = 0;
        mListEntries[i]->rangeMax = 0;
        mListEntries[i]->isBlankSlot = false;
        mListEntries[i]->state = STATE_GALAXY_LOCKED;
    }

    mBackButton = 0;
    mRangeTable = 0;
    mArrowRight = 0;
    mArrowLeft = 0;
    mTotalCoinsInPage = 0;
    mTotalCollectedCoinsInPage = 0;
    mBlueCoinTotalCount = 0;
    mCurrentPage = 1;
    mPageDirection = 0;
    mCursorPosition = 0;
    mMaxPages = 1;
    mDefaultCursorPosition = 1;
    mDefaultPage = 1;
}

void BlueCoinList::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinList", 2);
    MR::connectToSceneLayoutOnPause(this);
    
    mBackButton = new BackButtonCancelB("BackButtonBlueCoinList", 0);
    MR::connectToSceneLayoutOnPause(mBackButton);
    mBackButton->initWithoutIter();

    MR::createAndAddPaneCtrl(this, "Cursor", 2);
    MR::createAndAddPaneCtrl(this, "WinGalaxy", 1);
    MR::createAndAddPaneCtrl(this, "Arrow", 1);
    MR::createAndAddPaneCtrl(this, "Galaxy", 1);

    initNerve(&NrvBlueCoinList::NrvInit::sInstance);
    
    mRangeTable = BlueCoinUtil::getBlueCoinIDRangeTable();

    mArrowRight = new ButtonPaneController(this, "Right", "BoxArrowR", 0, 1);
    mArrowRight->mFadeAfterSelect = false;

    mArrowLeft = new ButtonPaneController(this, "Left", "BoxArrowL", 0, 1);
    mArrowLeft->mFadeAfterSelect = false;


    for (s32 i = 0; i < MR::getCsvDataElementNum(mRangeTable); i++) {
        s32 pageNum;
        s32 slotNum;
        MR::getCsvDataS32(&pageNum, mRangeTable, "ListPage", i);
        MR::getCsvDataS32(&slotNum, mRangeTable, "ListSlot", i);

        if (pageNum > 0 && slotNum > 0) {
            const char* pStageName;
            MR::getCsvDataStrOrNULL(&pStageName, mRangeTable, "StageName", i);
            mBlueCoinTotalCount = mBlueCoinTotalCount + BlueCoinUtil::calcBlueCoinTotalInRange(pStageName, false);

            if (MR::isEqualStageName(pStageName)) {
                mDefaultPage = pageNum;
                mDefaultCursorPosition = slotNum - 1;
            }
        }
    }
}

void BlueCoinList::appear() {
    setNerve(&NrvBlueCoinList::NrvAppear::sInstance);
    LayoutActor::appear();
}

void BlueCoinList::control() {
    mArrowRight->update();
    mArrowLeft->update();

    if (mArrowRight->isWait() && mArrowRight->getNerveStep() == 1 && mArrowLeft->isWait()) {
        f32 frameLeft = MR::getPaneAnimFrame(this, "Left", 0);
        MR::setPaneAnimFrame(this, "Right", frameLeft, 0);
    }

    if (mArrowLeft->isWait() && mArrowLeft->getNerveStep() == 1 && mArrowRight->isWait()) {
        f32 frameRight = MR::getPaneAnimFrame(this, "Right", 0);
        MR::setPaneAnimFrame(this, "Left", frameRight, 0);
    }
}

void BlueCoinList::exeAppear() {
    if (MR::isFirstStep(this)) {
        mBackButton->appear();

        mArrowRight->appear();
        mArrowLeft->appear();

        mCursorPosition = mDefaultCursorPosition;
        mCurrentPage = mDefaultPage;

        populateListEntries();
        updateTextBoxes();
        updateBlueCoinTextPane();
        setCursorPosition(mCursorPosition);

        MR::setTextBoxGameMessageRecursive(this, "ShaTotals", "BlueCoinList_Totals");

        
        if (mCurrentPage == 1)
            MR::hidePaneRecursive(this, "Left");
        else 
            MR::showPaneRecursive(this, "Left");

        if (mCurrentPage == mMaxPages) 
            MR::hidePaneRecursive(this, "Right");
        else 
            MR::showPaneRecursive(this, "Right");

        wchar_t completeTotalStr[2];
        completeTotalStr[1] = 0;
    
        completeTotalStr[0] = 0x52;
    
        if (BlueCoinUtil::getTotalBlueCoinNumCurrentFile(false) == mBlueCoinTotalCount) {
            completeTotalStr[0] = 0x50;
        }

        MR::setTextBoxFormatRecursive(this, "ShaCompleteT", completeTotalStr);

        MR::setTextBoxGameMessageRecursive(this, "ShaNoData", "BlueCoinList_NoData");

        MR::setTextBoxGameMessageRecursive(this, "ShaProgress", "BlueCoinList_Progress");

        MR::setTextBoxGameMessageRecursive(this, "InfoPage", "BlueCoinList_Page");
        MR::setTextBoxArgNumberRecursive(this, "InfoPage", mCurrentPage, 0);
        MR::setTextBoxArgNumberRecursive(this, "InfoPage", mMaxPages, 1);

        MR::startPaneAnimAndSetFrameAndStop(this, "WinGalaxy", "Visibility", 0.0f, 0);

        MR::setTextBoxGameMessageRecursive(this, "ShaBlueCoinT", "BlueCoinList_Counter");
        MR::setTextBoxArgNumberRecursive(this, "ShaBlueCoinT", BlueCoinUtil::getTotalBlueCoinNumCurrentFile(false), 0);

        wchar_t bufProgressT[6];
        pt::str2wcsfullwidth(bufProgressT, mBlueCoinTotalCount);

        MR::setTextBoxFormatRecursive(this, "ShaCoinMaxT", bufProgressT);

        MR::setTextBoxGameMessageRecursive(this, "ShaBlueCoinP", "BlueCoinList_Counter");
        MR::setTextBoxArgNumberRecursive(this, "ShaBlueCoinP", mTotalCollectedCoinsInPage, 0);

        MR::startAnim(this, "Appear", 0);
    }

    if (MR::isStep(this, 20))
        setNerve(&NrvBlueCoinList::NrvWait::sInstance);
}

void BlueCoinList::exeChange() {
    if (MR::isFirstStep(this)) {
        MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_SLIDE", -1, -1);

        const char* animName = "NextPage";
            if (mPageDirection == -1) 
                animName = "PreviousPage";

        MR::startAnim(this, animName, 0);
    }

    if (MR::isStep(this, 11)) {
        if (mCurrentPage == 1)
            MR::hidePaneRecursive(this, "Left");
        else 
            MR::showPaneRecursive(this, "Left");

        if (mCurrentPage == mMaxPages) 
            MR::hidePaneRecursive(this, "Right");
        else 
            MR::showPaneRecursive(this, "Right");

        populateListEntries();
        updateTextBoxes();

        while (mCursorPosition != 7 && mCursorPosition != -1 && isEntryBlank(getEntry(mCursorPosition))) {
            mCursorPosition = (mCursorPosition-1) % 8;
        }

        setCursorPosition(mCursorPosition);
        updateBlueCoinTextPane();
    }

    if (MR::isStep(this, 26)) {
        setNerve(&NrvBlueCoinList::NrvWait::sInstance);
    }
}

void BlueCoinList::exeChangeFail() {
    if (MR::isFirstStep(this)) {
        mCurrentPage = mCurrentPage - mPageDirection;

        MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_NG", -1, -1);
        const char* animName = "NextPageOver";
            if (mPageDirection == -1) 
                animName = "PreviousPageOver";

        MR::startAnim(this, animName, 0);
    }

    MR::setNerveAtAnimStopped(this, &NrvBlueCoinList::NrvWait::sInstance, 0);
}

void BlueCoinList::exeClose() {
    if (MR::isFirstStep(this))
        MR::startAnim(this, "End", 0);
    
    if (MR::isStep(this, 20))
        kill();
}


void BlueCoinList::exeWait() {
    if (MR::isFirstStep(this)) {
        if (!mArrowRight->isWait())
            mArrowRight->forceToWait();
        if (!mArrowLeft->isWait())
            mArrowLeft->forceToWait();
    }
    
    if (__kAutoMap_8045E9D0(mBackButton)) {
        if (mBackButton->isHidden()) {
            setNerve(&NrvBlueCoinList::NrvClose::sInstance);
        }
    }
    else {

    mPageDirection = 0;
    s32 cursorDirection = 0;

    bool isLeftValid = !MR::isHiddenPane(this, "Left");
    bool isRightValid = !MR::isHiddenPane(this, "Right");

    if (isLeftValid && mArrowLeft->isPointingTrigger() || isRightValid && mArrowRight->isPointingTrigger())
        MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM", -1, -1);

    if (MR::testCorePadTriggerLeft(0) || MR::testSubPadStickTriggerLeft(0) || isLeftValid && mArrowLeft->trySelect()) {
        mPageDirection = -1;

        if (!mArrowLeft->isDecidedWait())
            __kAutoMap_80461860(mArrowLeft);
    }

    if (MR::testCorePadTriggerRight(0) || MR::testSubPadStickTriggerRight(0) || isRightValid && mArrowRight->trySelect()) {
        mPageDirection = 1;

        if (!mArrowRight->isDecidedWait())
            __kAutoMap_80461860(mArrowRight);
    }

    if (mPageDirection != 0) {
        mCurrentPage = mCurrentPage + mPageDirection;
        
        if (mCurrentPage < 1 || mCurrentPage == (mMaxPages+1))
            setNerve(&NrvBlueCoinList::NrvChangeFail::sInstance);
        else
            setNerve(&NrvBlueCoinList::NrvChange::sInstance);

    }
    else {
        if (MR::testCorePadTriggerUp(0))
            cursorDirection = -1;
        if (MR::testCorePadTriggerDown(0))
            cursorDirection = 1;
    }


    if (cursorDirection != 0) {
        mCursorPosition = (mCursorPosition +cursorDirection) % 8;
        
        while (mCursorPosition != 7 && mCursorPosition != -1 && isEntryBlank(getEntry(mCursorPosition))) {
            mCursorPosition = (mCursorPosition +cursorDirection) % 8;
        }

        if (mCursorPosition == -1)
            mCursorPosition = 7;

        setCursorPosition(mCursorPosition);
        updateBlueCoinTextPane();
    }
}
}


void BlueCoinList::setCursorPosition(s32 slot) {
    char paneName[13];
    if (slot < 7)
        snprintf(paneName, 13, "TxtGalaxy%d", slot);
    else
        MR::copyString(paneName, "TxtTotals", 11);

    TBox2f txtPaneBox;
    MR::calcTextBoxRectRecursive(&txtPaneBox, this, paneName);
    f32 strLength = txtPaneBox.mPointMax.x-txtPaneBox.mPointMin.x;

    MR::startPaneAnimAndSetFrameAndStop(this, "Cursor", "CursorScale", strLength+32.0f, 0);
    MR::startPaneAnimAndSetFrameAndStop(this, "Cursor", "CursorPosition", (f32)slot, 1);
}

void BlueCoinList::populateListEntries() {
    mTotalCollectedCoinsInPage = 0;
    mTotalCoinsInPage = 0;
    for (s32 i = 0; i < 7; i++) {
        ListEntry* entry = getEntry(i);
        setEntryBlank(entry);
        entry->state = STATE_GALAXY_LOCKED;
    }

    for (s32 i = 0; i < MR::getCsvDataElementNum(mRangeTable); i++) {
        s32 pageNum = 0;
        MR::getCsvDataS32(&pageNum, mRangeTable, "ListPage", i);

        if (mMaxPages < pageNum)
            mMaxPages = pageNum;

        if (pageNum == mCurrentPage) {
            s32 slotNum = 0;
            MR::getCsvDataS32(&slotNum, mRangeTable, "ListSlot", i);
            
            if (slotNum < 8) {
                const char* pName = 0;
                MR::getCsvDataStrOrNULL(&pName, mRangeTable, "StageName", i);

                if (pName) {
                    ListEntry* entry = getEntry(slotNum-1);
                    MR::copyString(entry->pStageName, pName, 48);
                    entry->rangeMax = BlueCoinUtil::getBlueCoinRange(pName, true);
                    entry->rangeMin = BlueCoinUtil::getBlueCoinRange(pName, false);
                    entry->coinNum = BlueCoinUtil::calcBlueCoinTotalInRange(pName, true);
                    entry->isBlankSlot = false;
                    mTotalCoinsInPage += BlueCoinUtil::calcBlueCoinTotalInRange(pName, false);
                    mTotalCollectedCoinsInPage += BlueCoinUtil::calcBlueCoinTotalInRange(pName, true);
                    
                    if (MR::isEqualStageName(pName) || MR::getPowerStarNumOwnedInStage(pName) || entry->coinNum > 0) {
                        entry->state = STATE_GALAXY_OPENED;

                        if (entry->coinNum == (entry->rangeMax-entry->rangeMin)+1)
                            entry->state = STATE_GALAXY_ALLCOINS;
                    }
                    setEntryNotBlank(entry);
                }
            }
        }
    }

    //printListDebugInfo();
}

void BlueCoinList::updateTextBoxes() {
    for (s32 i = 0; i < 7; i++) {
        ListEntry* entry = getEntry(i);
        char txtPaneName[16];
        char coinMiPaneName[17];
        char coinMaPaneName[17];
        char coinGalaxy[15];
        char coinComplete[16];

        s32 coinMiArg = 0;
        s32 coinMaArg = 0;

        snprintf(txtPaneName, 16, "StageGalaxy%d", i);
        snprintf(coinMiPaneName, 17, "CoinMiGalaxy%d", i);
        snprintf(coinMaPaneName, 17, "CoinMaGalaxy%d", i);
        snprintf(coinGalaxy, 15, "CoinGalaxy%d", i);
        snprintf(coinComplete, 16, "ShaComplete%d", i);

        if (isEntryBlank(entry)) {
            MR::setTextBoxFormatRecursive(this, txtPaneName, L"");
            MR::hidePaneRecursive(this, coinGalaxy);
        }
        else {
            if (entry->state != STATE_GALAXY_LOCKED) {
                MR::setTextBoxFormatRecursive(this, txtPaneName, MR::getGalaxyNameShortOnCurrentLanguage(entry->pStageName));
                MR::showPaneRecursive(this, coinGalaxy);
                coinMiArg = entry->coinNum;
                coinMaArg = (entry->rangeMax-entry->rangeMin)+1;
            }
            else {
                MR::setTextBoxFormatRecursive(this, txtPaneName, L"----");
                MR::hidePaneRecursive(this, coinGalaxy);
            }
        }   
        
        MR::setTextBoxGameMessageRecursive(this, coinMiPaneName, "BlueCoinList_Counter");
        MR::setTextBoxArgNumberRecursive(this, coinMiPaneName, coinMiArg, 0);
        
        wchar_t bufProgress[6];
        pt::str2wcsfullwidth(bufProgress, coinMaArg);
        MR::setTextBoxFormatRecursive(this, coinMaPaneName, bufProgress);

        wchar_t completeStr[2];
        completeStr[1] = 0;

        completeStr[0] = 0x52;

        if (entry->state == STATE_GALAXY_ALLCOINS) {
            completeStr[0] = 0x50;
        }

        MR::setTextBoxFormatRecursive(this, coinComplete, completeStr);
    }
    
    char listPageName[22];
    snprintf(listPageName, 22, "BlueCoinList_Page%d", mCurrentPage);
    MR::setTextBoxGameMessageRecursive(this, "TextPage", listPageName);
    
    MR::setTextBoxArgNumberRecursive(this, "InfoPage", mCurrentPage, 0);

    MR::setTextBoxArgNumberRecursive(this, "ShaBlueCoinP", mTotalCollectedCoinsInPage, 0);

    wchar_t bufProgressP[6];
    pt::str2wcsfullwidth(bufProgressP, mTotalCoinsInPage);

    MR::setTextBoxFormatRecursive(this, "ShaCoinMaxP", bufProgressP);
}

void BlueCoinList::updateBlueCoinTextPane() {    
    if (mCursorPosition != 7) {
        wchar_t IDListStr[32];
        ListEntry* entry = getEntry(mCursorPosition);
        s32 newLineOff = 0;

        if (!isEntryBlank(entry)) {
            s32 coinId = entry->rangeMin;
            s32 totalCoins = (entry->rangeMax-entry->rangeMin)+1;
            
            if (totalCoins > 30)
                totalCoins = 30;
                
            for (s32 i = 0; i < totalCoins + (s32)(totalCoins > 15); i++) {

                if (i == totalCoins/2 && totalCoins > 15) {
                    MR::addNewLine(&IDListStr[i]);
                    newLineOff++;
                }
                else {
                    bool isGot = BlueCoinUtil::isBlueCoinGotCurrentFile(coinId);
                    int icon = 0xC0;

                    if (isGot)
                        icon = 0x8A;

                    MR::addPictureFontCode(&IDListStr[i], icon);
                    coinId++;
                }
            }
        }

        MR::setTextBoxFormatRecursive(this, "CoinListWin", IDListStr);
    }

    f32 frame = 0.0f;

    if (mCursorPosition == 7)
        frame = 1.0f;
    else if (getEntry(mCursorPosition)->state == STATE_GALAXY_LOCKED)
        frame = 2.0f;

    MR::startPaneAnimAndSetFrameAndStop(this, "WinGalaxy", "Visibility", frame, 0);
}

BlueCoinList::ListEntry* BlueCoinList::getEntry(s32 slot) {
    if (slot < 0 || slot > 6)
        return 0;

    return mListEntries[slot];
};

bool BlueCoinList::isEntryBlank(ListEntry* pEntry) {
    return pEntry->isBlankSlot;
}

void BlueCoinList::setEntryBlank(ListEntry* pEntry) {
    pEntry->isBlankSlot = true;
}

void BlueCoinList::setEntryNotBlank(ListEntry* pEntry) {
    pEntry->isBlankSlot = false;
}

void BlueCoinList::printListDebugInfo() {
    OSReport("----ENTRIES----\n");
    for (s32 i = 0; i < 7; i++) {
        ListEntry* entry = getEntry(i);

        OSReport("Entry %d, %s, [%d - %d], %d, Galaxy State: %d,Blank? %s\n",
        i,
        entry->pStageName,
        entry->rangeMin, entry->rangeMax,
        entry->coinNum, 
        entry->state,
        isEntryBlank(entry) ? "Yes" : "No");
    }

    OSReport("------------\n");
}


BlueCoinList::~BlueCoinList() {

}

namespace NrvBlueCoinList {
    void NrvInit::execute(Spine* pSpine) const {}

    void NrvAppear::execute(Spine* pSpine) const {
        ((BlueCoinList*)pSpine->mExecutor)->exeAppear();
    }

    void NrvChange::execute(Spine* pSpine) const {
        ((BlueCoinList*)pSpine->mExecutor)->exeChange();
    }

    void NrvChangeFail::execute(Spine* pSpine) const {
        ((BlueCoinList*)pSpine->mExecutor)->exeChangeFail();
    }

    void NrvWait::execute(Spine* pSpine) const {
        ((BlueCoinList*)pSpine->mExecutor)->exeWait();
    }

    void NrvClose::execute(Spine* pSpine) const {
        ((BlueCoinList*)pSpine->mExecutor)->exeClose();
    }


    NrvInit(NrvInit::sInstance);
    NrvAppear(NrvAppear::sInstance);
    NrvChange(NrvChange::sInstance);
    NrvChangeFail(NrvChangeFail::sInstance);
    NrvWait(NrvWait::sInstance);
    NrvClose(NrvClose::sInstance);
}