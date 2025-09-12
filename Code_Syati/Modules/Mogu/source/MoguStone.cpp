#include "MoguStone.h"

/*
* Author: Aurum
* Objects: n/a
* Parameters: n/a
* 
* This is the throwing wrench used by Monty Moles to attack the player. This is also used as a base class for the ice
* chunks thrown by King Brr.
*/

namespace pt {
	MoguStone::MoguStone(const char *pName, const char *pModelName) : ModelObj(pName, pModelName, NULL, -2, -2, -2, false) {
		mRotateQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
		mUpQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
		mFrontVec.set(0.0f, 0.0f, 1.0f);
		mSpeed = 0.0f;
		mCalcGravity = true;
	}

	void MoguStone::init(const JMapInfoIter &rIter) {
		ModelObj::init(rIter);

		MR::invalidateClipping(this);
		MR::onCalcGravity(this);
		MR::initShadowVolumeSphere(this, 50.0f * mScale.y);

		initNerve(&NrvMoguStone::NrvTaken::sInstance, 0);
		makeActorDead();
	}

	void MoguStone::appear() {
		ModelObj::appear();
		setNerve(&NrvMoguStone::NrvTaken::sInstance);
	}

	void MoguStone::kill() {
		ModelObj::kill();
		MR::emitEffect(this, "Break");
	}

	void MoguStone::calcAndSetBaseMtx() {
		TMtx34f mtxTR;
		((TPos3f*)&mtxTR)->setTrans(mTranslation);
		((TRot3f*)&mtxTR)->setQuat(mRotateQuat);
		MR::setBaseTRMtx(this, *(TPos3f*)&mtxTR);
	}

	void MoguStone::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (pSender == getSensor("Body") && MR::isSensorPlayer(pReceiver)) {
			if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
				MR::startActionSound(this, "BmIcemerakingStoneBreak", -1, -1, -1);
				kill();
			}
		}
	}

	bool MoguStone::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (!isNerve(&NrvMoguStone::NrvThrow::sInstance)) {
			return false;
		}

		if (MR::isMsgLockOnStarPieceShoot(msg)) {
			return true;
		}

		if (!MR::isMsgPlayerHitAll(msg) && !MR::isMsgStarPieceAttack(msg)) {
			return false;
		}

		MR::startActionSound(this, "BmIcemerakingStoneBreak", -1, -1, -1);
		kill();
		return true;
	}

	void MoguStone::doBehavior() {
		if (MR::isFirstStep(this)) {
			mUpQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
		}

		if (mCalcGravity) {
			MR::vecKillElement(mFrontVec, mGravity, &mFrontVec);
			MR::normalizeOrZero(&mFrontVec);
		}

		mVelocity.scale(mSpeed, mFrontVec);
		MR::makeQuatUpFront(&mRotateQuat, -mGravity, mFrontVec);

		if (MR::isGreaterStep(this, 100)) {
			startBreakSound();
			kill();
		}
	}

	void MoguStone::startBreakSound() {
		MR::startActionSound(this, "EmMoguSpanaBreak", -1, -1, -1);
	}

	void MoguStone::startThrowLevelSound() {
		MR::startActionSound(this, "EmLvMoguSpanaFly", -1, -1, -1);
	}

	void MoguStone::exeThrow() {
		startThrowLevelSound();
		doBehavior();

		PSQUATMultiply(mUpQuat, mRotateQuat, mRotateQuat);
		mRotateQuat.normalize();

		if (MR::isBinded(this)) {
			startBreakSound();
			kill();
		}
		else if (MR::isGreaterStep(this, 100)) {
			setNerve(&NrvMoguStone::NrvFall::sInstance);
		}
	}

	void MoguStone::exeFall() {
		PSQUATMultiply(mUpQuat, mRotateQuat, mRotateQuat);
		mRotateQuat.normalize();

		MR::applyVelocityDampAndGravity(this, 2.0f, 0.8f, 0.98f, 0.98f, 1.0f);

		if (MR::isBinded(this) || MR::isGreaterStep(this, 60)) {
			startBreakSound();
			kill();
		}
	}

	void MoguStone::emit(bool calcGravity, const TVec3f &rTranslation, const TVec3f &rFrontVec, f32 speed) {
		mSpeed = speed;
		mTranslation.set(rTranslation);
		mVelocity.zero();
		MR::calcGravity(this);

		if (calcGravity) {
			MR::onCalcGravity(this);
			MR::vecKillElement(rFrontVec, mGravity, &mFrontVec);
			mCalcGravity = true;
		}
		else {
			MR::offCalcGravity(this);
			mFrontVec.set(rFrontVec);
			mCalcGravity = false;
		}

		MR::normalizeOrZero(&mFrontVec);

		setNerve(&NrvMoguStone::NrvThrow::sInstance);
	}

	bool MoguStone::isTaken() {
		return isNerve(&NrvMoguStone::NrvTaken::sInstance);
	}

	namespace NrvMoguStone {
		void NrvTaken::execute(Spine *pSpine) const {}

		void NrvThrow::execute(Spine *pSpine) const {
			MoguStone *pActor = (MoguStone*)pSpine->mExecutor;
			pActor->exeThrow();
		}

		void NrvFall::execute(Spine *pSpine) const {
			MoguStone *pActor = (MoguStone*)pSpine->mExecutor;
			pActor->exeFall();
		}

		NrvTaken(NrvTaken::sInstance);
		NrvThrow(NrvThrow::sInstance);
		NrvFall(NrvFall::sInstance);
	}
}
