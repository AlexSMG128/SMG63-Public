#include "BlueCoin.h"
#include "BlueCoinCounter.h"
#include "BlueCoinUtil.h"
#include "Game/Screen/GameSceneLayoutHolder.h"
#include "Game/Screen/CounterLayoutControllerExt.h"
#include "Game/NPC/TalkMessageCtrl.h"
#include "Game/MapObj/FileSelector.h"
#include "Game/LiveActor/ExtActorActionKeeper.h"
#include "BlueCoinStageProgress.h"

void* initializeBlueCoinArrayAndLoadTable() {
    BlueCoinUtil::initBlueCoinArray();
    return pt::loadArcAndFile("/SystemData/BlueCoinIDRangeTable.arc", "/BlueCoinIDRangeTable.bcsv", 0);
}

BlueCoinData* gBlueCoinData;
void* gBlueCoinIDRangeTable = initializeBlueCoinArrayAndLoadTable();
    
namespace BlueCoinUtil {
    void loadBlueCoinData() {
        OSReport("(BlueCoinUtil) Attempting BlueCoinData.bin read.\n");
        NANDFileInfo info;
        s32 code = NANDOpen("BlueCoinData.bin", &info, 3);
        
        if (code == -12) {
            OSReport("(BlueCoinUtil) BlueCoinData.bin not found. A new one will be created on game save.\n");
        }

        if (code == 0) {
            u32 size;
            NANDGetLength(&info, &size);

            if (size != BINSIZE) {
                NANDClose(&info);
                if (size == 873 && USEUPDATER) {
                    updateToNewFormat();
                }
                else if (MR::testCorePadButtonB(0)) {
                    saveBlueCoinData();
                }
                else {
                    char errstr[200];
                    snprintf(errstr, 200, "Blue Coin Read Error\nExpected size of %d\nGot size %d\n\n Max Coins: %d, Max Flags: %d\nNANDRead code: %d\nDelete or hex edit BlueCoinData.bin and try again.\n\nRestart, and hold B when this error appears to attempt an update.\n", BINSIZE, size, COLLECTIONCOUNT, FLAGSCOUNT, code);
                    GXColor bg = { 0, 0, 0, 0 };
                    GXColor fg = { 255, 255, 255, 255 };
                    OSFatal(fg, bg, errstr);
                }
            }
            else {
                u8* buffer = new(0x20) u8[BINSIZE];
                code = NANDRead(&info, buffer, BINSIZE);
                
                memcpy(gBlueCoinData->mCollectionData->mFlags, &buffer[0], getCollectionByteNum());
                memcpy(gBlueCoinData->mFlags->mFlags, &buffer[FLAGSLOC], getFlagsByteNum());
                
                for (int i = 0; i < 3; i++) {
                    memcpy(&gBlueCoinData->mSpentData[i], &buffer[SPENTLOC+(2*i)], 2);
                    gBlueCoinData->mHasSeenTextBox[i] = buffer[TEXTLOC+i];
                }
            
                delete [] buffer;
            }
            OSReport("(BlueCoinUtil) BlueCoinData.bin successfully read.\n");
            printBlueCoinSaveFileInfo();
        }
        NANDClose(&info);
    }
    
    void saveBlueCoinData() {
        s32 code = NANDCreate("BlueCoinData.bin", 0x30, 0);
        if (code == 0 || code == -6) {
            NANDFileInfo info;
            code = NANDOpen("BlueCoinData.bin", &info, 3);
            if (code == 0) {
                NANDSeek(&info, 0, 0);
                u8* buffer = new(0x20) u8[BINSIZE];
                memcpy(&buffer[0], gBlueCoinData->mCollectionData->mFlags, getCollectionByteNum());
                memcpy(&buffer[FLAGSLOC], gBlueCoinData->mFlags->mFlags, getFlagsByteNum());
                
                for (int i = 0; i < 3; i++) {
                    memcpy(&buffer[SPENTLOC+(2*i)], &gBlueCoinData->mSpentData[i], 2);
                    buffer[TEXTLOC+i] = gBlueCoinData->mHasSeenTextBox[i];
                }
                
                code = NANDWrite(&info, buffer, BINSIZE);

                if (code != BINSIZE) {
                    char errstr[180];
                    snprintf(errstr, 180, "Blue Coin Write Error\nExpected size of %d\nNANDWrite code: %d\n", BINSIZE, code);
		            GXColor bg = { 0, 0, 0, 0 };
                    GXColor fg = { 255, 255, 255, 255 };
                    OSFatal(fg, bg, errstr);
                }

                delete [] buffer;
                OSReport("(BlueCoinUtil) BlueCoinData.bin successfully saved.\n");
                printBlueCoinSaveFileInfo();
            }
            NANDClose(&info);
        }
    }

    #ifdef USEUPDATER
    void updateToNewFormat() {
        OSReport("Attempting BlueCoinData.bin update\n");

        NANDFileInfo info;
        u8* buffer = new(0x20) u8[873];
        s32 code = NANDOpen("BlueCoinData.bin", &info, 3);
        code = NANDRead(&info, buffer, 873);
        
        for (int i = 0; i < 768; i++) {
            gBlueCoinData->mCollectionData->set(i, buffer[i]);
        }

        for (int i = 0; i < 96; i++) {
            gBlueCoinData->mFlags->set(i, buffer[768+i]);
        }

        for (int i = 0; i < 3; i++) {
            memcpy(&gBlueCoinData->mSpentData[i], &buffer[864+(2*i)], 2);
            gBlueCoinData->mHasSeenTextBox[i] = buffer[870+i];
        }

        NANDClose(&info);
        NANDDelete("BlueCoinData.bin");
        delete [] buffer;
        OSReport("BlueCoinData.bin has been updated to the default sizing. Saving...\n");
        saveBlueCoinData();
    }
    #endif

    int getCollectionByteNum() {
        return (gBlueCoinData->mCollectionData->mFlagCount + 7 & ~7) / 8;
    }

    int getFlagsByteNum() {
        return (gBlueCoinData->mFlags->mFlagCount + 7 & ~7) / 8;
    }

    void printBlueCoinSaveFileInfo() {
        const int flagsStrSize = FLAGSCOUNT+(FLAGSCOUNT/8);
        char flagstr[3][flagsStrSize];
        s32 numcoins[3];

        for (s32 i = 0; i < 3; i++) {
            flagstr[i][flagsStrSize-1] = 0;
            numcoins[i] = 0;
            s32 flagidx = 0;
            

            for (s32 j = 0; j < flagsStrSize-1; j++) {
                if (j % 9 == 8)
                    flagstr[i][j] = 0x20;
                else {
                    flagstr[i][j] = gBlueCoinData->mFlags->isOn((FLAGSCOUNT*i)+flagidx) ? 0x31 : 0x30;
                    flagidx++;
                }
            }

            for (s32 j = 0; j < COLLECTIONCOUNT; j++) {
                if (gBlueCoinData->mCollectionData->isOn((COLLECTIONCOUNT*i)+j))
                    numcoins[i]++;
            }
        }

        OSReport("Blue Coin save file info\nc0: %d, c1: %d, c2: %d\nf0: %s\nf1: %s\nf2: %s\ns0: %d, s1: %d, s2: %d\nm0: %s, m1: %s, m2: %s\nMax Coin Count: %d, Max Flag Count: %d\n", 
        numcoins[0], 
        numcoins[1], 
        numcoins[2],
        flagstr[0],
        flagstr[1],
        flagstr[2],
        gBlueCoinData->mSpentData[0],
        gBlueCoinData->mSpentData[1],
        gBlueCoinData->mSpentData[2],
        gBlueCoinData->mHasSeenTextBox[0] ? "Yes" : "No",
        gBlueCoinData->mHasSeenTextBox[1] ? "Yes" : "No",
        gBlueCoinData->mHasSeenTextBox[2] ? "Yes" : "No",
        COLLECTIONCOUNT,
        FLAGSCOUNT
        );
    }

   void clearBlueCoinData() {
       for (int i = 0; i < 3; i++) {
           MR::zeroMemory(gBlueCoinData->mCollectionData->mFlags, getCollectionByteNum());
           MR::zeroMemory(gBlueCoinData->mFlags->mFlags, getFlagsByteNum());
           gBlueCoinData->mSpentData[i] = 0;
           gBlueCoinData->mHasSeenTextBox[i] = 0;
       }
       
       OSReport("(BlueCoinUtil) Blue Coin array cleared.\n");
   }

    void initBlueCoinArray() {
        gBlueCoinData = new BlueCoinData;
        gBlueCoinData->mCollectionData = new MR::BitArray(COLLECTIONCOUNT*3);
        gBlueCoinData->mFlags = new MR::BitArray(FLAGSCOUNT*3);
        for (int i = 0; i < 3; i++) {
            gBlueCoinData->mSpentData[i] = 0;
            gBlueCoinData->mHasSeenTextBox[i] = 0;
        }
        OSReport("(BlueCoinUtil) Blue Coin array initialized.\n");
    }

    s32 getCurrentFileNum() {
        return GameDataFunction::getSaveDataHandleSequence()->mCurrentFileNum-1;
    }

    void setBlueCoinGotCurrentFile(u16 id) {
        gBlueCoinData->mCollectionData->set((COLLECTIONCOUNT*getCurrentFileNum())+id, true);
    }

    bool isBlueCoinGot(u8 file, u16 id) {
        return gBlueCoinData->mCollectionData->isOn((COLLECTIONCOUNT*file)+id);
    }
    
    bool isBlueCoinGotCurrentFile(u16 id) {
        return gBlueCoinData->mCollectionData->isOn((COLLECTIONCOUNT*getCurrentFileNum())+id);
    }

    bool hasSeenBlueCoinTextBoxCurrentFile() {
        return gBlueCoinData->mHasSeenTextBox[getCurrentFileNum()];
    }  

    void setSeenBlueCoinTextBoxCurrentFile() {
        gBlueCoinData->mHasSeenTextBox[getCurrentFileNum()] = true;
    }

    void setOnBlueCoinFlagCurrentFile(u8 flag) {
        gBlueCoinData->mFlags->set((FLAGSCOUNT*getCurrentFileNum())+flag, true);
    }

    bool isOnBlueCoinFlagCurrentFile(u8 flag) {
        return gBlueCoinData->mFlags->isOn((FLAGSCOUNT*getCurrentFileNum())+flag);
    }

    void resetAllBlueCoinTargetFile(u8 file) {
        int collectionCount = gBlueCoinData->mCollectionData->mFlagCount;
        MR::zeroMemory(&gBlueCoinData->mCollectionData->mFlags[FLAGSCOUNT*file], getCollectionByteNum()/3);
        MR::zeroMemory(&gBlueCoinData->mFlags->mFlags[FLAGSCOUNT*file], getFlagsByteNum()/3);
        gBlueCoinData->mSpentData[file] = 0;
        gBlueCoinData->mHasSeenTextBox[file] = 0;
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
        numcoin == 0 ? 30 : numcoin;
        
        if (getTotalBlueCoinNumCurrentFile(true) >= numcoin)
            gBlueCoinData->mSpentData[getCurrentFileNum()] += numcoin;
    }

    s32 getSpentBlueCoinNum(u8 file) {
        return gBlueCoinData->mSpentData[file];
    }

    s32 getSpentBlueCoinNumCurrentFile() {
        return gBlueCoinData->mSpentData[getCurrentFileNum()];
    }

    s32 getTotalBlueCoinNum(u8 file, bool ignoreSpent) {
        s32 total = 0;
        for (s32 i = 0; i < COLLECTIONCOUNT; i++) {
            if (gBlueCoinData->mCollectionData->isOn((COLLECTIONCOUNT*file)+i))
                total++;
        }

        if (ignoreSpent) 
            return total -= getSpentBlueCoinNum(file);
        else
            return total;
    }

    s32 getTotalBlueCoinNumCurrentFile(bool ignoreSpent) {
        return getTotalBlueCoinNum(getCurrentFileNum(), ignoreSpent);
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
        OSReport("Blue Coin Spawning for %s\n", pSourceActor->mName);
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
}

void resetAllBlueCoinOnDeleteFile(SaveDataHandleSequence* pSeq, UserFile* pFile, int fileID) {
    pSeq->restoreUserFileConfigData(pFile, fileID);
    BlueCoinUtil::resetAllBlueCoinTargetFile(fileID-1);
}

kmCall(0x804D9BF8, resetAllBlueCoinOnDeleteFile); // bl resetAllBlueCoinOnDeleteFile

void saveBlueCoinDataOnGameSave(const char* pName) {
    MR::startSystemSE(pName, -1, -1);
    BlueCoinUtil::saveBlueCoinData();
}

#if defined TWN || defined KOR
    kmCall(0x804DB060, saveBlueCoinDataOnGameSave); // bl saveBlueCoinDataOnGameSave
#else
    kmCall(0x804DAFD0, saveBlueCoinDataOnGameSave); // bl saveBlueCoinDataOnGameSave
#endif

void onTitleScreenLoad(FileSelector* pFileSelector) {
    pFileSelector->initHitSensor(1);
    BlueCoinUtil::clearBlueCoinData();
    BlueCoinUtil::loadBlueCoinData();
}

kmCall(0x8024F358, onTitleScreenLoad); // bl saveBlueCoinDataOnGameSave