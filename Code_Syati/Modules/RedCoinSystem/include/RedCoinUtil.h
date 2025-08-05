#pragma once

#include "syati.h"

class RedCoin;
class CoinBase;

namespace RedCoinUtil {
    LiveActor* getSpecificActorFromGroup(LiveActor* pActor, const char* pName);
    CoinBase* tryLinkToChildRedCoin(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 arg);
    bool tryAppearLinkedRedCoin(LiveActor* pSourceActor, const TVec3f& pPosition);
    NameObj* createRedCoin(const char* pName);
}

