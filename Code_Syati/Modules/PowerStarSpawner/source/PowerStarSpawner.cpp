#include "PowerStarSpawner.h"
//#include "pt/Extras/ExtPowerStarColors.h"

/*
* This version of the PowerStarSpawner is exclusive to PT Debug.
*
* Created by Aurum, Evanbowl & Someone, with Group ID support thanks to Zyphro.
* 
* Spawns the specified Power Star if its SW_A is activated. The delay until the star spawns as well
* as the position from where the star spawns can be specified as well.
*
* This object is a bit overdeveloped but is one of my favorites.
*
	Obj_args:
	0: Power Star ID, int
	1: Spawn Type, int
	2: Spawn Delay, int
	3: Use Sound Effect, bool
	4: Use Mario's Position, bool
	5: Use Display Model, int
	6: Y Offset for spawning at the player, float, Default is 300.
*/

namespace pt {
	PowerStarSpawner::PowerStarSpawner(const char* pName) : LiveActor(pName) {
		mScenario = -1;
		mSpawnMode = -1;
		mDelay = 0;
		mUseSE = false;
		mFromMario = false;
		mElapsed = -1;
		mSpawnCondition = -1;
	}

	void PowerStarSpawner::init(JMapInfoIter const& rIter) {
		MR::initDefaultPos(this, rIter);

		MR::processInitFunction(this, "StarPiece", false);
		MR::hideModel(this); // A model is specified then hidden since it is not necessary, or else the ModelObj will crash the game.
		MR::connectToSceneMapObj(this);
		MR::invalidateClipping(this); // This object will never unload when offscreen.
		MR::calcGravity(this);

		MR::useStageSwitchReadA(this, rIter); // Reads SW_A.

		MR::getJMapInfoArg0NoInit(rIter, &mScenario); // Star ID
		MR::getJMapInfoArg1NoInit(rIter, &mSpawnMode); // Time Stop/Instant Appear/Squizzard Spawn
		MR::getJMapInfoArg2NoInit(rIter, &mDelay); // Delay before spawn.
		MR::getJMapInfoArg3NoInit(rIter, &mUseSE); // Play a sound effect?
		MR::getJMapInfoArg4NoInit(rIter, &mFromMario); // Should the Star start it's spawn path at Mario?
		MR::getJMapInfoArg5NoInit(rIter, &mSpawnCondition);

		initSound(1, "PowerStarSpawner", &mTranslation, 0);

		if (mScenario == -1)
			MR::declarePowerStar(this);
		else
			MR::declarePowerStar(this, mScenario);

		makeActorAppeared();
	}

	void PowerStarSpawner::movement() {
		if (MR::isOnSwitchA(this))
			spawnStar();
	}

	void PowerStarSpawner::spawnStar() {
		if (mFromMario) {
			MR::setPosition(this, *MR::getPlayerPos());
			JMAVECScaleAdd((Vec*)MarioAccess::getPlayerActor()->getGravityVec(), (Vec*)&mTranslation, (Vec*)&mTranslation, -250);
		}

		mElapsed++;

		if (mElapsed == 0 && mUseSE)
			MR::startActionSound(this, "OjPowerStarSpawnerSpawn", -1, -1, -1);

		if (spawnCondition() && mElapsed >= mDelay) {
			switch (mSpawnMode) { // I do not know how to improve this section.
				case -1:
				MR::appearEventPowerStar("PowerStarSpawner", mScenario, &mTranslation, 0, 0, 0);
				break;
				case 0:
				MR::appearEventPowerStar("PowerStarSpawner", mScenario, &mTranslation, 1, 0, 0);
				break;
				case 1:
				MR::appearEventPowerStar("PowerStarSpawner", mScenario, &mTranslation, 0, 1, 0);
				break;
				case 2:
				MR::appearEventPowerStar("PowerStarSpawner", mScenario, &mTranslation, 0, 0, 1);
				break;
				case 3:
				MR::appearEventPowerStar("PowerStarSpawner", mScenario, &mTranslation, 1, 0, 1);
				break;
			}
			makeActorDead();
		}
	}

	bool PowerStarSpawner::spawnCondition() {
		if (mSpawnCondition == 0)
			return !MR::isPlayerInRush();
		if (mSpawnCondition == 1)
			return MR::isOnGroundPlayer() || MR::isPlayerSwimming();
		if (mSpawnCondition == 2)
			return !MR::isPlayerInRush() && MR::isOnGroundPlayer();

		return true;
	}
}