#include "ModuleData_ExtGameData.h"
#include "ExtGameDataUtil.h"

#define BASE_CHUNK_COUNT 6

void extCreateChunks(ExtGameDataHolder *pHolder) {
    for (s32 i = 1; i < cExtSaveChunkCount; i++) {
        BinaryDataChunkBase *pChunk = cExtSaveChunkCreateTable[i]();

        if (pChunk) {
            // TODO Find a better way to do this
            u32 memberOffset = (u32)pHolder + sizeof(GameDataHolder) + 4 * (i - 1);
            *(u32 *)memberOffset = (u32)pChunk;

            pHolder->mBinaryDataChunkHolder->addChunk(pChunk);
        }
    }

    pHolder->resetAllData();
}

void extResetAllData(ExtGameDataHolder *pHolder) {
    ((BinaryDataChunkBase*)pHolder->mEventValueChecker)->initializeData();
    BinaryDataChunkHolder* pChunkHolder = pHolder->mBinaryDataChunkHolder;

    for (s32 i = BASE_CHUNK_COUNT; i < pChunkHolder->mNumChunks; i++) {
        pChunkHolder->mChunks[i]->initializeData();
    }
}

kmWrite16(0x804D483A, BASE_CHUNK_COUNT + cExtSaveChunkCount);
kmCall(0x804D488C, extCreateChunks);
kmWrite16(0x804DF00E, sizeof(ExtGameDataHolder));

kmWrite32(0x804D4B6C, 0x7FE3FB78); // mr r3, r31
kmCall(0x804D4B70, extResetAllData);
kmWrite32(0x804D4B74, 0x4800000C); // b 0xC