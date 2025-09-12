#include "CollectSwitchCtrl.h"

/*
* Authors: Aurum, Evanbowl
* Objects: CollectSwitchCtrl
* Parameters:
*  - Obj_arg0, list, 0: Item type
*   0: Coins
*   1: Purple Coins
*   2: Star Bits
*   3: Power Stars
*  - Obj_arg1, long, 0: Required amount
*  - Obj_arg2, list, 0: Comparator
*   0: amount == arg1
*   1: amount  < arg1
*   2: amount <= arg1
*   3: amount  > arg1
*   4: amount >= arg1
*   5: amount != arg1
*  - Obj_arg3, bool, false: Activate once
*  - Obj_arg4, bool, false: Clear Star Bits (requires Arg0 set to 2)
*  - SW_A, need, write: Set when conditions are met
* 
* Activates the SW_A event when the specified amount of the configured item has been collected. It can check a variety
* of items, such as Coins, Purple Coins, Star Bits and Power Stars.
*/

namespace pt {
	CollectSwitchCtrl::CollectSwitchCtrl(const char *pName) : NameObj(pName) {
		mStageSwitchCtrl = NULL;
		mItem = 0;
		mCount = 0;
		mComparator = 0;
		mSwitchOnce = false;
	}

	void CollectSwitchCtrl::init(const JMapInfoIter &rIter) {
		MR::connectToSceneMapObjMovement(this);

		// Get item information
		MR::getJMapInfoArg0NoInit(rIter, &mItem);
		MR::getJMapInfoArg1NoInit(rIter, &mCount);
		MR::getJMapInfoArg2NoInit(rIter, &mComparator);
		MR::getJMapInfoArg3NoInit(rIter, &mSwitchOnce);

		MR::registerDemoSimpleCastAll(this);

		// Try to clear Star Bits
		bool clearStarPiece = false;
		MR::getJMapInfoArg4NoInit(rIter, &clearStarPiece);

		if (clearStarPiece && mItem == 2) {
			MR::addStarPiece(-999);
			MR::forceSyncStarPieceCounter();
		}

		mStageSwitchCtrl = MR::createStageSwitchCtrl(this, rIter);
	}

	void CollectSwitchCtrl::movement() {
		// Fetch specified item count
		s32 count = 0;

		if (mItem == 0) {
			count = MR::getCoinNum();
		}
		else if (mItem == 1) {
			count = MR::getPurpleCoinNum();
		}
		else if (mItem == 2) {
			count = MR::getStarPieceNum();
		}
		else if (mItem == 3) {
			count = MR::getPowerStarNum();
		}
		
		#ifdef BLUECOINSYSTEM
		else if (mItem == 4) {
			count = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(false);
		}
		#endif

		// Compare actual count with expected count
		bool state = false;

		if (mComparator == 0) {
			state = count == mCount;
		}
		else if (mComparator == 1) {
			state = count < mCount;
		}
		else if (mComparator == 2) {
			state = count <= mCount;
		}
		else if (mComparator == 3) {
			state = count > mCount;
		}
		else if (mComparator == 4) {
			state = count >= mCount;
		}
		else if (mComparator == 5) {
			state = count != mCount;
		}

		// Set switch depending on result
		if (state) {
			mStageSwitchCtrl->onSwitchA();
		}
		else if (!mSwitchOnce) {
			mStageSwitchCtrl->offSwitchA();
		}
	}
}
