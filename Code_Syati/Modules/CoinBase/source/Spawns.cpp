#include "syati.h"
#include "Game/Enemy/Kuribo.h"
#include "Game/Enemy/SamboHead.h"
#include "Game/Enemy/Teresa.h"
#include "Game/LiveActor/ExtActorActionKeeper.h"
#include "ModuleData_Spawns.h"

// KURIBO

void KuriboSetUpCustomCoin(Kuribo* pKuribo, const JMapInfoIter& rIter, const char* pStr) {
    s32 coinType = -1;
    s32 coinArg = 0;
    MR::processInitFunction(pKuribo, rIter, pStr, 0);
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg2NoInit(rIter, &coinArg);
        MR::getJMapInfoArg3NoInit(rIter, &coinType);
    }
    CoinBaseSpawnUtil::createCustomCoin(pKuribo, rIter, coinArg, coinType);
}

kmCall(0x801B8A68, KuriboSetUpCustomCoin);

void KuriboAppearCustomCoin(Kuribo* pKuribo) {
    pKuribo->LiveActor::makeActorDead();
    
    CoinBaseSpawnUtil::appearCustomCoin(pKuribo, pKuribo->mTranslation);
}

kmBranch(0x801B8E70, KuriboAppearCustomCoin);

// SAMBOHEAD


void SamboHeadSetUpCustomCoin(SamboHead* pSamboHead, const JMapInfoIter& rIter, const char* pStr) {
    MR::processInitFunction(pSamboHead, rIter, pStr, 0);

    s32 coinType = -1;
    s32 coinArg = 0;
    MR::getJMapInfoArg1NoInit(rIter, &coinArg);
    MR::getJMapInfoArg2NoInit(rIter, &coinType);
    CoinBaseSpawnUtil::createCustomCoin(pSamboHead, rIter, coinArg, coinType);
}

kmCall(0x801F8024, SamboHeadSetUpCustomCoin);

void SamboHeadAppearCustomCoin(SamboHead* pSamboHead, const char* pStr) {
    MR::emitEffect(pSamboHead, pStr);

    CoinBaseSpawnUtil::appearCustomCoin(pSamboHead, pSamboHead->mTranslation);
}

kmCall(0x801F81D4, SamboHeadAppearCustomCoin);

// TERESA


s32 TeresaSetUpCustomCoin(const JMapInfoIter& rIter, Teresa* pTeresa) {
    s32 modelId = MR::getDummyDisplayModelId(rIter, -1);

    s32 coinType = -1;
    s32 coinArg = 0;

    MR::getJMapInfoArg5NoInit(rIter, &coinType);
    MR::getJMapInfoArg6NoInit(rIter, &coinArg);

    bool coinCreated = CoinBaseSpawnUtil::createCustomCoin(pTeresa, rIter, coinArg, coinType);

    if (coinCreated)
        pTeresa->_10C = 0;
    
    return modelId;
}

kmWrite32(0x8020C15C, 0x7F84E378); // mr r4, r28
kmCall(0x8020C160, TeresaSetUpCustomCoin);

void TeresaAppearCustomCoin(Teresa* pTeresa) {
    if (pTeresa->_10C)
        MR::appearCoinPop(pTeresa, pTeresa->mTranslation, 1);

    CoinBaseSpawnUtil::appearCustomCoin(pTeresa, pTeresa->mTranslation);
}

kmWrite32(0x8020C394, 0x7FE3FB78); // mr r3, r31
kmCall(0x8020C398, TeresaAppearCustomCoin);
kmWrite32(0x8020C39C, 0x48000014); // b 0x14

namespace CoinBaseSpawnUtil {
    CoinBase* createCustomCoin(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 arg, s32 type) {
    ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
        if (type > -1) {
            OSReport("Creating coin type %d for enemy %s with arg %d\n", type, pSourceActor->mName, arg);
            CoinCreateInfo* pInfo = &cCoinCreateInfo[1];

            for (s32 i = 1; i < cNumNewCoinCreateInfo; i++) {
                if (type == pInfo->type) {
                    pKeeper->mType = pInfo->type;
                    return (pInfo->mCreateFunc)(pSourceActor, rIter, arg);
                    break;
                }
                else
                    pInfo = &cCoinCreateInfo[i+1];
            }
        }

        pKeeper->mType = -1;
        pKeeper->mNewActor = 0;
        return 0;
    }
    
    bool appearCustomCoin(LiveActor* pSourceActor, const TVec3f& rPos) {
        ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
        if (pKeeper->mNewActor) {
            s32 type = pKeeper->mType;

            for (s32 i = 1; i < cNumNewCoinCreateInfo; i++) {
                if (type == cCoinAppearInfo[i].type) {
                    CoinAppearInfo* pInfo = &cCoinAppearInfo[i];
                    ((ExtActorActionKeeper*)pSourceActor->mActionKeeper)->mType = pInfo->type; 
                    return (pInfo->mAppearFunc)(pSourceActor, rPos);
                    break;
                }
            }
        }
        return false;
    }
}