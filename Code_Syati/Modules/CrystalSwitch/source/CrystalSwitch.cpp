#include "CrystalSwitch.h"

/*
* Authors: Evanbowl, Someone, Aurum
* Objects: CrystalSwitch
* Parameters:
*  - Obj_arg0, long, -1: Activation time
*  - SW_A, need, write: Activated when triggered
*  - SW_AWAKE
* 
* CrystalSwitch is one of SMG1's many unused switch controllers. It features a unique crystal design not seen anywhere
* else in the game. When triggered through any of the player's attacks, the SW_A event will be activated and the crystal
* starts spinning. There is unused code in SMG1 for slowing down and deactivating the event again. This unfinished
* behavior has been restored here and can be utilized with Obj_arg0.
*/

namespace pt {
	CrystalSwitch::CrystalSwitch(const char *pName) : LiveActor(pName) {
		mActiveTime = -1;
		mRotSpeed = 0.0f;
		mStartSpin = false;
	}

	void CrystalSwitch::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, false);

		// Set initial animations
		MR::startAction(this, "Wait");
		MR::startBpk(this, "Off");

		// Get activation time
		MR::getJMapInfoArg0NoInit(rIter, &mActiveTime);

		// Setup nerve and make appeared
		initNerve(&NrvCrystalSwitch::NrvOff::sInstance, 0);
		makeActorAppeared();
	}

	void CrystalSwitch::control() {
		calcRotSpeed();
		mStartSpin = false;
		MR::isStarPointerPointing2PAttachTarget(this);
	}

	void CrystalSwitch::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		MR::sendMsgPush(pReceiver, pSender);
	}

	bool CrystalSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		bool hitAll = MR::isMsgPlayerHitAll(msg);

		if (hitAll || MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg) || MR::isMsgStarPieceReflect(msg)) {
			mStartSpin = true;
			return !hitAll;
		}

		return false;
	}

	void CrystalSwitch::exeOff() {
		if (MR::isFirstStep(this)) {
			MR::validateClipping(this);
			MR::offSwitchA(this);
		}

		if (mStartSpin) {
			setNerve(&NrvCrystalSwitch::NrvSwitchDown::sInstance);
		}
	}

	void CrystalSwitch::exeSwitchDown() {
		if (MR::isFirstStep(this)) {
			MR::startBpk(this, "On");
			MR::invalidateClipping(this);
		}

		if (MR::isGreaterStep(this, 10)) {
			setNerve(&NrvCrystalSwitch::NrvOn::sInstance);
		}
	}

	void CrystalSwitch::exeOn() {
		if (MR::isFirstStep(this)) {
			MR::onSwitchA(this);
			MR::shakeCameraNormal();
			MR::startActionSound(this, "OjCrystalSwitchOn", -1, -1, -1);
			mRotSpeed = 0.0f;
		}

		// This behavior is not implemented in SMG1
		if (mActiveTime > -1 && MR::isStep(this, mActiveTime)) {
			setNerve(&NrvCrystalSwitch::NrvSwitchUp::sInstance);
		}
	}

	// Never called in SMG1, making this behavior effectively unused in that game
	void CrystalSwitch::exeSwitchUp() {
		if (MR::isFirstStep(this)) {
			MR::startBpk(this, "Off");
		}

		if (mRotSpeed >= 8.0f) {
			setNerve(&NrvCrystalSwitch::NrvOff::sInstance);
		}
	}

	void CrystalSwitch::calcRotSpeed() {
		// Switch gets activated
		if (isNerve(&NrvCrystalSwitch::NrvOn::sInstance)) {
			if (mRotSpeed < 10.0f) {
				mRotSpeed += 1.5f;
			}

			if (mRotSpeed > 10.0f) {
				mRotSpeed = 10.0f;
			}

			MR::setBckRate(this, mRotSpeed);
		}
		// Switch gets deactivated
		else {
			if (mRotSpeed > 10.0f) {
				mRotSpeed = 10.0f;
			}

			MR::setBckRate(this, mRotSpeed);

			mRotSpeed *= 0.99f;

			if (mRotSpeed < 0.1f) {
				mRotSpeed = 0.0f;
			}
		}
	}

	namespace NrvCrystalSwitch {
		void NrvOff::execute(Spine *pSpine) const {
			CrystalSwitch *pActor = (CrystalSwitch*)pSpine->mExecutor;
			pActor->exeOff();
		}

		void NrvSwitchDown::execute(Spine *pSpine) const {
			CrystalSwitch *pActor = (CrystalSwitch*)pSpine->mExecutor;
			pActor->exeSwitchDown();
		}

		void NrvOn::execute(Spine *pSpine) const {
			CrystalSwitch *pActor = (CrystalSwitch*)pSpine->mExecutor;
			pActor->exeOn();
		}

		void NrvSwitchUp::execute(Spine *pSpine) const {
			CrystalSwitch *pActor = (CrystalSwitch*)pSpine->mExecutor;
			pActor->exeSwitchUp();
		}

		NrvOff(NrvOff::sInstance);
		NrvSwitchDown(NrvSwitchDown::sInstance);
		NrvOn(NrvOn::sInstance);
		NrvSwitchUp(NrvSwitchUp::sInstance);
	}
}
