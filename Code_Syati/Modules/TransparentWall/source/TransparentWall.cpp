#include "TransparentWall.h"
#include "Game/Util.h"

/*
* Author: Aurum
* Objects: TransparentWall class
* Parameters:
*  - Obj_arg4, int, -1: Validity Bits
*   0: No power-up
*   1: Rainbow power-up
*   2: Fire power-up
*   3: Ice power-up
*   4: Bee power-up
*   5: Spring power-up
*   6: Boo power-up
*   7: Flying power-up
*   8: "Bone" mode
*   9: Tornado mode
*   10: Riding Yoshi
*   11: "Horror" mode
*   12: Cloud power-up
*   13: Rock power-up
*   14: Player is skating
*   15: Boo Mario not disappeared
*  - SW_APPEAR
*/

namespace pt {
    extern bool isPlayerElementMode(s32 mode);
    extern bool isPlayerSkating();
	
	TransparentWall::TransparentWall(const char *pName) : InvisiblePolygonObj(pName) {
		mAllowFlags = -1;
		mValidated = false;
	}

	void TransparentWall::init(const JMapInfoIter &rIter) {
		InvisiblePolygonObj::init(rIter);
		MR::getJMapInfoArg4NoInit(rIter, &mAllowFlags);
		MR::invalidateCollisionParts(this);
		MR::setSensorType(this, "Body", ATYPE_ICEJUMP_WALL);
	}

	void TransparentWall::control() {
		if (!mValidated) {
			for (s32 i = 0; i < TRANSPARENT_WALL_FLAG_EXTRA_START; i++) {
				if (checkAllowed(i) && pt::isPlayerElementMode(i)) {
					mValidated = true;
					break;
				}
			}

			if (checkAllowed(TRANSPARENT_WALL_CHECK_PLAYER_SKATING) && pt::isPlayerSkating()) {
				mValidated = true;
			}

			if (checkAllowed(TRANSPARENT_WALL_CHECK_PLAYER_TERESA_DISAPPEAR) && !MR::isPlayerTeresaDisappear()) {
				mValidated = true;
			}

			if (mValidated) {
				MR::validateCollisionParts(this);
			}
		}
		else {
			bool invalidate = true;

			for (s32 i = 0; i < TRANSPARENT_WALL_FLAG_EXTRA_START; i++) {
				if (checkAllowed(i) && pt::isPlayerElementMode(i)) {
					invalidate = false;
					break;
				}
			}

			if (checkAllowed(TRANSPARENT_WALL_CHECK_PLAYER_SKATING)) {
				invalidate = !pt::isPlayerSkating();
			}

			if (checkAllowed(TRANSPARENT_WALL_CHECK_PLAYER_TERESA_DISAPPEAR)) {
				invalidate = MR::isPlayerTeresaDisappear();
			}

			if (invalidate) {
				MR::invalidateCollisionParts(this);
				mValidated = false;
			}
		}
	}
}
