#pragma once

#include "syati.h"


class BlueCoinDataStorage : public BinaryDataChunkBase {
    public:
    
    BlueCoinDataStorage();
    virtual ~BlueCoinDataStorage();
    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual u32 serialize(u8* pPosition, u32 u) const;
    virtual u32 deserialize(const u8* pPosition, u32 size);
    virtual void initializeData();

    MR::BitArray* mCollectionData;
    MR::BitArray* mFlags;
    u16 mSpentData;
    bool mHasSeenTextBox;
    u32 mCollectionBytes;
    u32 mFlagsBytes;
    bool mIsUpdateLocked;
};

namespace BlueCoinDataUtil {
    BlueCoinDataStorage* getBlueCoinDataStorage();
};