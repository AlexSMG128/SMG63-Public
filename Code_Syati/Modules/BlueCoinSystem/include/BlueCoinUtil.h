#pragma once
#include "syati.h"
class BlueCoin;
class CounterLayoutController;
class BlueCoinCounter;

class CoinBase;

// These can be customized but save files will not carry over.
// Modifying these will disable the updater.
#define COLLECTIONCOUNT 256
#define FLAGSCOUNT 32


#define FLAGSLOC (COLLECTIONCOUNT*3)/8
#define SPENTLOC FLAGSLOC+(FLAGSCOUNT*3)/8
#define TEXTLOC SPENTLOC+6

#define BINSIZE (((COLLECTIONCOUNT+FLAGSCOUNT)/8)*3+9)

#define USEUPDATER (COLLECTIONCOUNT == 256 && FLAGSCOUNT == 32)

namespace pt {
    extern void* loadArcAndFile(const char *pArc, const char *pFile, JKRHeap *pHeap);
    extern void setTextBoxArgStringNumberFontRecursive(LayoutActor* pLayout, const char* paneName, u16 num, s32 index);
}

struct BlueCoinData {
    MR::BitArray* mCollectionData;
    MR::BitArray* mFlags;
    u16 mSpentData[3];
    bool mHasSeenTextBox[3];
};

extern BlueCoinData* gBlueCoinData;

namespace BlueCoinUtil {
    void loadBlueCoinData();
    void saveBlueCoinData();
    void updateToNewFormat();

    int getCollectionByteNum();
    int getFlagsByteNum();

    void printBlueCoinSaveFileInfo();

    void clearBlueCoinData();
    void initBlueCoinArray();

    s32 getCurrentFileNum();

    void setBlueCoinGotCurrentFile(u16 id);

    bool isBlueCoinGot(u8 file, u16 id);
    bool isBlueCoinGotCurrentFile(u16 id);
   
    void setOnBlueCoinFlagCurrentFile(u8 flag);
    bool isOnBlueCoinFlagCurrentFile(u8 flag);

    bool hasSeenBlueCoinTextBoxCurrentFile();
    void setSeenBlueCoinTextBoxCurrentFile();
    bool isBlueCoinTextBoxAppeared();

    void resetAllBlueCoinTargetFile(u8 file);

    void setCounter();
    void showTextBox();  

    void spendBlueCoinCurrentFile(u16 numcoin);
    s32 getSpentBlueCoinNum(u8 file);
    s32 getSpentBlueCoinNumCurrentFile();

    s32 getTotalBlueCoinNum(u8 file, bool ignoreSpent);
    s32 getTotalBlueCoinNumCurrentFile(bool ignoreSpent);
    
    s32 getBlueCoinRange(const char* pStageName, bool minOrMax);
    s32 calcBlueCoinTotalInRange(const char* pStageName, bool collectedCoinsOnly);

    JMapInfo* getBlueCoinIDRangeTable();

    CoinBase* tryCreateBlueCoinForSpawningActorActionKeeper(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 id);
    bool tryAppearBlueCoinActionKeeper(LiveActor* pSourceActor, const TVec3f& rPosition);
    bool isValidBlueCoinActionKeeper(LiveActor* pSourceActor);

    NameObj* createBlueCoin(const char* pName);

    bool checkStageBlueCoinCompletion(const char* pStageName);
    void updateStageProgress(s32 id);
};
