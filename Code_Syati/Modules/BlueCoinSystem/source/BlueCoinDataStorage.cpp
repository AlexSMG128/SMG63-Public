#include "BlueCoinDataStorage.h"
#include "ExtGameDataUtil.h"
#include "BlueCoinUtil.h"

BlueCoinDataStorage::BlueCoinDataStorage() : BinaryDataChunkBase() {
    mCollectionData = new MR::BitArray(COLLECTIONCOUNT);
    mFlags = new MR::BitArray(FLAGSCOUNT);
    mSpentData = 0;
    mHasSeenTextBox = false;
    mCollectionBytes = (mCollectionData->mFlagCount + 7 & ~7) / 8;
    mFlagsBytes = (mFlags->mFlagCount + 7 & ~7) / 8;
    mIsUpdateLocked = false;
};

BlueCoinDataStorage::~BlueCoinDataStorage() {

};

u32 BlueCoinDataStorage::makeHeaderHashCode() const {
    return MR::getHashCode("BlueCoinStorage");
};

u32 BlueCoinDataStorage::getSignature() const {
    return 'BCS1';
};

u32 BlueCoinDataStorage::serialize(u8* pPosition, u32 u) const {
    memcpy((u32*)pPosition, mCollectionData->mFlags, mCollectionBytes);
    memcpy((u32*)(pPosition+=mCollectionBytes), mFlags->mFlags, mFlagsBytes);
    *(u16*)(pPosition+=mFlagsBytes) = mSpentData;
    *(u8*)(pPosition+=2) = mHasSeenTextBox;
    *(u8*)(pPosition+=1) = mIsUpdateLocked;

    u32 size = mCollectionBytes+mFlagsBytes+4;
    return size;
};

u32 BlueCoinDataStorage::deserialize(const u8* pPosition, u32 size) {
    memcpy(mCollectionData->mFlags, pPosition, mCollectionBytes);
    memcpy(mFlags->mFlags, (pPosition+=mCollectionBytes), mFlagsBytes);
    mSpentData = *(u16*)(pPosition+=mFlagsBytes);
    mHasSeenTextBox = *(u8*)(pPosition+=2);
    mIsUpdateLocked = *(u8*)(pPosition+=1);
    return 0;
};

void BlueCoinDataStorage::initializeData() {
    MR::zeroMemory(mCollectionData->mFlags, mCollectionBytes);
    MR::zeroMemory(mFlags->mFlags, mFlagsBytes);
    mSpentData = 0;
    mHasSeenTextBox = 0;
};

namespace BlueCoinDataUtil {
    BlueCoinDataStorage* getBlueCoinDataStorage() {
        ExtGameDataHolder* pHolder = ExtGameDataUtil::getCurrentGameDataHolder();
        
        if (pHolder)
            return pHolder->mBlueCoinDataStorage;

        return 0;
    }
};

void performBlueCoinDataUpdate(SaveDataHandleSequence* pSeq, const Nerve* pNerve) {
    pSeq->setNerve(pNerve);

    BlueCoinDataStorage* pStorage = BlueCoinDataUtil::getBlueCoinDataStorage();

    if (pStorage->mIsUpdateLocked)
        return;

    NANDFileInfo info;
    s32 code = NANDOpen("BlueCoinData.bin", &info, NAND_MODE_READ);

    if (code != 0) {
        pStorage->mIsUpdateLocked = true;
        return;
    }

    u8* buf = new(0x20) u8[117];
    NANDRead(&info, buf, 117);
     
    u32 f = (pSeq->mCurrentFileNum-1);
    memcpy(pStorage->mCollectionData->mFlags, &buf[32*f], 32);
    memcpy(pStorage->mFlags->mFlags, &buf[96+(4*f)], 4);
    pStorage->mSpentData = *((u16*)buf+54+f);
    pStorage->mHasSeenTextBox = *((u8*)buf+114+f);
    
    pStorage->mIsUpdateLocked = true;
    delete[] buf;
    NANDClose(&info);
};

kmCall(0x804D9D30, performBlueCoinDataUpdate);

void makeNewFileUpdateLocked(UserFile* pFile) {
    pFile->resetAllData();
    BlueCoinDataStorage* pStorage = ((ExtGameDataHolder*)pFile->mGameDataHolder)->mBlueCoinDataStorage;
    pStorage->mIsUpdateLocked = true;
}

kmCall(0x804D9B80, makeNewFileUpdateLocked);