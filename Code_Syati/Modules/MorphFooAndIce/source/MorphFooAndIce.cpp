#include "MorphFooAndIce.h"

/*
* Author: Aurum
* Objects: MorphItemNeoIce, MorphItemNeoFoo, MorphItemCollectionIce, MorphItemCollectionFoo
* Parameters: See other MorphItemNeo and MorphItemCollection objects
* 
* Two Power-ups from SMG1, the Ice Flower and Flying Star, did not make an appearance in SMG2 for unknown reasons.
* Luckily enough, all of their transformations still remain in the game. Only the models and the collectible Power-up
* items have to added back to the game. It appears that the developers attempted to delete some of the Flying Star's
* functionality since wall jumping and pulling coins does not work anymore.
* 
* Both Power-ups have unused effects for appearing, however. The Power-ups now use their respective effects as
* originally intended.
*/

namespace pt {
	/*
	* Ice Flower
	*/
	MorphItemNeoIce::MorphItemNeoIce(const char *pName) : MorphItemObjNeo(pName) {

	}

	u32 MorphItemNeoIce::getPowerUp() {
		return MORPH_ICE;
	}

	void MorphItemNeoIce::emitAppearEffects() {
		MR::emitEffect(this, "AppearIceFlower");
		MR::startActionSound(this, "OjMorphFlowerAppear", -1, -1, -1);
	}


	/*
	* Flying Star
	*/
	MorphItemNeoFoo::MorphItemNeoFoo(const char *pName) : MorphItemObjNeo(pName) {

	}

	u32 MorphItemNeoFoo::getPowerUp() {
		return MORPH_FOO;
	}

	void MorphItemNeoFoo::emitAppearEffects() {
		MR::emitEffect(this, "AppearFoo");
		MR::startActionSound(this, "OjMorphItemAppear", -1, -1, -1);
	}

	kmWrite32(0x80024104, 0x48000010); // set first collection flag
	kmWrite32(0x80024584, 0x60000000); // skip collection flag check


	/*
	* MorphItemCollection checks the object's name to detect the proper power-up type. This is done by initPowerUpModel.
	* We hijack calls to this function to check our custom object names first and apply the power-up types accordingly.
	* However, any other object name is redirected to be handled in the original function.
	*/
	void initCollectionPowerUp(MorphItemCollection *pActor, const JMapInfoIter &rIter) {
		if (MR::isObjectName(rIter, "MorphItemCollectionIce")) {
			pActor->mPowerUpType = MORPH_ICE;
		}
		else if (MR::isObjectName(rIter, "MorphItemCollectionFoo")) {
			pActor->mPowerUpType = MORPH_FOO;
		}
		else {
			pActor->initPowerUpModel(rIter);
		}
	}

	kmCall(0x802D1C3C, initCollectionPowerUp); // redirection hook
}
