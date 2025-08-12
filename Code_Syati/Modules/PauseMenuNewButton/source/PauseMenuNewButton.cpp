#include "syati.h"
#include "Game/Screen/PauseMenuExt.h"

PauseMenuExt* createPauseMenuExt() {
    return new PauseMenuExt();
}

kmCall(0x804712C0, createPauseMenuExt);
kmWrite32(0x804712C4, 0x48000010); // b 0x10

PauseMenuExt::PauseMenuExt() : PauseMenu() {
    mBlueCoinList = 0;
    mDisplayMode = 0;
    mIsInvalidBack = 0;
    mIsExistBlueCoins = 0;
    mButtonBlueCoinList = 0;
    mButtonBlueCoinFollowPos = TVec2f(0.0f, 0.0f);
    mIsUsedBlueCoinButton = false;

    mButtonNew = 0;
    mButtonNewFollowPos = TVec2f(0.0f, 0.0f);
    mIsUsedNewButton = false;
}

PauseMenuExt::~PauseMenuExt() {

}

#if defined TWN || defined KOR
#define REGIONOFF 0x10
#else
#define REGIONOFF 0
#endif

#ifdef SMSS
    #define STAGE_CHECK MR::isStageMarioFaceShipOrWorldMap() || MR::isEqualStageName("PrisonGalaxy")
#else
    #define STAGE_CHECK MR::isStageMarioFaceShipOrWorldMap() || MR::isEqualStageName("PeachCastleGalaxy")
#endif


void PauseMenuInitNewButton(PauseMenuExt* pPauseMenu, const Nerve* pNerve) { 
    MR::createAndAddPaneCtrl(pPauseMenu, "NewButton", 1);
    MR::createAndAddPaneCtrl(pPauseMenu, "CoinButton", 1);

    bool isNewCometInStage = false;

    #ifdef PMNBCOMETRESTRICTION
        GalaxyStatusAccessor gsa = MR::makeGalaxyStatusAccessor(MR::getCurrentStageName());
        isNewCometInStage = gsa.isValidComet(MR::getCurrentScenarioNo()) && !gsa.hasPowerStar(MR::getCurrentScenarioNo());
    #endif

    if (!MR::isStageStoryBook()) {
        pPauseMenu->mButtonBlueCoinList = new ButtonPaneController(pPauseMenu, "CoinListButton", "BoxButton5", 0, 1);
        pPauseMenu->mButtonBlueCoinList->mFadeAfterSelect = false;
        MR::setFollowPos(&pPauseMenu->mButtonBlueCoinFollowPos, pPauseMenu, "CoinButton");
        MR::setFollowTypeReplace(pPauseMenu, "CoinButton");
    }

    if (!(STAGE_CHECK) && !isNewCometInStage) {
        pPauseMenu->mButtonNew = new ButtonPaneController(pPauseMenu, "NBackNew", "BoxButton4", 0, 1);
        pPauseMenu->mButtonNew->mFadeAfterSelect = false;
    
        const wchar_t* pStr = L"Restart Stage";
        
        TalkMessageInfo info;
        MessageSystem::getGameMessageDirect(&info, "PauseMenu_ButtonRestartStage");

        if (info.mMessage)
            pStr = info.mMessage;

        MR::setTextBoxFormatRecursive(pPauseMenu, "Text4", pStr);

        MR::setFollowPos(&pPauseMenu->mButtonNewFollowPos, pPauseMenu, "NewButton");
        MR::setFollowTypeReplace(pPauseMenu, "NewButton");
        MR::showPaneRecursive(pPauseMenu, "NewButton");
    }
    else
        MR::hidePaneRecursive(pPauseMenu, "NBackNew");

    pPauseMenu->initNerve(pNerve);
}

kmCall(0x8048702C+REGIONOFF, PauseMenuInitNewButton);


// Crashing?
void ButtonControl(TVec2f* pPos, PauseMenuExt* pPauseMenu, const char* pStr) {
    MR::copyPaneTrans(&pPauseMenu->mButtonTopFollowPos, pPauseMenu, pStr);
    //OSReport("Top: %.0f, %.0f Bottom: %.0f, %.0f, New: %.0f, %.0f\n", pPauseMenu->mButtonTopFollowPos.x, pPauseMenu->mButtonTopFollowPos.y, pPauseMenu->mButtonBottomFollowPos.x, pPauseMenu->mButtonBottomFollowPos.y, pPauseMenu->mButtonNewFollowPos.x, pPauseMenu->mButtonNewFollowPos.y);
    
    if (pPauseMenu->mButtonNew) {
        MR::copyPaneTrans(&pPauseMenu->mButtonNewFollowPos, pPauseMenu, "NewButtonPos");
        pPauseMenu->mButtonNew->update();
    }

    if (pPauseMenu->mButtonBlueCoinList) {
        MR::copyPaneTrans(&pPauseMenu->mButtonBlueCoinFollowPos, pPauseMenu, "CoinButtonPos");
        pPauseMenu->mButtonBlueCoinList->update();
    }
}

kmCall(0x8048727C+REGIONOFF, ButtonControl);

void PauseMenuSetButtonPosition(PauseMenuExt* pPauseMenu, const char* pStr1, const char* pStr2, f32 frame, u32 u) { 
    if (pPauseMenu->mButtonNew)
        pStr2 = "ButtonPositionR";

    MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, pStr1, pStr2, frame, u);
}

kmCall(0x804874D4+REGIONOFF, PauseMenuSetButtonPosition);

void ForceToWaitNewButton(PauseMenuExt* pPauseMenu) {
    pPauseMenu->mButtonTop->forceToWait();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->forceToWait();

    if (pPauseMenu->mButtonBlueCoinList)
        pPauseMenu->mButtonBlueCoinList->forceToWait();
}

kmWrite32(0x80487504+REGIONOFF, 0x60000000); // nop (Skip overwriting r3)
kmCall(0x80487508+REGIONOFF, ForceToWaitNewButton);

void PauseMenuAppearNewButton(PauseMenuExt* pPauseMenu) {
    pPauseMenu->mButtonTop->appear();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->appear();

    if (pPauseMenu->mButtonBlueCoinList)
        pPauseMenu->mButtonBlueCoinList->appear();
}

kmWrite32(0x80487560+REGIONOFF, 0x7FE3FB78); // mr r3, r31 (PauseMenuExt* into r3)
kmCall(0x80487564+REGIONOFF, PauseMenuAppearNewButton);

bool IsNewButtonPressed(PauseMenuExt* pPauseMenu) {
    bool isPressed = false;

    if (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->trySelect()) {
        pPauseMenu->mIsUsedNewButton = false;
        isPressed = true;
    }

    if (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->trySelect()) {
        pPauseMenu->mIsUsedNewButton = true;
        isPressed = true;
    }

    return isPressed;
}

kmWrite32(0x804877B4+REGIONOFF, 0x7F63DB78); // mr r3, r27 (PauseMenuExt* into r3)
kmCall(0x804877C0+REGIONOFF, IsNewButtonPressed);

bool isListButtonSelect(PauseMenuExt* pPauseMenu) {
    bool isStarListSelect = pPauseMenu->mButtonStarList && pPauseMenu->mButtonStarList->trySelect();
    bool isCoinListSelect = pPauseMenu->mButtonBlueCoinList && pPauseMenu->mButtonBlueCoinList->trySelect();
    return isStarListSelect || isCoinListSelect;
}

kmWrite32(0x804877CC, 0x7F63DB78); // mr r3, r27
kmCall(0x804877D0, isListButtonSelect); // bl isListButtonSelect
kmWrite32(0x804877D4, 0x2C030000); // cmpwi r3, 0
kmWrite32(0x804877D8, 0x4182001C); // beq 0x1C
kmWrite32(0x804877DC, 0x48000008); // b 0x8

void PauseMenuSetInfoWindowStr(PauseMenuExt* pPauseMenu, const char* pStr) {
    if (pPauseMenu->mIsUsedNewButton)
        pStr = "PauseMenu_ConfirmRestartStage";

    pPauseMenu->mSysInfoWindow->appear(pStr, SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);
}

kmWrite32(0x80487C4C+REGIONOFF, 0x7FE3FB78); // mr r3, r31 (PauseMenuExt* into r4)
kmCall(0x80487C5C+REGIONOFF, PauseMenuSetInfoWindowStr);

void DisappearNewButton(PauseMenuExt* pPauseMenu) {
    pPauseMenu->mButtonTop->disappear();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->disappear();

    if (pPauseMenu->mButtonBlueCoinList)
        pPauseMenu->mButtonBlueCoinList->disappear();
}

kmWrite32(0x80487B10+REGIONOFF, 0x7FC3F378); // mr r3, r30 (PauseMenuExt* into r3)
kmCall(0x80487B14+REGIONOFF, DisappearNewButton);

bool IsNewButtonTimingForSelectedSE(PauseMenuExt* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->isTimingForSelectedSe()) || 
    (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isTimingForSelectedSe());
}

kmWrite32(0x804879C8+REGIONOFF, 0x7FC3F378); // mr r3, r30
kmCall(0x804879CC+REGIONOFF, IsNewButtonTimingForSelectedSE);

bool PauseMenuValidateButton(PauseMenuExt* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->_24) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->_24);
}

kmWrite32(0x804879B0+REGIONOFF, 0x7FC3F378); // mr r3, r30
kmCall(0x804879B4+REGIONOFF, PauseMenuValidateButton);
kmWrite32(0x804879B8+REGIONOFF, 0x2C030000); // cmpwi r3, 0
kmWrite32(0x804879BC+REGIONOFF, 0x41820080); // beq 0x80
kmWrite32(0x804879C0+REGIONOFF, 0x48000008); // b 0x8


bool IsNewButtonDecidedWait(PauseMenuExt* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->isDecidedWait()) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isDecidedWait());
}

kmWrite32(0x80487A00+REGIONOFF, 0x7FC3F378); // mr r3, r30 (PauseMenuExt* into r3)
kmCall(0x80487A04+REGIONOFF, IsNewButtonDecidedWait);


bool isListButtonPressed(PauseMenuExt* pPauseMenu) {
    bool isListButtonPressed = pPauseMenu->mButtonStarList && pPauseMenu->mButtonStarList->_24;
    bool isCoinButtonPressed = pPauseMenu->mButtonBlueCoinList && pPauseMenu->mButtonBlueCoinList->_24;
    return isListButtonPressed || isCoinButtonPressed;
}

kmWrite32(0x80487A3C, 0x7FC3F378); // mr r3, r30
kmCall(0x80487A40, isListButtonPressed); // bl isListButtonPressed
kmWrite32(0x80487A44, 0x2C030000); // cmpwi r3, 0
kmWrite32(0x80487A48, 0x41820064); // beq 0x64
kmWrite32(0x80487A4C, 0x60000000); // beq 0x64
kmWrite32(0x80487A50, 0x7FC3F378); // mr r3, r30


bool isNewListButtonTimingSelectedSE(PauseMenuExt* pPauseMenu) {
    bool isListButtonTiming = pPauseMenu->mButtonStarList->isTimingForSelectedSe();
    bool isCoinButtonTiming = pPauseMenu->mButtonBlueCoinList->isTimingForSelectedSe();
    return isListButtonTiming || isCoinButtonTiming;
}

kmCall(0x80487A54, isNewListButtonTimingSelectedSE);

bool isNewListButtonDecidedWait(PauseMenuExt* pPauseMenu) {
    bool isListButtonDecided = pPauseMenu->mButtonStarList->isDecidedWait();
    bool isCoinButtonDecided = pPauseMenu->mButtonBlueCoinList->isDecidedWait();

    if (isCoinButtonDecided)
        pPauseMenu->mIsUsedBlueCoinButton = true;
        
    return isListButtonDecided || isCoinButtonDecided;
}

kmWrite32(0x80487A88, 0x7FC3F378); // mr r3, r30
kmCall(0x80487A8C, isNewListButtonDecidedWait);

void DoNewButtonAction(PauseMenuExt* pPauseMenu, bool isValid) {
    if (!pPauseMenu->mIsUsedNewButton)
        isValid ? GameSequenceFunction::notifyToGameSequenceProgressToEndScene() : GameSequenceFunction::requestChangeStageWorldMap();
    else {
        GameSequenceFunction::requestChangeScenarioSelect(MR::getCurrentStageName());
		GameSequenceFunction::requestChangeStage(MR::getCurrentStageName(), MR::getCurrentScenarioNo(), MR::getCurrentSelectedScenarioNo(), JMapIdInfo(0, 0));
    }
}

kmWrite32(0x80487CA4+REGIONOFF, 0x7C641B78); // mr r4, r3 (isValid into r4)
kmWrite32(0x80487CA8+REGIONOFF, 0x7FE3FB78); // mr r3, r31 (PauseMenuExt* into r3)
kmCall(0x80487CAC+REGIONOFF, DoNewButtonAction);
kmWrite32(0x80487CB0+REGIONOFF, 0x48000008); // b 0x8 (Skip useless instructions)

void addStarPointerMovePositionNewButton(PauseMenuExt* pPauseMenu, const char* pStr, TVec2f* pOffsetVec) {
    StarPointerUtil::addStarPointerMovePositionFromPane(pPauseMenu, pStr, pOffsetVec);
    TVec2f pVec = TVec2f(0.0f, 0.0f);
    
    if (pPauseMenu->mButtonNew)
        StarPointerUtil::addStarPointerMovePositionFromPane(pPauseMenu, "BoxButton4", &pVec);

    if (pPauseMenu->mButtonBlueCoinList)
        StarPointerUtil::addStarPointerMovePositionFromPane(pPauseMenu, "BoxButton5", &pVec);
}

kmCall(0x804875F0+REGIONOFF, addStarPointerMovePositionNewButton);

void addStarPointerForStarList(PauseMenuExt* pPauseMenu, const char* pStr) {
    TVec2f pVec = TVec2f(0.0f, 0.0f);
    
    StarPointerUtil::addStarPointerMovePositionFromPane(pPauseMenu, "BoxButton3", &pVec);
};

kmWrite32(0x80487620+REGIONOFF, 0x48000008); // b 0x8
kmCall(0x80487630+REGIONOFF, addStarPointerForStarList);

void setupNewConnection1to2(PauseMenuExt* pPauseMenu) {
    s32 f = MR::getPaneAnimFrame(pPauseMenu, "ButtonPosition", 0);

    if (pPauseMenu->mButtonNew && (f == 1 || f == 2))
        StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton4", "BoxButton2");

    StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton2");

    if (pPauseMenu->mButtonBlueCoinList)
        StarPointerUtil::setConnectionMovePositionRight2Way("BoxButton3", "BoxButton5");
}

kmWrite32(0x80487640+REGIONOFF, 0x60000000); // nop
kmWrite32(0x80487644+REGIONOFF, 0x7FC3F378); // mr r3, r30

kmCall(0x80487648+REGIONOFF, setupNewConnection1to2);

void setupButtonConnection(PauseMenuExt* pPauseMenu) {
    StarPointerUtil::setDefaultAllMovePosition("BoxButton1");
    s32 f = MR::getPaneAnimFrame(pPauseMenu, "ButtonPosition", 0);
    OSReport("f: %d\n", f);

    if (pPauseMenu->mButtonTop && pPauseMenu->mButtonNew) {
        switch ((s32)f) {
            case 0:
            case 3:
            case 4:
            StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton4");
            break;
            case 1:
            case 2:
            StarPointerUtil::setConnectionMovePositionRight2Way("BoxButton1", "BoxButton4");
            break;
        }
    }
}

#ifdef BLUECOINSYSTEM
extern void PauseMenuIDListControls(PauseMenuExt* pPauseMenu);
#endif

kmWrite32(0x804876C0+REGIONOFF, 0x7FC3F378); // mr r3, r30
kmCall(0x804876C4+REGIONOFF, setupButtonConnection);

bool PauseMenuIsNewButtonPointingTrigger(PauseMenuExt* pPauseMenu) {
    #ifdef BLUECOINSYSTEM
        PauseMenuIDListControls(pPauseMenu);
    #endif
    
    return (pPauseMenu->mButtonTop && pPauseMenu->mButtonTop->isPointingTrigger()) || 
    (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isPointingTrigger() ||
    (pPauseMenu->mButtonBlueCoinList) && (pPauseMenu->mButtonBlueCoinList->isPointingTrigger()));
}

kmWrite32(0x80487714+REGIONOFF, 0x7F63DB78); // mr r3, r27 (PauseMenuExt* into r3)
kmCall(0x80487720+REGIONOFF, PauseMenuIsNewButtonPointingTrigger);

#ifdef WIP
s32 gRestartTimer = 0;

void handleRestartStageButtonCombination(NerveExecutor* pExecutor) {
    pExecutor->updateNerve();

    if (MR::testSubPadButtonC(0) && MR::testCorePadButtonDown(0) && !STAGE_CHECK)
        gRestartTimer++;
    else
        gRestartTimer = 0;

    if (gRestartTimer == 30) {
        GameSequenceFunction::requestChangeScenarioSelect(MR::getCurrentStageName());
		GameSequenceFunction::requestChangeStage(MR::getCurrentStageName(), MR::getCurrentScenarioNo(), MR::getCurrentSelectedScenarioNo(), JMapIdInfo(0, 0));
        gRestartTimer = 0;
    }
}

kmCall(0x804518B0, handleRestartStageButtonCombination);
#endif