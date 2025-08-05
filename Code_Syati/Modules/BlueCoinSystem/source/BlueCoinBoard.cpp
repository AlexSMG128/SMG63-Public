#include "BlueCoinBoard.h"  
#include "BlueCoin.h"  
#include "BlueCoinUtil.h"
#ifdef BLUECOINBOARD
/*
    Super Mario Starshine: Blue Coin Board

    This layout handles the purchasing and entering of the Blue Coin bonus stages.

    Blue Coin Flags were only ever implemented since I planned very far ahead.
    This is the only reason why they ever existed.

    Made for Starshine only. Do not ask me for this in your mods.

    First Planned in 5/2023
    Started 9/24/2023
    Finished 10/1/2023
    Revealed 10/2/2023

    Uses Blue Coin Flags specified in the BlueCoinBoardDataTable BCSV file.
    BlueCoinSign Obj_arg0 is the Completion flag.

    Made by Evanbowl
    
    I thank SPG64, Lord Giganticus, and Xandog for very helpful feedback.
*/

BlueCoinSign::BlueCoinSign(const char* pName) : NPCActor(pName) {
    pBoard = 0;
}

void BlueCoinSign::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps = NPCActorCaps("BlueCoinSign");
    caps.setDefault();
    caps.mInitLightCtrl = false;
    caps.mInitYoshiLockOnTarget = false;
    caps.mWaitNerve = &NrvBlueCoinSign::NrvWait::sInstance;

    initialize(rIter, caps, 0, 0, false);

    mTalkCtrl->mShowFrame = false;

    MR::registerEventFunc(mTalkCtrl, TalkMessageFunc(this, &eventFunc));

    pBoard = new BlueCoinBoard("BlueCoinBoard");
    pBoard->initWithoutIter();
    s32 flag = 8;
    MR::getJMapInfoArg0NoInit(rIter, &flag);
    pBoard->mCompleteFlag = flag;
    makeActorAppeared();
}

bool BlueCoinSign::eventFunc(u32 eventParam) {
    if (isNerve(&NrvBlueCoinSign::NrvOpen::sInstance)) {
        if (MR::isDead(pBoard)) {
            popNerve();
            return true;
        }
        else
            return false;
    }
    else {  
        pushNerve(&NrvBlueCoinSign::NrvOpen::sInstance);
        return false;
    }

    return false;
}

void BlueCoinSign::exeWait() {
    MR::tryTalkNearPlayer(mTalkCtrl);
}

void BlueCoinSign::exeOpen() {
    if (MR::isFirstStep(this))
        pBoard->appear();
}

namespace NrvBlueCoinSign {
    void NrvWait::execute(Spine* pSpine) const {
        ((BlueCoinSign*)pSpine->mExecutor)->exeWait();
    }

    void NrvOpen::execute(Spine* pSpine) const {
        ((BlueCoinSign*)pSpine->mExecutor)->exeOpen();
    }

    NrvWait(NrvWait::sInstance);
    NrvOpen(NrvOpen::sInstance);
}


BlueCoinBoard::BlueCoinBoard(const char* pName) : LayoutActor(pName, 0) {
    for (s32 i = 0; i < 8; i++) {
        mButtons[i] = 0;
        mButtonFollowPositions[i] = TVec2f(120.0f, 0.0f);
        sprintf(mBoxButtonName[i], "BoxButton%d", i);
        sprintf(mButtonName[i], "CoinButton%d", i);
        sprintf(mButtonTxtName[i], "Button%dText", i);
        sprintf(mButtonFollow[i], "Button%d", i);
        sprintf(mCopyPosName[i], "Button%dPos", i);
    }
    mSysInfoWindowSelect = 0;
    mSysInfoWindowBox = 0;
    mBlueCoinPaneRumbler = 0;
    mBackButton = 0;
    mTable = MR::tryCreateCsvParser("/SystemData/BlueCoinBoardDataTable.arc", "BlueCoinBoardDataTable.bcsv");
    mBlueCoinCounterFollowPos = TVec2f(0.0f, 0.0f);
    mSelectedButton = -1;
    mBlueCoinNumToDisplay = 0;
    mTotalBlueCoinPrices = 0;
    mCompleteFlag = 0;
    mHasSpentBlueCoins = false;
}

void BlueCoinBoard::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinBoard", 2);
    initEffectKeeper(0, NULL, NULL);
    MR::registerDemoSimpleCastAll(this);
    initNerve(&NrvBlueCoinBoard::NrvHide::sInstance);
    MR::connectToScene(this, 0xE, 0xD, -1, 0x48);

    mBackButton = new BackButtonCancelB("BackButtonForBlueCoin", 0);
    MR::connectToScene(mBackButton, 0xE, 0xD, -1, 0x48);
    mBackButton->initWithoutIter();

    mSysInfoWindowSelect = MR::createSysInfoWindow();
    MR::connectToScene(mSysInfoWindowSelect, 0xE, 0xD, -1, 0x48);

    mSysInfoWindowBox = MR::createSysInfoWindowMiniExecuteWithChildren();
    MR::connectToScene(mSysInfoWindowBox, 0xE, 0xD, -1, 0x48);

    mBlueCoinPaneRumbler = new CountUpPaneRumbler(this, "CounterBlueCoin");
    mBlueCoinPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;
    mBlueCoinPaneRumbler->reset();

    checkBoardProgress();

    MR::createAndAddPaneCtrl(this, "Misc", 1);
    MR::createAndAddPaneCtrl(this, "StarCounter", 1);
    MR::createAndAddPaneCtrl(this, "BlueCoinCounter", 1);

    MR::setFollowPos(&mBlueCoinCounterFollowPos, this, "BlueCoinCounter");

    for (s32 i = 0; i < 8; i++) {
        MR::createAndAddPaneCtrl(this, mButtonFollow[i], 1);

        mButtons[i] = new ButtonPaneController(this, mButtonName[i], mBoxButtonName[i], 0, 1);
        mButtons[i]->mFadeAfterSelect = false;

        MR::setFollowPos(&mButtonFollowPositions[i], this, mButtonFollow[i]);
        MR::setFollowTypeReplace(this, mButtonFollow[i]);

        s32 priceFromTable = 0;
        MR::getCsvDataS32(&priceFromTable, mTable, "BlueCoinPrice", i);
        mTotalBlueCoinPrices += priceFromTable;
    }
}

void BlueCoinBoard::appear() {
    setNerve(&NrvBlueCoinBoard::NrvAppear::sInstance);
    LayoutActor::appear();
}

void BlueCoinBoard::exeAppear() {
    if (MR::isFirstStep(this)) {
        s32 numStars = 0;
        s32 scenarioNoFromTable = 0;
        s32 flag = 0;
        const char* nameFromTable;
        mHasSpentBlueCoins = false;
        
        for (s32 i = 0; i < 8; i++) {
            MR::getCsvDataStr(&nameFromTable, mTable, "StageName", i);
            MR::getCsvDataS32(&scenarioNoFromTable, mTable, "ScenarioNo", i);
            MR::getCsvDataS32(&flag, mTable, "BlueCoinFlag", i);

            if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(flag)) {
                const char* label = "BoardButton_UnlockedUncleared";

                if (MR::makeGalaxyStatusAccessor(nameFromTable).hasPowerStar(scenarioNoFromTable)) {
                    label = "BoardButton_UnlockedCleared";
                    numStars++;
                }

                MR::setTextBoxGameMessageRecursive(this, mButtonTxtName[i], label);
                MR::setTextBoxArgStringRecursive(this, mButtonTxtName[i], MR::getGalaxyNameOnCurrentLanguage(nameFromTable), 0);
            }
            else {
                s32 priceFromTable = 0;
                MR::getCsvDataS32(&priceFromTable, mTable, "BlueCoinPrice", i);

                MR::setTextBoxGameMessageRecursive(this, mButtonTxtName[i], "BoardButton_Locked");
                MR::setTextBoxArgNumberRecursive(this, mButtonTxtName[i], i+1, 0);
                pt::setTextBoxArgStringNumberFontRecursive(this, mButtonTxtName[i], priceFromTable, 1);
            }

            mButtons[i]->appear();
        }

        MR::requestMovementOn(mSysInfoWindowSelect);
        MR::requestMovementOn(mSysInfoWindowBox);

        if (MR::isExistSceneObj(SCENE_OBJ_PAUSE_BLUR)) {
            PauseBlur* pPauseBlur = (PauseBlur*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_PAUSE_BLUR);
            MR::requestMovementOn(pPauseBlur);
            pPauseBlur->_30+=1;
        }

        MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_OPEN", -1, -1);

        MR::startAnim(this, "Appear", 1);
        MR::startStarPointerModeChooseYesNo(this);

        mBlueCoinNumToDisplay = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true);

        MR::setTextBoxNumberRecursive(this, "CounterBlueCoin", mBlueCoinNumToDisplay);
        MR::setTextBoxNumberRecursive(this, "CounterStar", numStars);

        MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_NoSelection");
        MR::setTextBoxGameMessageRecursive(this, "TextTitle", "Board_Title");

        if (BlueCoinUtil::getSpentBlueCoinNumCurrentFile() >= mTotalBlueCoinPrices) {
            MR::showPaneRecursive(this, "TextComplete");
            MR::hidePaneRecursive(this, "BlueCoinCounter");

            if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(BLUE_COIN_BOARD_COMPLETE))
                MR::setTextBoxGameMessageRecursive(this, "TextComplete", "WinBase_Complete");
            else
                MR::setTextBoxGameMessageRecursive(this, "TextComplete", "WinBase_AllSpent");
        }

        MR::copyPaneTrans(&mBlueCoinCounterFollowPos, this, BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true) > 99 ? "BlueCoinPos100" : "BlueCoinPos10");

        mBackButton->appear();

        setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

void BlueCoinBoard::control() {
    for (s32 i = 0; i < 8; i++) {
        MR::copyPaneTrans(&mButtonFollowPositions[i], this, mCopyPosName[i]);
        mButtons[i]->update();
    }

    mBlueCoinPaneRumbler->update();
}

void BlueCoinBoard::exeSelecting() {
    if (MR::isFirstStep(this))
        MR::requestMovementOn(mBackButton);

    mSelectedButton = -1;
    s32 pointedButton = -1;

    for (s32 i = 0; i < 8; i++) {
        if (mButtons[i]->trySelect())
            mSelectedButton = i;

        if (mButtons[i]->isPointing())
            pointedButton = i;

        if (mButtons[i]->isPointingTrigger())
            MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM", -1, -1);
    }

    if (mSelectedButton > -1) {
        pointedButton = mSelectedButton;
        setNerve(&NrvBlueCoinBoard::NrvSelected::sInstance);
    }

    if (mBackButton->_30)
        setNerve(&NrvBlueCoinBoard::NrvDisappear::sInstance);

    connectButtonsToDPad();

    if (!MR::isFirstStep(this)) {
        if (pointedButton > -1) {
            const char* nameFromTable;
            s32 scenarioNoFromTable = 0;
            s32 flag = 0;

            MR::getCsvDataStr(&nameFromTable, mTable, "StageName", pointedButton);
            MR::getCsvDataS32(&scenarioNoFromTable, mTable, "ScenarioNo", pointedButton);
            MR::getCsvDataS32(&flag, mTable, "BlueCoinFlag", pointedButton);

            if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(flag)) {
                const char* label = "WinBase_UnlockedUncleared";

                if (MR::makeGalaxyStatusAccessor(nameFromTable).hasPowerStar(scenarioNoFromTable))
                    label = "WinBase_UnlockedCleared";

                MR::setTextBoxGameMessageRecursive(this, "TextWinBase", label);
                MR::setTextBoxArgStringRecursive(this, "TextWinBase", MR::getGalaxyNameOnCurrentLanguage(nameFromTable), 0);
            }
            else {
                s32 priceFromTable = 0;
                MR::getCsvDataS32(&priceFromTable, mTable, "BlueCoinPrice", pointedButton);
                
                MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_Locked");
                MR::setTextBoxArgNumberRecursive(this, "TextWinBase", pointedButton+1, 0);
                pt::setTextBoxArgStringNumberFontRecursive(this, "TextWinBase", priceFromTable, 1);
            }
        }
        else
            MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_NoSelection");
    }
}

void BlueCoinBoard::exeDisappear() { 
    if (MR::isStep(this, 30)) {
        for (s32 i = 0; i < 8; i++) {
            mButtons[i]->disappear();
        }

        if (MR::isExistSceneObj(SCENE_OBJ_PAUSE_BLUR))
            ((PauseBlur*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_PAUSE_BLUR))->_30-=1;
        
        MR::startAnim(this, "End", 0);
    }

    if (MR::isStep(this, 50)) {
        BlueCoinUtil::setCounter();
        MR::endStarPointerMode(this);
        kill();
    }
}

void BlueCoinBoard::exeSelected() {
    if (MR::isFirstStep(this)) {
        MR::requestMovementOff(mBackButton);
        MR::startSystemSE("SE_SY_PAUSE_OFF", -1, -1);
    }

    if (MR::isStep(this, 25)) {
        mButtons[mSelectedButton]->forceToWait();

        s32 flag;
        MR::getCsvDataS32(&flag, mTable, "BlueCoinFlag", mSelectedButton);

        if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(flag))
            setNerve(&NrvBlueCoinBoard::NrvConfirmPlayStage::sInstance);
        else
            setNerve(&NrvBlueCoinBoard::NrvConfirmUnlock::sInstance);
    }
}

void BlueCoinBoard::exeConfirmUnlock() {
    s32 priceFromTable;
    MR::getCsvDataS32(&priceFromTable, mTable, "BlueCoinPrice", mSelectedButton);

    if (MR::isFirstStep(this)) {
        MR::requestMovementOn(mSysInfoWindowSelect);
        mSysInfoWindowSelect->appear("BoardInfoWindow_ConfirmUnlockStage", SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);
        MR::setTextBoxArgNumberRecursive(mSysInfoWindowSelect, mSysInfoWindowSelect->_3C, mSelectedButton+1, 0);
        MR::setTextBoxArgNumberRecursive(mSysInfoWindowSelect, mSysInfoWindowSelect->_3C, priceFromTable, 1);
    }

    if (MR::isDead(mSysInfoWindowSelect)) {
        if (mSysInfoWindowSelect->isSelectedYes() && !BlueCoinUtil::isOnBlueCoinFlagCurrentFile(BLUE_COIN_BOARD_COMPLETE)) {
            
            if (BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true) >= priceFromTable)
                setNerve(&NrvBlueCoinBoard::NrvCountDownBlueCoin::sInstance);
            else
                setNerve(&NrvBlueCoinBoard::NrvNotEnoughBlueCoins::sInstance);
        }
        else
            setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

void BlueCoinBoard::exeCountDownBlueCoin() {
    s32 priceFromTable = 0;
    MR::getCsvDataS32(&priceFromTable, mTable, "BlueCoinPrice", mSelectedButton);

    if (MR::testCorePadTriggerA(0)) {
        mBlueCoinNumToDisplay = (BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true)-priceFromTable);
    }

    if (mBlueCoinNumToDisplay > (BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true)-priceFromTable)) {
        if (getNerveStep() % 2 == 0) {
            #ifdef SMSS
                MR::startSystemSE("SE_BV_BLUE_COIN_GET", -1, -1);
            #else
                MR::startSystemSE("SE_SY_PURPLE_COIN", -1, -1);
            #endif
        }

        mBlueCoinNumToDisplay--;
    }
    else {
        MR::startPaneAnim(this, "CounterBlueCoin", "Flash", 0);
        //MR::emitEffect(this, "BlueCoinBoardCounterLight");
        mBlueCoinPaneRumbler->start();
        mHasSpentBlueCoins = true;
        BlueCoinUtil::spendBlueCoinCurrentFile(priceFromTable);

        s32 flag;
        MR::getCsvDataS32(&flag, mTable, "BlueCoinFlag", mSelectedButton);
        BlueCoinUtil::setOnBlueCoinFlagCurrentFile(flag);
        setNerve(&NrvBlueCoinBoard::NrvChangeButtonText::sInstance);
    }

    if (mBlueCoinNumToDisplay < 100)
        MR::copyPaneTrans(&mBlueCoinCounterFollowPos, this, "BlueCoinPos10");

    MR::setTextBoxNumberRecursive(this, "CounterBlueCoin", mBlueCoinNumToDisplay);  
}

void BlueCoinBoard::exeNotEnoughBlueCoins() {
    if (MR::isFirstStep(this))
        mSysInfoWindowBox->appear("BoardInfoWindow_NotEnoughBlueCoins", SysInfoWindow::SysInfoType_0, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);

    if (MR::isDead(mSysInfoWindowBox))
        setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
}

void BlueCoinBoard::exeChangeButtonText() {
    const char* nameFromTable;
    MR::getCsvDataStr(&nameFromTable, mTable, "StageName", mSelectedButton);

    if (MR::isStep(this, 20)) {
        MR::setTextBoxGameMessageRecursive(this, mButtonTxtName[mSelectedButton], "BoardButton_UnlockedUncleared");
        MR::setTextBoxArgStringRecursive(this, mButtonTxtName[mSelectedButton], MR::getGalaxyNameOnCurrentLanguage(nameFromTable), 0);

        MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_UnlockedUncleared");
        MR::setTextBoxArgStringRecursive(this, "TextWinBase", MR::getGalaxyNameOnCurrentLanguage(nameFromTable), 0);

        if (BlueCoinUtil::getSpentBlueCoinNumCurrentFile() == mTotalBlueCoinPrices) {
            MR::showPaneRecursive(this, "TextComplete");
            MR::hidePaneRecursive(this, "BlueCoinCounter");
            MR::setTextBoxGameMessageRecursive(this, "TextComplete", "WinBase_AllSpent");
        }
    }

    if (MR::isStep(this, 30))
        setNerve(&NrvBlueCoinBoard::NrvConfirmPlayStage::sInstance);
}

void BlueCoinBoard::exeConfirmPlayStage() {
    const char* nameFromTable;
    s32 scenarioNoFromTable;
    MR::getCsvDataStr(&nameFromTable, mTable, "StageName", mSelectedButton);
    MR::getCsvDataS32(&scenarioNoFromTable, mTable, "ScenarioNo", mSelectedButton);

    if (MR::isFirstStep(this)) {
        mSysInfoWindowSelect->appear("BoardInfoWindow_ConfirmPlayStage", SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);
        MR::setTextBoxArgStringRecursive(mSysInfoWindowSelect, mSysInfoWindowSelect->_3C, MR::getGalaxyNameOnCurrentLanguage(nameFromTable), 0);
    }

    if (MR::isDead(mSysInfoWindowSelect)) {
        if (mSysInfoWindowSelect->isSelectedYes()) {
            MR::startSystemWipeCircleWithCaptureScreen(90);
            MR::stopStageBGM(60);
            GameSequenceFunction::requestChangeScenarioSelect(nameFromTable);
            GameSequenceFunction::requestChangeStage(nameFromTable, scenarioNoFromTable, scenarioNoFromTable, JMapIdInfo(0, 0));
        }
        else
            setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

void BlueCoinBoard::checkBoardProgress() {
    if (!BlueCoinUtil::isOnBlueCoinFlagCurrentFile(BLUE_COIN_BOARD_COMPLETE)) {
        s32 completedStages = 0;
        for (s32 i = 0; i < 8; i++) {
            s32 flag = 0;
            MR::getCsvDataS32(&flag, mTable, "BlueCoinFlag", i);
            if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(flag)) {
                const char* nameFromTable;
                s32 scenarioNoFromTable;
                MR::getCsvDataStr(&nameFromTable, mTable, "StageName", i);
                MR::getCsvDataS32(&scenarioNoFromTable, mTable, "ScenarioNo", i);

                if (MR::makeGalaxyStatusAccessor(nameFromTable).hasPowerStar(scenarioNoFromTable))
                    completedStages++;
            }
        }

        if (completedStages == 8)
            BlueCoinUtil::setOnBlueCoinFlagCurrentFile(BLUE_COIN_BOARD_COMPLETE);
    }
}

// This is what happens when I am not given any symbol names
void BlueCoinBoard::connectButtonsToDPad() {
    if (StarPointerUtil::sub_8005E720(this, 1)) {
        //192.3 424.0 TARGET
        //406.0, 228.0 CENTER POINT
        //TARGET - CENTER POINT = NEW POS
        TVec2f vec = TVec2f(-213.7f, 196.0f);
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "Counter", &vec);

        for (s32 i = 0; i < 8; i++) {
            StarPointerUtil::addStarPointerMovePositionFromPane(this, mBoxButtonName[i], StarPointerUtil::getDefaultButtonOffsetVec2());
        }

        for (s32 i = 0; i < 3; i++) {
            StarPointerUtil::setConnectionMovePositionDown2Way(mBoxButtonName[i*2], mBoxButtonName[(i*2)+2]);
            StarPointerUtil::setConnectionMovePositionDown2Way(mBoxButtonName[(i*2)+1], mBoxButtonName[(i*2)+3]);
        }

        for (s32 i = 0; i < 4; i++) {
            StarPointerUtil::setConnectionMovePositionRight2Way(mBoxButtonName[i*2], mBoxButtonName[(i*2)+1]);
        }
    }
    
    StarPointerUtil::setConnectionMovePositionRight2Way("Counter", mBoxButtonName[6]);

    StarPointerUtil::setDefaultAllMovePosition(mBoxButtonName[0]);
    StarPointerUtil::sub_8005E940(this);
    StarPointerUtil::sub_8005E790(this);
}

namespace NrvBlueCoinBoard {
    void NrvAppear::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeAppear();
    }

    void NrvSelecting::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeSelecting();
    }

    void NrvDisappear::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeDisappear();
    }

    void NrvHide::execute(Spine* pSpine) const {}

    void NrvSelected::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeSelected();
    }

    void NrvConfirmUnlock::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeConfirmUnlock();
    }

    void NrvCountDownBlueCoin::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeCountDownBlueCoin();
    }

    void NrvNotEnoughBlueCoins::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeNotEnoughBlueCoins();
    }

    void NrvChangeButtonText::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeChangeButtonText();
    }

    void NrvConfirmPlayStage::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeConfirmPlayStage();
    }

    NrvAppear(NrvAppear::sInstance);
    NrvSelecting(NrvSelecting::sInstance);
    NrvDisappear(NrvDisappear::sInstance);
    NrvHide(NrvHide::sInstance);
    NrvSelected(NrvSelected::sInstance);
    NrvConfirmUnlock(NrvConfirmUnlock::sInstance);
    NrvCountDownBlueCoin(NrvCountDownBlueCoin::sInstance);
    NrvNotEnoughBlueCoins(NrvNotEnoughBlueCoins::sInstance);
    NrvChangeButtonText(NrvChangeButtonText::sInstance);
    NrvConfirmPlayStage(NrvConfirmPlayStage::sInstance);
}
#endif

NameObj* BlueCoinSignUtil::createBlueCoinSign(const char* pName) {
    #ifdef BLUECOINBOARD
        OSReport("Board Created\n");
        return new BlueCoinSign(pName);
    #else
        OSReport("Board Disabled! Returning 0\n");
        return new BlueCoin(pName);
    #endif
}