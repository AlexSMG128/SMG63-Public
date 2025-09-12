#include "BlueChip.h"

/*
* Author: Aurum
* Objects: BlueChip, BlueChipGroup
* Parameters: See YellowChip and YellowChipGroup
* 
* The blue chips from SMG1 were cut and the developers even "tried" to delete them entirely. Most of the code is intact,
* but the SceneObjs and the layout for them had to be restored using some code.
*/

namespace pt {
	BlueChip::BlueChip(const char *pName) : ChipBase(pName, CHIP_BLUE, "BlueChip") {

	}

	BlueChipGroup::BlueChipGroup(const char *pName) : ChipGroup(pName, CHIP_BLUE) {

	}

	NameObj* createBlueChipHolder() {
		return new ChipHolder("BlueChipHolder", CHIP_BLUE);
	}

	ChipHolder* getBlueChipHolder() {
		return (ChipHolder*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_BLUE_CHIP_HOLDER);
	}


	/*
	* Patch hooks
	*/

	/*
	* There is one ChipHolder for every chip type, however, the blue one has been completely removed from SMG2. Whenever
	* a chip or chip group is initialized, the game tries to create a ChipHolder if it does not exist already.
	* Interestingly, the game performs a check to see if the chip type is blue, but it simply returns NULL in that case.
	*/
	void createChipHolder(s32 type) {
		if (type == CHIP_BLUE) {
			MR::createSceneObj(SCENE_OBJ_BLUE_CHIP_HOLDER);
		}
		else if (type == CHIP_YELLOW) {
			MR::createSceneObj(SCENE_OBJ_YELLOW_CHIP_HOLDER);
		}
	}

	kmBranch(0x80284E40, createChipHolder); // redirect MR::createChipHolder(s32)

	/*
	* MR::getChipHolder checks for the blue type, but never retrieves a ChipHolder for blue chips. In that case, NULL is
	* returned instead. However, we can simply insert a call to return our new blue chip holder here.
	*/
	kmCall(0x80284E80, getBlueChipHolder);

	/*
	* The function which loads the counter layouts misses the code for the blue chip counter as well. We can easily add
	* this back, but we have to copy some of the original code to make it work properly. This entire thing can be
	* heavily improved when written in pure assembly but I'm not sure how to accomplish this here...
	*/
	void initChipCounter(ChipCounter *pActor) {
		const char* layoutName;

		if (pActor->mType == CHIP_BLUE) {
			layoutName = "BlueChipCounter";
		}
		else if (pActor->mType == CHIP_YELLOW) {
			layoutName = "YellowChipCounter";
		}

		pActor->initLayoutManager(layoutName, 2);
	}

	kmWrite32(0x80283894, 0x7F63DB78);   // "mr r3, r27" so that r3 becomes "this->"
	kmCall(0x80283898, initChipCounter); // redirect hook
	kmWrite32(0x8028389C, 0x48000014);   // skip useless instructions
}
