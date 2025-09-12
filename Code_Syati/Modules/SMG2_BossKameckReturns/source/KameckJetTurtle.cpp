#pragma once
#include "syati.h"

#include "Game/Enemy/KameckTurtle.h"

// This file fixes the shell magikoopas
// Originally from Aurum.
// No matter how hard I tried, I was unable to fix the ability to spin the beam to get the koopa shell early. So you will have to let it land first...

namespace {
	/*
	* Shell-casting Magikoopa
	*/
	void initNewKameck(Kameck* pActor, const JMapInfoIter& rIter) {
		pActor->initJMapParam(rIter);

		if (MR::isValidInfo(rIter)) {
			if (MR::isObjectName(rIter, "KameckJetTurtle")) {
				pActor->mBeamType = KAMECK_BEAM_TURTLE;
			}
		}
	}

	kmCall(0x801A49D4, initNewKameck);

	/*
	* While carrying over stuff from the first game, they forgot to update parts of the KameckTurtle actor. Therefore,
	* it will crash the game and cause various other problems. First of all, it tries to load the animation from SMG1,
	* which does not exist anymore (Koura.brk was renamed to Color.brk). Also, Mario won't try to pull in the shell
	* due to the shell's actor name being wrong. For some reason it expects a specific actor name...
	* Lastly, the actor should be dead by default, but they made it appear nevertheless.
	*/
	void initFixKameckTurtle(LiveActor* pActor) {
		pActor->mName = "カメックビーム用カメ";
		MR::startBrk(pActor, "Color");
	}

	kmCall(0x801A8CFC, initFixKameckTurtle); // redirect BRK assignment to initialization fix
	kmWrite32(0x801A8DD0, 0x818C0038);       // Call makeActorDead instead of makeActorAppeared


	bool fix_KameckTurtle_tryStorm(KameckTurtle* pTurtle)
	{
		return false;
	}

	kmBranch(0x801A91D0, fix_KameckTurtle_tryStorm);





	//// This code swaps the matter splatter collision detection so it works in haunty halls. In the future this will be properly implemented
	//bool TEMPORARY_FUNCTION(bool input) {
	//	return input;
	//}
	//kmWrite32(0x801A6B10, 0x60000000);
	//kmCall(0x801A6B14, TEMPORARY_FUNCTION);
}