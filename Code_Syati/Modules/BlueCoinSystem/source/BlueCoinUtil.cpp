#include "BlueCoin.h"
#include "BlueCoinCounter.h"
#include "BlueCoinUtil.h"
#include "BlueCoinDataStorage.h"
#include "ExtGameDataUtil.h"
#include "Game/Screen/GameSceneLayoutHolder.h"
#include "Game/Screen/CounterLayoutControllerExt.h"
#include "Game/NPC/TalkMessageCtrl.h"
#include "Game/MapObj/FileSelector.h"
#include "Game/LiveActor/ExtActorActionKeeper.h"
#include "BlueCoinStageProgress.h"


void* gBlueCoinIDRangeTable = pt::loadArcAndFile("/SystemData/BlueCoinIDRangeTable.arc", "/BlueCoinIDRangeTable.bcsv", 0);
    
namespace BlueCoinUtil {
    s32 getCurrentFileNum() {
        return GameDataFunction::getSaveDataHandleSequence()->mCurrentFileNum-1;
    }

    void setBlueCoinGotCurrentFile(u16 id) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        pStorage->mCollectionData->set(id, true);
    }
    
    bool isBlueCoinGotCurrentFile(u16 id) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        return pStorage->mCollectionData->isOn(id);
    }

    bool hasSeenBlueCoinTextBoxCurrentFile() {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        return pStorage->mHasSeenTextBox;
    }  

    void setSeenBlueCoinTextBoxCurrentFile() {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        pStorage->mHasSeenTextBox = true;
    }

    void setOnBlueCoinFlagCurrentFile(u8 flag) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        pStorage->mFlags->set(flag, true);
    }

    bool isOnBlueCoinFlagCurrentFile(u8 flag) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        return pStorage->mFlags->isOn(flag);
    }
    
    bool isBlueCoinTextBoxAppeared() {
        return MR::isDemoActive("BlueCoinFirstTimeText");
    }

    void setCounter() {
        CounterLayoutControllerExt* pCounterLayoutControllerExt = (CounterLayoutControllerExt*)MR::getGameSceneLayoutHolder()->mCounterLayoutController;
        BlueCoinCounter* pCounter = pCounterLayoutControllerExt->mBlueCoinCounter;
        
        if (pCounter)
            pCounter->setCounter();
    }

    void showTextBox() {
        ((CounterLayoutControllerExt*)MR::getGameSceneLayoutHolder()->mCounterLayoutController)->mBlueCoinCounter->setNerve(&NrvBlueCoinCounter::NrvShowTextBox::sInstance);
    }

    void spendBlueCoinCurrentFile(u16 numcoin) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        numcoin == 0 ? 30 : numcoin;
        
        if (getTotalBlueCoinNum(pStorage, true) >= numcoin)
            pStorage->mSpentData += numcoin;
    }


    u16 getSpentBlueCoinNum(BlueCoinDataStorage* pStorage) {
        return pStorage->mSpentData;
    }

    u16 getSpentBlueCoinNumCurrentFile() {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        return getSpentBlueCoinNum(pStorage);
    }

    s32 getTotalBlueCoinNum(BlueCoinDataStorage* pStorage, bool ignoreSpent) {

        s32 total = 0;
        for (s32 i = 0; i < COLLECTIONCOUNT; i++) {
            if (pStorage->mCollectionData->isOn(i))
                total++;
        }

        if (ignoreSpent) 
            return total -= getSpentBlueCoinNum(pStorage);
        else
            return total;
    }

    s32 getTotalBlueCoinNumCurrentFile(bool ignoreSpent) {
        BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();
        return getTotalBlueCoinNum(pStorage, ignoreSpent);
    }

    s32 getBlueCoinRange(const char* pStageName, bool minOrMax) {
        JMapInfo table = JMapInfo();
        table.attach(gBlueCoinIDRangeTable);

        const char* tableStageName;
        s32 targetLine = -1;

        if (!pStageName)
            pStageName = MR::getCurrentStageName();

        for (s32 i = 0; i < MR::getCsvDataElementNum(&table); i++) {
            MR::getCsvDataStr(&tableStageName, &table, "StageName", i);

            if (MR::isEqualString(pStageName, tableStageName)) {
                targetLine = i;
                break;
            }
        }

        if (targetLine > -1) {
            s32 val = 0;
            MR::getCsvDataS32(&val, &table, minOrMax ? "BlueCoinRangeMax" : "BlueCoinRangeMin", targetLine);
            return val;
        }
        return -1;
    }

    s32 calcBlueCoinTotalInRange(const char* pStageName, bool collectedCoinsOnly) {
        JMapInfo table = JMapInfo();
        table.attach(gBlueCoinIDRangeTable);

        const char* tableStageName;
        s32 targetLine = -1;

        if (!pStageName)
            pStageName = MR::getCurrentStageName();

        for (s32 i = 0; i < MR::getCsvDataElementNum(&table); i++) {
            MR::getCsvDataStr(&tableStageName, &table, "StageName", i);

            if (MR::isEqualString(pStageName, tableStageName)) {
                targetLine = i;
                break;
            }
        }

        if (targetLine > -1) {
            s32 rangeMin;
            s32 rangeMax;
            s32 count = 0;

            MR::getCsvDataS32(&rangeMin, &table, "BlueCoinRangeMin", targetLine);
            MR::getCsvDataS32(&rangeMax, &table, "BlueCoinRangeMax", targetLine);

            for (u32 i = rangeMin; i < rangeMax + 1; i++) {
                if (collectedCoinsOnly) {
                    if (isBlueCoinGotCurrentFile(i))
                        count++;
                }
                else
                    count++;
            }

            return count;
        }
        
        return -1;
    }

    JMapInfo* getBlueCoinIDRangeTable() {
        JMapInfo* table = new JMapInfo();
        table->attach(gBlueCoinIDRangeTable);
        return table;
    }

    CoinBase* tryCreateBlueCoinForSpawningActorActionKeeper(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 id) {
        if (id > -1) {
            BlueCoin* coin = new BlueCoin("BlueCoinS");
            MR::addToCoinHolder(pSourceActor, coin);
            coin->mID = id;
            coin->initWithoutIter();
            MR::hideModel(coin);
            MR::invalidateHitSensors(coin);
            ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
            pKeeper->mNewActor = coin;
            pKeeper->mItemGenerator = 0;
            return coin;
        }
        return false;
    }
    
    bool tryAppearBlueCoinActionKeeper(LiveActor* pSourceActor, const TVec3f& rPosition) {
        ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
        BlueCoin* pKeeperActor = (BlueCoin*)pKeeper->mNewActor;
        if (pKeeperActor) {
            TVec3f coinVelocity = TVec3f(0.0f, 25.0f, 0.0f);
            coinVelocity.scale(coinVelocity.y, -pSourceActor->mGravity);

            MR::startSystemSE("SE_SY_PURPLE_COIN_APPEAR", -1, -1);

            pKeeperActor->appearMove(rPosition, coinVelocity, 0x7FFFFFFF, -1);
            return true;
        }

        return false;
    }

    bool isValidBlueCoinActionKeeper(LiveActor* pSourceActor) {
        ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
        BlueCoin* pKeeperActor = (BlueCoin*)pKeeper->mNewActor;
        return (bool)pKeeperActor;
    }

    NameObj* createBlueCoin(const char* pName) {
        return new BlueCoin(pName);
    }

    bool checkStageBlueCoinCompletion(const char* pStageName) {
        u16 currentCoinNum = BlueCoinUtil::calcBlueCoinTotalInRange(pStageName, true);
        u16 maxCoinNum = BlueCoinUtil::calcBlueCoinTotalInRange(pStageName, false);
        return currentCoinNum == maxCoinNum;
    }

    void updateStageProgress(s32 id) {
        CounterLayoutControllerExt* pController = (CounterLayoutControllerExt*)MR::getGameSceneLayoutHolder()->mCounterLayoutController;
        BlueCoinStageProgress* pStageProgress = pController->mBlueCoinStageProgress;

        if (pStageProgress) {
            pStageProgress->setTargetIDAndAppear(id);
        }
    }

    //void resetAllBlueCoinOnDeleteFile(SaveDataHandleSequence* pSeq, UserFile* pFile, int fileID) {
    //pSeq->restoreUserFileConfigData(pFile, fileID);
    //
    //BlueCoinDataUtil::getBlueCoinDataStorage()->initializeData();
    //}
    //kmCall(0x804D9BF8, resetAllBlueCoinOnDeleteFile); // bl resetAllBlueCoinOnDeleteFile
}


