#include "syati.h"

#ifdef BLUECOINBOARD
#include "BlueCoinUtil.h"
#include "Game/Screen/ButtonPaneController.h"
#include "Game/Screen/SysInfoWindow.h"
#include "Game/Screen/CountUpPaneRumbler.h"
#include "Game/Screen/BackButton.h"
#include "Game/Map/PauseBlur.h"
#pragma once

#define BLUE_COIN_BOARD_COMPLETE 8

class BlueCoinBoard : public LayoutActor {
public:
    BlueCoinBoard(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void appear();
    void exeAppear();
    void exeSelecting();
    void exeDisappear();
    void exeSelected();
    void exeConfirmUnlock();
    void exeCountDownBlueCoin();
    void exeNotEnoughBlueCoins();
    void exeChangeButtonText();
    void exeConfirmPlayStage();
    void checkBoardProgress();
    void connectButtonsToDPad();

    ButtonPaneController* mButtons[8];
    SysInfoWindow* mSysInfoWindowSelect;
    SysInfoWindow* mSysInfoWindowBox;
    CountUpPaneRumbler* mBlueCoinPaneRumbler;
    BackButtonCancelB* mBackButton;
    const JMapInfo* mTable;
    TVec2f mButtonFollowPositions[8];
    TVec2f mBlueCoinCounterFollowPos;
    char mBoxButtonName[8][12];
    char mButtonName[8][13];
    char mButtonTxtName[8][13];
    char mButtonFollow[8][9];
    char mCopyPosName[8][12];
    s32 mSelectedButton;
    s32 mBlueCoinNumToDisplay;
    s32 mTotalBlueCoinPrices;
    s32 mCompleteFlag;
    bool mHasSpentBlueCoins;
};

namespace NrvBlueCoinBoard {
    NERVE(NrvAppear);
    NERVE(NrvSelecting);
    NERVE(NrvDisappear);
    NERVE(NrvHide);
    NERVE(NrvSelected);
    NERVE(NrvConfirmUnlock);
    NERVE(NrvCountDownBlueCoin);
    NERVE(NrvNotEnoughBlueCoins);
    NERVE(NrvChangeButtonText);
    NERVE(NrvConfirmPlayStage);
};

class BlueCoinSign : public NPCActor {
public:
    BlueCoinSign(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    bool eventFunc(u32 eventParam);
    void exeWait();
    void exeOpen();

    BlueCoinBoard* pBoard;
};

namespace NrvBlueCoinSign {
    NERVE(NrvWait);
    NERVE(NrvOpen);
};
#endif

namespace BlueCoinSignUtil {
    NameObj* createBlueCoinSign(const char* pName);
};