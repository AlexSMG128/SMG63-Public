#include "Mogu.h"

/*
* Author: Aurum
* Objects: Mogu
* Parameters:
* - Obj_arg0, bool, false: Disable wrench gravity
* - Obj_arg1, bool, false: Use Dreadnought sight
* - SW_AWAKE
* - SW_DEAD, use, write: Activated when killed
* - SW_A, use, read: Enable throwing
* 
* A Monty Mole that hides in a bunker. If the player approaches one, it hops out of its bunker to
* attack the player using its wrenches. It will hide again if the player is too close or too far
* from this enemy.
*/

namespace pt {
	Mogu::Mogu(const char *pName) : LiveActor(pName) {
		mAnimScaleCtrl = NULL;
		mStone = NULL;
		mHole = NULL;
		mFrontVec.set(0.0f, 0.0f, 1.0f);
		mUpVec.set(0.0f, 1.0f, 0.0f);
		mCalcStoneGravity = true;
		mIsCannonFleetSight = false;
	}

	void Mogu::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, false);

		// Gravity based on up vector
		MR::calcUpVec(&mGravity, this);
		mGravity.negate();

		// Initialize items
		MR::initShadowVolumeSphere(this, 60.0f * mScale.y);
		MR::invalidateShadow(this, NULL);

		MR::declareStarPiece(this, 3);
		MR::declareCoin(this, 1);

		// Get Obj_args
		bool useStoneGravity = false;
		MR::getJMapInfoArg0NoInit(rIter, &useStoneGravity);
		mCalcStoneGravity = !useStoneGravity;

		MR::getJMapInfoArg1NoInit(rIter, &mIsCannonFleetSight);

		// Initialize 2P behavior
		mAnimScaleCtrl = new AnimScaleController(NULL);
		mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);

		// Initialize Stone and Hole
		mArmPos = new FixedPosition(this, "ArmR2", TVec3f(0.0f, 0.0f, 0.0f), TVec3f(67.38f, 0.0f, 0.0f));
		mStone = new MoguStone("投石用の石", "MoguStone");
		mStone->initWithoutIter();

		mHole = new ModelObj("巣穴モデル", "MoguHole", NULL, -2, -2, -2, false);
		mHole->mTranslation.set(mTranslation);
		mHole->mRotation.set(mRotation);
		mHole->initWithoutIter();
		MR::startBrk(mHole, "Normal"); // Required in SMG2, smh

		// Setup nerve and make appeared
		initNerve(&NrvMogu::NrvSearch::sInstance, 0);
		makeActorAppeared();
	}

	void Mogu::kill() {
		LiveActor::kill();

		MR::emitEffect(this, "Death");
		MR::startAction(mHole, "Down");
		MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
		MR::startActionSound(this, "OjStarPieceBurst", -1, -1, -1);
	}

	void Mogu::endClipped() {
		LiveActor::endClipped();

		// Reset NrvTurn
		if (isNerve(&NrvMogu::NrvTurn::sInstance)) {
			setNerve(&NrvMogu::NrvTurn::sInstance);
		}
	}

	void Mogu::control() {
		mAnimScaleCtrl->updateNerve();

		if (!isNerveTypeInactive() && mBindStarPointer->tryStartPointBind()) {
			setNerve(&NrvMogu::NrvDPDSwoon::sInstance);
		}
	}

	void Mogu::calcAndSetBaseMtx() {
		TMtx34f mtxTR;
		MR::makeMtxUpFrontPos((TPos3f*)&mtxTR, mUpVec, mFrontVec, mTranslation);
		MR::setBaseTRMtx(this, *(TPos3f*)&mtxTR);
		MR::updateBaseScale(this, mAnimScaleCtrl);

		// Update wrench position
		if (isNerve(&NrvMogu::NrvThrow::sInstance) && MR::isLessStep(this, 47)) {
			mArmPos->calc();
			mArmPos->copyTrans(&mStone->mTranslation);
		}
	}

	void Mogu::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (pSender == getSensor("Body")) {
			if ((isNerve(&NrvMogu::NrvDPDSwoon::sInstance) || !isNerveTypeInactive()) && MR::isSensorPlayer(pReceiver)) {
				MR::sendMsgPush(pReceiver, pSender);
			}
		}
	}

	bool Mogu::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgStarPieceAttack(msg)) {
			if (isNerveTypeInactive()) {
				mAnimScaleCtrl->startHitReaction();
			}
			else if (isNerve(&NrvMogu::NrvAppear::sInstance)
				|| isNerve(&NrvMogu::NrvSearch::sInstance)
				|| isNerve(&NrvMogu::NrvThrow::sInstance))
			{
				setNerve(&NrvMogu::NrvSwoonStart::sInstance);
				return true;
			}
		}
		else {
			if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgInvincibleAttack(msg)) {
				if (isNerve(&NrvMogu::NrvHideWait::sInstance)
					|| isNerve(&NrvMogu::NrvHide::sInstance)
					|| isNerve(&NrvMogu::NrvStampDeath::sInstance)
					|| isNerve(&NrvMogu::NrvHitBlow::sInstance)) {
					return false;
				}
			}
			else if (!isNerve(&NrvMogu::NrvDPDSwoon::sInstance) && isNerveTypeInactive()) {
				return false;
			}
			else if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
				setNerve(&NrvMogu::NrvStampDeath::sInstance);
				return true;
			}
			else if (!MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
				return false;
			}

			return tryPunchHitted(pSender, pReceiver);
		}

		return false;
	}

	void Mogu::exeHideWait() {
		TVec3f playerOffset = *MR::getPlayerPos() - mTranslation;
		MR::normalizeOrZero(&playerOffset);

		// Is player above?
		if (mGravity.dot(playerOffset) >= -0.75f) {
			f32 distToPlayer = MR::calcDistanceToPlayer(this);

			if (MR::isGreaterStep(this, 120) && 400.0f < distToPlayer && distToPlayer < 2000.0f) {
				setNerve(&NrvMogu::NrvAppear::sInstance);
			}
		}
	}

	void Mogu::exeHide() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Hide");
			MR::startAction(mHole, "Hide");
			MR::startActionSound(this, "EmMoguHide", -1, -1, -1);

			TVec3f playerOffset = mTranslation - *MR::getPlayerCenterPos();
			f32 mag = PSVECMag(playerOffset);

			MR::vecKillElement(playerOffset, mGravity, &playerOffset);
			MR::normalizeOrZero(&playerOffset);
			f32 dot = MR::getPlayerVelocity()->dot(playerOffset);

			if (8.0f < dot || mag < 200.0f) {
				MR::setBckRate(this, 1.5f);
			}
		}

		if (MR::isActionEnd(this)) {
			MR::startActionSound(this, "EmMoguholeClose", -1, -1, -1);
			setNerve(&NrvMogu::NrvHideWait::sInstance);
		}
	}

	void Mogu::exeAppear() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Appear");
			MR::startAction(mHole, "Open");
			MR::startActionSound(this, "EmMoguholeOpen", -1, -1, -1);
			MR::startActionSound(this, "EmMoguAppear", -1, -1, -1);

			TVec3f dirToPlayer;
			MR::calcVecToPlayerH(&dirToPlayer, this, NULL);
			MR::turnVecToVecRadian(&mFrontVec, mFrontVec, dirToPlayer, JGeometry::TUtil<f32>::PI(), mUpVec);
		}

		if (MR::isGreaterStep(this, 30) && isNearPlayerHipDrop()) {
			setNerve(&NrvMogu::NrvSwoonStart::sInstance);
		}
		else if (MR::calcDistanceToPlayer(this) < 400.0f || isPlayerExistUp()) {
			setNerve(&NrvMogu::NrvHide::sInstance);
		}
		else if (MR::isActionEnd(this)) {
			setNerve(&NrvMogu::NrvSearch::sInstance);
		}
	}

	void Mogu::exeSearch() {
		if (MR::isFirstStep(this)) {
			if (isNerve(&NrvMogu::NrvTurn::sInstance)) {
				MR::startAction(this, "Turn");
			}
			else {
				MR::startAction(this, "Wait");
			}
		}

		if (isNearPlayerHipDrop()) {
			setNerve(&NrvMogu::NrvSwoonStart::sInstance);
		}
		else {
			if (isNerve(&NrvMogu::NrvTurn::sInstance)) {
				TVec3f dirToPlayer;
				MR::calcVecToPlayerH(&dirToPlayer, this, NULL);
				MR::turnVecToVecRadian(&mFrontVec, mFrontVec, dirToPlayer, 0.03f, mUpVec);
			}

			f32 distToPlayer = MR::calcDistanceToPlayer(this);
			f32 sight = mIsCannonFleetSight ? 1500.0f : 900.0f;

			// Hide if too close to player, too far from player or if player is above
			if (distToPlayer < 400.0f || isPlayerExistUp() || 2000.0f < distToPlayer) {
				setNerve(&NrvMogu::NrvHide::sInstance);
			}
			// Turn to player if in sight
			else if (isNerve(&NrvMogu::NrvSearch::sInstance) && distToPlayer < sight) {
				if (!MR::isValidSwitchA(this) || MR::isValidSwitchA(this)) {
					setNerve(&NrvMogu::NrvTurn::sInstance);
				}
			}
			// Search player if out of sight
			else if (isNerve(&NrvMogu::NrvTurn::sInstance) && sight < distToPlayer) {
				setNerve(&NrvMogu::NrvSearch::sInstance);
			}
			// Throw wrench if in direct sight
			else if (!MR::isValidSwitchA(this) || MR::isValidSwitchA(this)) {
				if (MR::isInSightFanPlayer(this, mFrontVec, sight, 10.0f, 90.0f)) {
					if (MR::isGreaterStep(this, 45) && MR::isDead(mStone)) {
						setNerve(&NrvMogu::NrvThrow::sInstance);
					}
				}
			}
		}
	}

	void Mogu::exeThrow() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Throw");
			mStone->appear();
			MR::startAction(mStone, "Rotate");
			MR::startActionSound(this, "EmMoguTakeItem", -1, -1, -1);
		}

		if (isNearPlayerHipDrop()) {
			setNerve(&NrvMogu::NrvSwoonStart::sInstance);
		}
		else {
			if (MR::isStep(this, 47)) {
				TVec3f upVec;
				MR::calcUpVec(&upVec, this);

				TVec3f playerOffset = *MR::getPlayerCenterPos() - mStone->mTranslation;
				TVec3f dirToPlayer;
				MR::vecKillElement(playerOffset, upVec, &dirToPlayer);

				f32 dot = upVec.dot(playerOffset);
				f32 mag = PSVECMag(dirToPlayer);
				TVec3f throwDir = mFrontVec * mag - upVec * dot;

				mStone->emit(mCalcStoneGravity, mStone->mTranslation, throwDir, 15.0f);
				MR::startActionSound(this, "EmMoguThrow", -1, -1, -1);
			}

			f32 distToPlayer = MR::calcDistanceToPlayer(this);

			if (distToPlayer < 400.0f || isPlayerExistUp() || 2000.0f < distToPlayer) {
				setNerve(&NrvMogu::NrvHide::sInstance);
			}
			else if (MR::isActionEnd(this)) {
				setNerve(&NrvMogu::NrvSearch::sInstance);
			}
		}
	}

	void Mogu::tearDownThrow() {
		if (mStone->isTaken()) {
			mStone->kill();
		}
	}

	void Mogu::exeSwoonStart() {
		if (MR::isFirstStep(this)) {
			MR::startActionSound(this, "EmMoguTurnover", -1, -1, -1);
			MR::startActionSound(this, "EvMoguTurnover", -1, -1, -1);
			MR::startAction(this, "SwoonStart");

			if (!MR::isBckPlaying(mHole, "Open")) {
				MR::startAction(mHole, "Break");
			}
		}

		MR::startActionSound(this, "EmLvSwoonS", -1, -1, -1);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvMogu::NrvSwoon::sInstance);
		}
	}

	void Mogu::exeSwoon() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Swoon");
		}

		MR::startActionSound(this, "EmLvSwoonS", -1, -1, -1);

		if (isNearPlayerHipDrop()) {
			setNerve(&NrvMogu::NrvHipDropReaction::sInstance);
		}
		else if (MR::isGreaterStep(this, 120)) {
			setNerve(&NrvMogu::NrvSwoonEnd::sInstance);
		}
	}

	void Mogu::exeSwoonEnd() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "SwoonEnd");
		}

		MR::startActionSound(this, "EmLvMoguSwoonRecover", -1, -1, -1);

		if (MR::isActionEnd(this)) {
			MR::startAction(mHole, "Close");
			MR::startActionSound(this, "EmMoguholeClose", -1, -1, -1);
			setNerve(&NrvMogu::NrvHideWait::sInstance);
		}
	}

	void Mogu::exeHipDropReaction() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "HipDropReaction");
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvMogu::NrvSwoon::sInstance);
		}
	}

	void Mogu::exeStampDeath() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Down");
			MR::startAction(mHole, "Close");
			MR::startActionSound(this, "EmStompedS", -1, -1, -1);
		}

		if (MR::isGreaterStep(this, 60)) {
			kill();
			MR::appearCoinPop(this, mTranslation, 1);
		}
	}

	void Mogu::exeHitBlow() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "PunchDown");
			MR::startAction(mHole, "Close");
			MR::startActionSound(this, "EmMoguholeClose", -1, -1, -1);
			MR::startBlowHitSound(this);
			MR::validateShadow(this, NULL);

			TVec3f dirToPlayer;
			MR::calcVecToPlayerH(&dirToPlayer, this, NULL);
			MR::turnVecToVecRadian(&mFrontVec, mFrontVec, dirToPlayer, JGeometry::TUtil<f32>::PI(), mUpVec);
		}

		MR::applyVelocityDampAndGravity(this, 2.0f, 0.8f, 0.98f, 0.98f, 1.0f);

		if (MR::isGreaterStep(this, 20)) {
			kill();
			MR::appearStarPiece(this, mTranslation, 3, 10.0f, 40.0f, false);
		}
	}

	void Mogu::exeDPDSwoon() {
		if (MR::isFirstStep(this)) {
			mAnimScaleCtrl->startDpdHitVibration();
		}

		MR::startDPDFreezeLevelSound(this);
		MR::updateActorStateAndNextNerve(this, (ActorStateBaseInterface*)mBindStarPointer, &NrvMogu::NrvSearch::sInstance);
	}

	void Mogu::endDPDSwoon() {
		mAnimScaleCtrl->startAnim();
	}

	bool Mogu::isPlayerExistUp() const {
		TVec3f playerOffset = *MR::getPlayerCenterPos() - mTranslation;

		if (mUpVec.dot(playerOffset) < 0.0f) {
			return false;
		}

		MR::vecKillElement(playerOffset, *MR::getPlayerGravity(), &playerOffset);
		f32 mag = PSVECMag(playerOffset);

		return mag < 400.0f;
	}

	bool Mogu::isNearPlayerHipDrop() const {
		if (MR::isPlayerHipDropLand()) {
			f32 distToPlayer = MR::calcDistanceToPlayer(this);

			if (130.0f < distToPlayer && distToPlayer < 1500.0f) {
				return true;
			}
		}

		return false;
	}

	bool Mogu::isNerveTypeInactive() const {
		if (isNerve(&NrvMogu::NrvHideWait::sInstance)
			|| isNerve(&NrvMogu::NrvHide::sInstance)
			|| isNerve(&NrvMogu::NrvStampDeath::sInstance)
			|| isNerve(&NrvMogu::NrvHitBlow::sInstance)
			|| isNerve(&NrvMogu::NrvDPDSwoon::sInstance))
		{
			return true;
		}

		return false;
	}

	bool Mogu::tryPunchHitted(HitSensor *pSender, HitSensor *pReceiver) {
		TVec3f offBetweenSensors = pReceiver->mPosition - pSender->mPosition;
		MR::vecKillElement(offBetweenSensors, mGravity, &offBetweenSensors);
		MR::normalizeOrZero(&offBetweenSensors);

		if (MR::isNearZero(offBetweenSensors, 0.001f)) {
			setNerve(&NrvMogu::NrvStampDeath::sInstance);
		}
		else {
			offBetweenSensors *= 30.0f;
			offBetweenSensors -= mGravity * 50.0f;
			mVelocity.set(offBetweenSensors);

			if (MR::isOnGround(this)) {
				mTranslation -= mGravity * 5.0f;
			}

			setNerve(&NrvMogu::NrvHitBlow::sInstance);
		}

		return true;
	}

	namespace NrvMogu {
		void NrvHideWait::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeHideWait();
		}

		void NrvHide::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeHide();
		}

		void NrvAppear::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvSearch::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeSearch();
		}

		void NrvTurn::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeSearch();
		}

		void NrvThrow::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeThrow();
		}

		void NrvThrow::executeOnEnd(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->tearDownThrow();
		}

		void NrvSwoonStart::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeSwoonStart();
		}

		void NrvSwoonEnd::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeSwoonEnd();
		}

		void NrvSwoon::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeSwoon();
		}

		void NrvHipDropReaction::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeHipDropReaction();
		}

		void NrvStampDeath::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeStampDeath();
		}

		void NrvHitBlow::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeHitBlow();
		}

		void NrvDPDSwoon::execute(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->exeDPDSwoon();
		}

		void NrvDPDSwoon::executeOnEnd(Spine *pSpine) const {
			Mogu *pActor = (Mogu*)pSpine->mExecutor;
			pActor->endDPDSwoon();
		}

		NrvHideWait(NrvHideWait::sInstance);
		NrvHide(NrvHide::sInstance);
		NrvAppear(NrvAppear::sInstance);
		NrvSearch(NrvSearch::sInstance);
		NrvTurn(NrvTurn::sInstance);
		NrvThrow(NrvThrow::sInstance);
		NrvSwoonStart(NrvSwoonStart::sInstance);
		NrvSwoonEnd(NrvSwoonEnd::sInstance);
		NrvSwoon(NrvSwoon::sInstance);
		NrvHipDropReaction(NrvHipDropReaction::sInstance);
		NrvStampDeath(NrvStampDeath::sInstance);
		NrvHitBlow(NrvHitBlow::sInstance);
		NrvDPDSwoon(NrvDPDSwoon::sInstance);
	}
}
