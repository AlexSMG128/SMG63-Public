#include "BlueChip.h"
#include "Game/MapObj/Coin.h"

namespace pt {
	/*
	* New Clam items
	*
	* New variants can be easily added and created here. As usual, I redirected the calls to Shellfish::initItem to check
	* for my new variants first. At the end, the original function is called in order to initialize the other existing item
	* types and to set up the hit sensors and more.
	*/
	void initNewShellfishItem(Shellfish *pActor, const JMapInfoIter &rIter) {
		if (MR::isObjectName(rIter, "ShellfishBlueChip")) {
			s32 group = -1;
			MR::getJMapInfoArg0NoInit(rIter, &group);

			BlueChip *chip = new BlueChip("ブルーチップ");
			chip->setHost(pActor);
			chip->setGroupID(group);
			chip->initWithoutIter();

			pActor->mItem = chip;
			pActor->mItemType = 2;
		}
		else if (MR::isObjectName(rIter, "ShellfishPurpleCoin")) {
			LiveActor *coin = MR::createPurpleCoin(pActor, "パープルコイン");
			coin->initWithoutIter();

			pActor->mItem = coin;
			pActor->mItemType = 4;
		}

		pActor->initItem(rIter);
	}

	kmCall(0x802FC034, initNewShellfishItem); // redirect call to initItem in init
	kmWrite32(0x802FC79C, 0x41820030);        // skip the repetitive check for blue chips

	/*
	* Shellfish::putItem is used to set the item's position. The blue chip's offset is still part of Shellfish::putItem,
	* so we do not have to include that here.
	*/
	void putNewShellfishItem(Shellfish *pActor) {
		MtxPtr pBaseMtx = (MtxPtr)pActor->getBaseMtx();

		if (pActor->mItemType == 4) {
			PSMTXMultVec(pBaseMtx, TVec3f(0.0f, 50.0f, 30.0f), pActor->mItem->mTranslation);
		}
		else {
			pActor->putItem();
		}
	}

	kmCall(0x802FC22C, putNewShellfishItem); // redirect putItem in exeOpen
	kmCall(0x802FC838, putNewShellfishItem); // redirect putItem in initItem
	kmCall(0x802FCA58, putNewShellfishItem); // redirect putItem in endBindItem
}
