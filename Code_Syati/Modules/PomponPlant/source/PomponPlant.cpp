#include "PomponPlant.h"

/*
* Authors: Someone, Aurum
* Objects: Pompon2Plant
* Parameters:
*  - Obj_arg0, list, -1: Item type
*   *: nothing
*   0: Coins
*   1: Star Bits
*  - Obj_arg1, long, 1, Item amount
*  - SW_AWAKE
* 
* This is the bubbly plant from Honeyhive Galaxy. When touched, it can spawn Coins or Star Bits. It is compatible with
* the Support Play Luma as well.
*/

namespace pt {
	PomponPlant::PomponPlant(const char *pName) : LiveActor(pName) {
		mCollectible = -1;
		mAmount = 1;
		mItemGenerated = false;
	}

	void PomponPlant::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, false);

		// Initialize items
		MR::getJMapInfoArg0NoInit(rIter, &mCollectible);
		MR::getJMapInfoArg1NoInit(rIter, &mAmount);

		if (mCollectible == -1) {
			MR::declareCoin(this, mAmount);
		}
		else if (mCollectible ==0) {
			MR::declareStarPiece(this, mAmount);
		}

		// Setup nerve and make appeared
		initNerve(&NrvPomponPlant::NrvWait::sInstance, 0);
		makeActorAppeared();
	}

	void PomponPlant::control() {
		MR::attachSupportTicoToTarget(this);
	}

	void PomponPlant::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorPlayer(pReceiver) && !isNerve(&NrvPomponPlant::NrvSwingPlayer::sInstance)) {
			if (MR::sendMsgTouchJump(pReceiver, pSender)) {
				setNerve(&NrvPomponPlant::NrvSwingPlayer::sInstance);
			}
		}
	}

	bool PomponPlant::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgSupportTicoSpin(msg)) {
			setNerve(&NrvPomponPlant::NrvSwingPlayer::sInstance);
		}
		else if (MR::isMsgStarPieceReflect(msg)) {
			setNerve(&NrvPomponPlant::NrvSwingPlayer::sInstance);
			return true;
		}

		return false;
	}

	bool PomponPlant::tryGenItem() {
		if (mItemGenerated) {
			return false;
		}

		if (mCollectible == -1) {
			MR::appearCoinPop(this, getSensor("Body")->mPosition, mAmount);
		}
		else if (mCollectible == 0) {
			if (MR::appearStarPiece(this, getSensor("Body")->mPosition, 10.0f, 40.0f, mAmount, false)) {
				MR::startActionSound(this, "OjStarPieceBurst", -1, -1, -1);
			}
		}

		mItemGenerated = true;
		return true;
	};

	void PomponPlant::exeSwingDpd() {
		if (MR::isFirstStep(this)) {
			MR::startActionSound(this, "OjPomponPlantDpdHit", -1, -1, -1);
			MR::startAction(this, "DpdHit");
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPomponPlant::NrvWait::sInstance);
		}
	}

	void PomponPlant::exeSwingPlayer() {
		if (MR::isFirstStep(this)) {
			MR::startActionSound(this, "OjPomponPlantHit", -1, -1, -1);
			MR::startAction(this, "Hit");
			tryGenItem();
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPomponPlant::NrvWait::sInstance);
		}
	}

	namespace NrvPomponPlant {
		void NrvSwingPlayer::execute(Spine *pSpine) const {
			PomponPlant *pActor = (PomponPlant*)pSpine->mExecutor;
			pActor->exeSwingPlayer();
		}

		void NrvSwingDpd::execute(Spine *pSpine) const {
			PomponPlant *pActor = (PomponPlant*)pSpine->mExecutor;
			pActor->exeSwingDpd();
		}

		void NrvWait::execute(Spine *pSpine) const {
			PomponPlant *pActor = (PomponPlant*)pSpine->mExecutor;

			if (MR::tryStarPointerCheckWithoutRumble(pActor, 0)) {
				pActor->setNerve(&NrvSwingDpd::sInstance);
			}
		}

		NrvWait(NrvWait::sInstance);
		NrvSwingPlayer(NrvSwingPlayer::sInstance);
		NrvSwingDpd(NrvSwingDpd::sInstance);
	}
}
