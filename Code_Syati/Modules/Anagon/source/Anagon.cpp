#include "Anagon.h"

/*
* Author: Aurum
* Objects: Anagon
* Parameters:
* - Obj_arg0, float, 20.0f: Speed
* - SW_AWAKE
* - SW_APPEAR, use
* - SW_DEAD, use, write: Activated when killed
* - Rail, need: For movement
*
* This is a simplified port of SkeletalFishBaby, the swimming Gringill enemy from Drip Drop Galaxy and Deep Dark Galaxy.
* As SMG1's actor has a lot of complicated code related to the Kingfin boss fight, I attempted to rewrite it entirely.
* The name is different because Anagon is the proper Japanese name for Gringills. Interestingly, sound data for this
* enemy was already present in ActionSound under that name, suggesting that Gringills were meant to appear in the game
* at some point.
*/

namespace {
	const float sStarPointerTargetSize[3] = { 150.0f, 120.0f, 120.0f };
	const char* sStarPointerTargetJoint[3] = { "Joint01", "Joint02", "Joint03" };
}

namespace pt {
	extern void initRailToNearestAndRepositionWithGravity(LiveActor* pActor);
	extern void getTransRotateFromRailWithGravity(LiveActor* pActor);

	Anagon::Anagon(const char *pName) : LiveActor(pName) {
		for (s32 i = 0; i < 3; i++) {
			mStarPointerTargets[i] = NULL;
		}

		mAnimScaleCtrl = NULL;
	}

	void Anagon::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, false);
		MR::onCalcGravity(this);

		// Initialize rail rider
		f32 speed = 20.0f;
		MR::getJMapInfoArg0NoInit(rIter, &speed);
		mRailRider->setSpeed(speed);

		pt::initRailToNearestAndRepositionWithGravity(this);

		// Initialize miscellaneous attributes
		MR::addToAttributeGroupSearchTurtle(this);
		MR::declareStarPiece(this, 10);

		// Initialize 2P behavior
		mAnimScaleCtrl = new AnimScaleController(NULL);

		for (s32 i = 0; i < 3; i++) {
			LiveActor *pTarget = new LiveActor("StarPointerTargetDummy");
			mStarPointerTargets[i] = pTarget;

			MtxPtr pMtx = (MtxPtr)MR::getJointMtx(this, ::sStarPointerTargetJoint[i]);
			pTarget->initActorStarPointerTarget(::sStarPointerTargetSize[i], NULL, pMtx, TVec3f(0.0f, 0.0f, 0.0f));
		}

		// Setup nerve and try to make appeared
		initNerve(&NrvAnagon::NrvSwim::sInstance, 0);
		MR::useStageSwitchSyncAppear(this, rIter);
	}

	void Anagon::makeActorAppeared() {
		LiveActor::makeActorAppeared();

		for (s32 i = 0; i < 3; i++) {
			mStarPointerTargets[i]->makeActorAppeared();
		}

		MR::startActionSound(this, "BmSklFishBabyAppear", -1, -1, -1);
		MR::validateClipping(this);
		setNerve(&NrvAnagon::NrvSwim::sInstance);
	}

	void Anagon::kill() {
		LiveActor::kill();

		for (s32 i = 0; i < 3; i++) {
			mStarPointerTargets[i]->makeActorDead();
		}

		MR::startActionSound(this, "EmExplodeUnderWater", -1, -1, -1);
	}

	void Anagon::control() {
		mAnimScaleCtrl->update();
	}

	void Anagon::calcAndSetBaseMtx() {
		LiveActor::calcAndSetBaseMtx();
		MR::updateBaseScale(this, mAnimScaleCtrl);
	}

	void Anagon::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorPlayerOrRide(pReceiver)) {
			if (isNerve(&NrvAnagon::NrvSwim::sInstance) && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
				MR::shakeCameraStrong();
			}
			else {
				MR::sendMsgPush(pReceiver, pSender);
			}
		}
	}

	bool Anagon::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgInvincibleAttack(msg)) {
			if (isNerve(&NrvAnagon::NrvSwim::sInstance) || isNerve(&NrvAnagon::NrvBind::sInstance)) {
				MR::shakeCameraNormal();
				setNerve(&NrvAnagon::NrvBreak::sInstance);
				return true;
			}
		}
		else if (MR::isMsgStarPieceReflect(msg)) {
			mAnimScaleCtrl->startHitReaction();
			return true;
		}

		return false;
	}

	void Anagon::exeSwim() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Swim");
		}

		MR::startActionSound(this, "EmLvSnakefishSwim", -1, -1, -1);

		if (isStarPointerPointing()) {
			setNerve(&NrvAnagon::NrvBind::sInstance);
		}

		mRailRider->move();
		pt::getTransRotateFromRailWithGravity(this);
	}

	void Anagon::exeBind() {
		if (MR::isFirstStep(this)) {
			mAnimScaleCtrl->startDpdHitVibration();
			MR::emitEffect(this, "StarPointerHold");
		}

		MR::startDPDFreezeLevelSound(this);

		if (!isStarPointerPointing()) {
			setNerve(&NrvAnagon::NrvSwim::sInstance);
		}
	}

	void Anagon::endBind() {
		MR::tryDeleteEffect(this, "StarPointerHold");
		mAnimScaleCtrl->startAnim();
	}

	void Anagon::exeBreak() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Death");
			MR::startActionSound(this, "EmExplodeUnderWater", -1, -1, -1);
			MR::startActionSound(this, "EvSnakeheadDamage", -1, -1, -1);
			MR::invalidateClipping(this);
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvAnagon::NrvDead::sInstance);
		}
	}

	void Anagon::exeDead() {
		if (MR::isFirstStep(this)) {
			TVec3f starPiecePos;
			MR::copyJointPos(this, "Center", &starPiecePos);
			MR::appearStarPiece(this, starPiecePos, 10, 10.0f, 40.0f, false);
			MR::startActionSound(this, "OjStarPieceBurstWF", -1, -1, -1);
			kill();
		}
	}

	bool Anagon::isStarPointerPointing() {
		if (MR::attachSupportTicoToTarget(this)) {
			return true;
		}

		for (s32 i = 0; i < 3; i++) {
			if (MR::attachSupportTicoToTarget(mStarPointerTargets[i])) {
				return true;
			}
		}

		return false;
	}

	namespace NrvAnagon {
		void NrvSwim::execute(Spine *pSpine) const {
			Anagon *pActor = (Anagon*)pSpine->mExecutor;
			pActor->exeSwim();
		}

		void NrvBind::execute(Spine *pSpine) const {
			Anagon *pActor = (Anagon*)pSpine->mExecutor;
			pActor->exeBind();
		}

		void NrvBind::executeOnEnd(Spine *pSpine) const {
			Anagon *pActor = (Anagon*)pSpine->mExecutor;
			pActor->endBind();
		}

		void NrvBreak::execute(Spine *pSpine) const {
			Anagon *pActor = (Anagon*)pSpine->mExecutor;
			pActor->exeBreak();
		}

		void NrvDead::execute(Spine *pSpine) const {
			Anagon *pActor = (Anagon*)pSpine->mExecutor;
			pActor->exeDead();
		}

		NrvSwim(NrvSwim::sInstance);
		NrvBind(NrvBind::sInstance);
		NrvBreak(NrvBreak::sInstance);
		NrvDead(NrvDead::sInstance);
	}
}
