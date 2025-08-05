#include "RedCoinUtil.h"
#include "RedCoin.h"
#include "Game/LiveActor/ExtActorActionKeeper.h"

namespace RedCoinUtil {
    LiveActor* getSpecificActorFromGroup(LiveActor* pActor, const char* pName) {
		LiveActor* pObj = 0;
    	LiveActorGroup* group = MR::getGroupFromArray(pActor);
		
    	for (s32 i = 0; i < group->mNumObjs; i++) {
    	    if (MR::isEqualString(group->getActor(i)->mName, pName)) {
    	        pObj = group->getActor(i);
    	        break;
    	    }
    	}
		return pObj;
    }

	CoinBase* tryLinkToChildRedCoin(LiveActor* pSourceActor, const JMapInfoIter& rIter, s32 arg) {
		if (MR::getChildObjNum(rIter) == 1) {
			RedCoin* pCoin = (RedCoin*)NameObjFactory::initChildObj(rIter, 0);

			if (pCoin) {
				pCoin->requestHide();
        		((ExtActorActionKeeper*)pSourceActor->mActionKeeper)->mNewActor = pCoin;
				pSourceActor->mActionKeeper->mItemGenerator = 0;
       			return pCoin;
			}
		}

		return false;
	}

	bool tryAppearLinkedRedCoin(LiveActor* pSourceActor, const TVec3f& pPosition) {
		OSReport("Red Coin Spawning for %s\n", pSourceActor->mName);
		ExtActorActionKeeper* pKeeper = (ExtActorActionKeeper*)pSourceActor->mActionKeeper;
		RedCoin* pCoin = (RedCoin*)pKeeper->mNewActor;

		if (pCoin) {
			MR::setPosition(pCoin, pPosition);
			pCoin->appearAndMove(25.0f, "SE_SY_RED_COIN_APPEAR");

			return true;
		}

		return false;
	}

	NameObj* createRedCoin(const char* pName) {
		return new RedCoin(pName);
	}
}