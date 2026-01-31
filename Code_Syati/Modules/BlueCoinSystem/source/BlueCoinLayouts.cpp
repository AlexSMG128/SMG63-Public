#include "BlueCoinUtil.h"
#include "BlueCoinLayouts.h"
#include "Game/Screen/PauseMenuExt.h"
#include "ExtGameDataUtil.h"
#include "GeneralUtil.h"

#if defined TWN || defined KOR
#define REGIONOFF 0x10
#else
#define REGIONOFF 0
#endif

// PAUSE MENU

void initPauseMenuBlueCoin(PauseMenuExt* pPauseMenu) {
    MR::connectToSceneLayoutOnPause(pPauseMenu);

    MR::setTextBoxFormatRecursive(pPauseMenu, "ShaBlueCoinTotal", counterPictureFonts);
    MR::setTextBoxFormatRecursive(pPauseMenu, "ShaBlueCoinStage", counterPictureFonts);

    BlueCoinList* pBlueCoinList = new BlueCoinList("BlueCoinList");
    pBlueCoinList->initWithoutIter();
    pPauseMenu->mBlueCoinList = pBlueCoinList;

    pPauseMenu->mIsExistBlueCoins = false;

    if (BlueCoinUtil::getBlueCoinRange(0, false) > -1)
        pPauseMenu->mIsExistBlueCoins = true;
}

kmCall(0x80486D60+REGIONOFF, initPauseMenuBlueCoin); // bl initPauseMenuBlueCoin

void setPauseMenuBlueCoinStageCount(PauseMenuExt* pPauseMenu) {
    MR::setTextBoxArgNumberRecursive(pPauseMenu, "ShaBlueCoinTotal", BlueCoinUtil::getTotalBlueCoinNumCurrentFile(false), 0);
    MR::setTextBoxFormatRecursive(pPauseMenu, "ShaCoinListWin", L"");
    
    if (pPauseMenu->mIsExistBlueCoins) {
        s32 rangeCollected = BlueCoinUtil::calcBlueCoinTotalInRange(0, true);
        s32 rangeTotal = BlueCoinUtil::calcBlueCoinTotalInRange(0, false);

        wchar_t str[5];
        pt::str2wcsfullwidth(str, rangeTotal);
        MR::setTextBoxFormatRecursive(pPauseMenu, "ShaBlueCoinNum", str);
    
        MR::setTextBoxArgNumberRecursive(pPauseMenu, "ShaBlueCoinStage", rangeCollected, 0);
        MR::showPaneRecursive(pPauseMenu, "ShaBlueCoinStage");
    }
    else
        MR::hidePaneRecursive(pPauseMenu, "ShaBlueCoinStage");
}

const char* savePauseMenuIsInvalidBack(PauseMenuExt* pPauseMenu, bool isInvalidBack) {
    pPauseMenu->mIsInvalidBack = isInvalidBack;

    const char* pLabel = "PauseMenu_ButtonBackWorldMap";

    if (isInvalidBack)
        pLabel = "PauseMenu_ButtonEndGame";

    return pLabel;
}

kmWrite32(0x80486EA0, 0x7C641B78); // mr r4, r3
kmWrite32(0x80486EA4, 0x7F63DB78); // mr r3, r27
kmCall(0x80486EA8, savePauseMenuIsInvalidBack); // bl savePauseMenuIsInvalidBack
kmWrite32(0x80486EAC, 0x7C651B78); // mr r5, r3

s32 setUpBlueCoinInfoOnAppear(PauseMenuExt* pPauseMenu) {
    setPauseMenuBlueCoinStageCount(pPauseMenu);
    MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, "ListButton", "ChangeList", 0.0f, 1);
    pPauseMenu->mDisplayMode = 0;  
    
    if (pPauseMenu->mIsExistBlueCoins) {
        s32 rangemin = BlueCoinUtil::getBlueCoinRange(0, false);
        wchar_t gIDListStr[32];
        wchar_t gCompleteIcon[2];
        wchar_t gStarIcon[2];
        wchar_t gBButtonIcon[2];

        s32 totalCoins = (BlueCoinUtil::getBlueCoinRange(0, true)-rangemin)+1;
        s32 newLineOff = 0;
        s32 collectedCount = 0;
        bool newLineAdded = false;
        bool stageCheck = MR::isStageMarioFaceShipOrWorldMap() || MR::isStageNoPauseMenuStars() || MR::isStageStoryBook();

        if (totalCoins > 30)
            totalCoins = 30;

        MR::showPaneRecursive(pPauseMenu, "BlueCoinAmounts");
        MR::showPaneRecursive(pPauseMenu, "CoinListIcons");
        MR::hidePane(pPauseMenu, "TxtCoinComplete");

        if (stageCheck) {
            MR::showPaneRecursive(pPauseMenu, "StageInfo");
            MR::setTextBoxMessageRecursive(pPauseMenu, "StageTitle", MR::getCurrentGalaxyNameOnCurrentLanguage());
            MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, "StageInfo", "Change", 2.0f, 1);
        }
        else  {
            MR::addPictureFontCode(gStarIcon, 0xC1);
            MR::setTextBoxFormatRecursive(pPauseMenu, "TxtCoinPage", gStarIcon);

            MR::addPictureFontCode(gBButtonIcon, 0x31);
            MR::setTextBoxFormatRecursive(pPauseMenu, "TxtCoinBButton", gBButtonIcon);
            MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, "StageInfo", "Change", 0.0f, 1);
        }

        for (s32 i = 0; i < totalCoins + (s32)(totalCoins > 15); i++) {
            newLineAdded = 0;

            if (i == totalCoins/2 && totalCoins > 15) {
                MR::addNewLine(&gIDListStr[i]);
                newLineOff++;
                newLineAdded = true;
            }

            if (!newLineAdded) {
                bool isGot = BlueCoinUtil::isBlueCoinGotCurrentFile((i+rangemin)-newLineOff);
                int icon = 0xC0;

                if (isGot) {
                    icon = 0x8A;
                    collectedCount++;
                }

                MR::addPictureFontCode(&gIDListStr[i], icon);
            }
        }

        MR::addPictureFontCode(&gCompleteIcon[0], collectedCount == totalCoins ? 0x50 : 0x52);

        MR::setTextBoxFormatRecursive(pPauseMenu, "TxtCoinComplete", gCompleteIcon);

        MR::setTextBoxFormatRecursive(pPauseMenu, "CoinListWin", gIDListStr);
    }

    return MR::getCoinNum();
}

kmWrite32(0x80486D14+REGIONOFF, 0x38A00002); // li r5, 2
kmWrite32(0x80486D54+REGIONOFF, 0x38A00002); // li r5, 2
kmCall(0x80487090+REGIONOFF, setUpBlueCoinInfoOnAppear); // bl setUpBlueCoinInfo

void PauseMenuIDListControls(PauseMenuExt* pPauseMenu) {
    bool stagecheck = MR::isStageNoPauseMenuStars() || MR::isStageStoryBook() || MR::isStageMarioFaceShipOrWorldMap();
    wchar_t gStarIconIDList[2];

    if (MR::testCorePadTriggerB(0) && !stagecheck) {
        if (pPauseMenu->mDisplayMode == 0)
            pPauseMenu->mDisplayMode = 1;
        else
            pPauseMenu->mDisplayMode = 0;

        const char* pLabel = "PauseMenu_StarList";

        if (pPauseMenu->mDisplayMode == 1)
            pLabel = "PauseMenu_BlueCoinList";

        f32 frame = (f32)pPauseMenu->mDisplayMode;
        
        if (pPauseMenu->mIsExistBlueCoins) {
            MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, "StageInfo", "Change", frame, 1);
            MR::addPictureFontCode(gStarIconIDList, pPauseMenu->mDisplayMode > 0 ? 0xC2 : 0xC1);
            MR::setTextBoxFormatRecursive(pPauseMenu, "TxtCoinPage", gStarIconIDList);
        }
    }
}

bool PauseMenuIsStageBlueCoin(PauseMenuExt* pPauseMenu, int unused, const char* pStr) {
    MR::startPaneAnim(pPauseMenu, "StageInfo", pStr, 0);

    return pPauseMenu->mIsExistBlueCoins;
}

kmCall(0x80487424+REGIONOFF, PauseMenuIsStageBlueCoin);
kmWrite32(0x80487428+REGIONOFF, 0x2C030001);
kmWrite32(0x8048742C+REGIONOFF, 0x4182007C);

#ifndef PAUSEMENUNEWBUTTON
PauseMenuExt* createPauseMenuExt() {
    return new PauseMenuExt();
}

kmCall(0x804712C0, createPauseMenuExt); // bl createPauseMenuExt
kmWrite32(0x804712C4, 0x48000010); // b 0x10

// EXTENDED PAUSEMENU
PauseMenuExt::PauseMenuExt() : PauseMenu() {
    // BCS
    mBlueCoinList = 0;
    mDisplayMode = 0;
    mIsInvalidBack = 0;

    mButtonNew = 0;
    mButtonNewFollowPos = TVec2f(0.0f, 0.0f);
    mIsUsedNewButton = false;
}
bool PauseMenuIsNewButtonPointingTrigger(PauseMenuExt* pPauseMenu) {
    PauseMenuIDListControls(pPauseMenu);
    return (pPauseMenu->mButtonTop && pPauseMenu->mButtonTop->isPointingTrigger());
}

PauseMenuExt::~PauseMenuExt() {

}

kmWrite32(0x80487714+REGIONOFF, 0x7F63DB78); // mr r3, r27 (PauseMenuExt* into r3)
kmCall(0x80487720+REGIONOFF, PauseMenuIsNewButtonPointingTrigger);

void PauseMenuMoveButtonForBlueCoin(PauseMenuExt* pPauseMenu, const char* pStr1, const char* pStr2, f32 frame, u32 u) {
    if (BlueCoinUtil::calcBlueCoinTotalInRange(MR::getCurrentStageName(), false) != -1) {
        frame = 2.0f;
    }
    MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, pStr1, pStr2, frame, u);
}

kmCall(0x804874D4+REGIONOFF, PauseMenuMoveButtonForBlueCoin); // bl PauseMenuMoveButtonForBlueCoin
#endif

void setPauseMenuNerve(PauseMenuExt* pPauseMenu, const Nerve* pNerve) {
    if (pPauseMenu->mIsUsedBlueCoinButton)
        pNerve = &NrvPauseMenuExt::NrvPauseMenuExtBlueCoinList::sInstance;

    pPauseMenu->setNerve(pNerve);
}

kmCall(0x80487BD0, setPauseMenuNerve);

void PauseMenuExt::exeBlueCoinList() {
    if (MR::isFirstStep(this))
        mBlueCoinList->appear();
    
    if (MR::isDead(mBlueCoinList)) {
        mIsUsedBlueCoinButton = false;
        __kAutoMap_80487540(this);
    }
}


namespace NrvPauseMenuExt {
    void NrvPauseMenuExtBlueCoinList::execute(Spine* pSpine) const {
        ((PauseMenuExt*)pSpine->mExecutor)->exeBlueCoinList();
    }

    NrvPauseMenuExtBlueCoinList(NrvPauseMenuExtBlueCoinList::sInstance);
}

void initBlueCoinCounterFileInfo(LayoutActor* pLayout) {
    MR::connectToSceneLayout(pLayout);
    MR::showPaneRecursive(pLayout, "BlueCoinFileInfo");
    MR::setTextBoxFormatRecursive(pLayout, "ShaBlueCoinFileInfo", counterPictureFonts);
    *((u32*)pLayout+0x44) = 0;
}

kmCall(0x8046D908, initBlueCoinCounterFileInfo); // bl initBlueCoinCounterFileInfo

s32 getBlueCoinNumFromUserFile(UserFile* pFile) {
    int* pFileSelector = 0;
    asm("mr %0, r30" : "=r" (pFileSelector));
    u32 pFileSelectInfo = *((u32*)pFileSelector+0x30);
    BlueCoinDataStorage* pStorage = ((ExtGameDataHolder*)pFile->mGameDataHolder)->mBlueCoinDataStorage;
    *((u32*)pFileSelectInfo+0x44) = BlueCoinUtil::getTotalBlueCoinNum(pStorage, false);
    return pFile->getPowerStarNum();
}

kmCall(0x802504A4, getBlueCoinNumFromUserFile);

kmWrite32(0x8024FB24, 0x38600114); // li r3, 0x114

void displayNewVar(LayoutActor* pActor, const char* pStr, s32 l1) {
    MR::setTextBoxNumberRecursive(pActor, pStr, l1);
    MR::setTextBoxArgNumberRecursive(pActor, "ShaBlueCoinFileInfo", *((u32*)pActor+0x44), 0);
}

kmCall(0x8046DCF8, displayNewVar);

#ifdef DISABLED
void initGalaxyInfoBlueCoinCount(LayoutActor* actor) {
    MR::setTextBoxFormatRecursive(actor, "ShaBlueCoinGalax", counterPictureFonts2);
}

#if defined TWN || defined KOR
kmCall(0x804A952C, initGalaxyInfoBlueCoinCount); // bl initGalaxyInfoBlueCoinCount
#else
kmCall(0x804A952C, initGalaxyInfoBlueCoinCount); // bl initGalaxyInfoBlueCoinCount
#endif

void setGalaxyInfoBlueCoinCount(LayoutActor* actor, const char* pGalaxyName, const wchar_t* pWStr) {
    MR::setTextBoxMessageRecursive(actor, "StarIcon", pWStr);

    s32 rangeNum = BlueCoinUtil::calcBlueCoinTotalInRange(pGalaxyName, true);
    MR::hidePaneRecursive(actor, "BlueCoin");

    if (rangeNum != -1) {
        MR::showPane(actor, "BlueCoin");
        MR::showPaneRecursive(actor, "ShaBlueCoinGalax");
        MR::setTextBoxFormatRecursive(actor, "ShaBlueCoinGalax", L"%ls%d", counterPictureFonts2, rangeNum);
    }
}

kmWrite32(0x804A95E4, 0x809B003C); // lwz r4, 0x3C(r27)
kmCall(0x804A95E8, setGalaxyInfoBlueCoinCount); // bl setGalaxyInfoBlueCoinCount
#endif

void onGLEScenarioSelectInit(LayoutActor* pLayout) {
    wchar_t strBuf[16];
    
    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    MR::startPaneAnim(pLayout, "ScenarioFrame", "GalaxyMove", 1);
    if (coinMin == -1) {
        MR::setPaneAnimFrameAndStop(pLayout, "ScenarioFrame", 0.0f, 1);
        MR::setTextBoxFormatRecursive(pLayout, "ShaBlueCoin", L"");
        return;
    }
    MR::setPaneAnimFrameAndStop(pLayout, "ScenarioFrame", 1.0f, 1);

    s32 coinMax = BlueCoinUtil::getBlueCoinRange(0, true);
    s32 coinNum = (coinMax+1)-coinMin;
    int icon;

    for (s32 i = 0; i < coinNum; i++) {
        if (BlueCoinUtil::isBlueCoinGotCurrentFile(coinMin+i))
            icon = 0x8A;
        else
            icon = 0xC0;

        MR::addPictureFontCode(&strBuf[i], icon);
        OSReport("%x ", icon);
    }
    OSReport("\n");
    
    strBuf[coinNum] = L'\0';
    MR::setTextBoxFormatRecursive(pLayout, "ShaBlueCoin", strBuf);
}

void onGLEScenarioSelectSelect(LayoutActor* pLayout) {

    if (!MR::isStep(pLayout, 60))
        return;

    s32 coinMin = BlueCoinUtil::getBlueCoinRange(0, false);
    
    if (coinMin != -1) 
        MR::startPaneAnim(pLayout, "ScenarioFrame", "GalaxyReturn", 1);
}

void onGLEGalaxyInfoInit(SimpleLayout* pLayout, const Nerve* pNerve) {
    pLayout->initNerve(pNerve);
    MR::hidePaneRecursive(pLayout, "BlueCoinCurrent");
}

kmCall(0x804A97A8, onGLEGalaxyInfoInit);

void onGLEGalaxyInfoAppear(SimpleLayout* pLayout, const char* pStr) {
    s32 coinMin = BlueCoinUtil::getBlueCoinRange(pStr, false);

    if (coinMin == -1)
        return;

    MR::showPaneRecursive(pLayout, "BlueCoinCurrent");
    s32 coinMax = BlueCoinUtil::getBlueCoinRange(pStr, true);
    s32 coinsCollected = BlueCoinUtil::calcBlueCoinTotalInRange(pStr, true);
    s32 coinTotal = coinMax-(coinMin-1);
    MR::setTextBoxFormatRecursive(pLayout, "ShaBlueCoinStage", L"%ls%d", counterPictureFonts2, coinsCollected);
    wchar_t str[5];
    pt::str2wcsfullwidth(str, coinTotal);
    MR::setTextBoxFormatRecursive(pLayout, "ShaBlueCoinNum", str);
}

bool onGLEGalaxyInfoHide(LayoutActor* pLayout, u32 l) {
    if (!MR::isAnimStopped(pLayout, l))
        return false;
    
    MR::hidePaneRecursive(pLayout, "BlueCoinCurrent");
    return true;
}

kmCall(0x804A9A4C, onGLEGalaxyInfoHide);