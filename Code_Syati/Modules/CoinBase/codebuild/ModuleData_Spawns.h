#pragma once

#include "syati.h"
#include "CoinBase.h"
#include "BlueCoinUtil.h"
#include "RedCoinUtil.h"


struct CoinCreateInfo {
    CoinBase* (*mCreateFunc)(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 arg);
    s32 type;
};

struct CoinAppearInfo {
    bool (*mAppearFunc)(LiveActor* pSourceActor, const TVec3f &rPos);
    s32 type;
};

CoinCreateInfo cCoinCreateInfo[] = {
    {NULL, -1},
    {BlueCoinUtil::tryCreateBlueCoinForSpawningActorActionKeeper, 1},
    {RedCoinUtil::tryLinkToChildRedCoin, 0},

};

const size_t cNumNewCoinCreateInfo = sizeof(cCoinCreateInfo)/sizeof(cCoinCreateInfo[0]);

CoinAppearInfo cCoinAppearInfo[] = {
    { NULL, -1 },
    {BlueCoinUtil::tryAppearBlueCoinActionKeeper, 1},
    {RedCoinUtil::tryAppearLinkedRedCoin, 0},

};

namespace CoinBaseSpawnUtil {
    CoinBase* createCustomCoin(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 arg, s32 type);
    bool appearCustomCoin(LiveActor* pSourceActor, const TVec3f& rPos);
};