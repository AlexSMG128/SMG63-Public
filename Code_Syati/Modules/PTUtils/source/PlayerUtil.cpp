#include "PlayerUtil.h"
#include "Game/Player/MarioAccess.h"
#include "Game/Player/MarioHolder.h"
#include "Game/Player/MarioState.h"

/*
* Author: Aurum
*/

namespace pt {
	bool isPlayerElementMode(s32 mode) {
		return MarioAccess::getPlayerActor()->mPlayerMode == mode;
	}

	bool isPlayerSkating() {
		return MarioAccess::getPlayerActor()->mMario->isStatusActive(MARIO_SKATE);
	}
}
