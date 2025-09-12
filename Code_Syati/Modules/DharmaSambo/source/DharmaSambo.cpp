#include "DharmaSambo.h"

/*
* Author: Aurum
* Objects: DharmaSambo
* Parameters:
*  - Obj_arg0, float, 800.0f: Appearance range
*  - SW_APPEAR, use
*  - SW_DEAD, use, write: Activated when killed
*  - SW_AWAKE
*/

namespace {
	const char* joint_name[4] = { "Spine1", "Spine2", "Spine3", "Spine4" };
	const float hPursueEscapeDist[4] = { 100.0f, 200.0f, 300.0f, 400.0f };
	const MR::ActorMoveParam hPursueGoParam           = { 0.3f, 1.0f, 0.97f, 3.0f };
	const MR::ActorMoveParam hPursueEscapeParam       = { -0.25f, 1.0f, 0.97f, 3.0f };
	const MR::ActorMoveParam hOnlyTurnParam           = { 0.0f, 1.0f, 0.97f, 3.0f };
	const MR::ActorMoveParam hNoMoveNoTurnParam       = { 0.0f, 1.0f, 0.8f, 0.0f };
	const MR::ActorMoveParam hSpinHittedOnGroundParam = { 0.0f, 1.0f, 0.8f, 3.0f };
	const MR::ActorMoveParam hSpinHittedAirParam      = { 0.0f, 2.0f, 0.97f, 0.0f };
	const MR::ActorMoveParam hEscapeOnGroundParam     = { 0.0f, 1.0f, 0.7f, 5.0f };
	const MR::ActorMoveParam hEscapeAirParam          = { 0.15f, 2.0f, 0.98f, 5.0f };
	const MR::ActorMoveParam hEscapeWaitOnGroundParam = { 0.0f, 1.0f, 0.7f, 5.0f };
	const MR::ActorMoveParam hEscapeWaitAirParam      = { 0.0f, 2.0f, 0.98f, 5.0f };
}

namespace pt {
	extern AnimScaleController* createSamboAnimScaleController(AnimScaleParam* pAnimScaleParam);
	extern void moveAndTurnToPlayer(LiveActor* pActor, const MR::ActorMoveParam& rParam);
	extern void moveAndTurnToDirection(LiveActor* pActor, const TVec3f& rDir, const MR::ActorMoveParam& rParam);

	DharmaSambo::DharmaSambo(const char *pName) : LiveActor(pName) {
		mResumeStarPointerBindNerve = NULL;
		mHomePos.zero();
		mHideCounter = 600;
		mRemainingParts = 4;
		mParts.mCount = 0;
		mPositions.mCount = 0;
		mBlowAttackerSensor = NULL;
		mAnimScaleCtrl = NULL;
		mAnimScaleParam = NULL;
		mAppearRange = 800.0f;
	}

	void DharmaSambo::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, "DharmaSambo", false);

		// Initialize other attributes
		mHomePos.set(mTranslation);
		MR::addEffectHitNormal(this, NULL);
		MR::onCalcGravity(this);
		MR::declareStarPiece(this, 6);
		MR::getJMapInfoArg0NoInit(rIter, &mAppearRange);

		// Initialize anim scale
		mAnimScaleParam = new AnimScaleParam();
		mAnimScaleCtrl = pt::createSamboAnimScaleController(mAnimScaleParam);

		// Initialize Parts and FixedPositions
		mParts.mArray.init(4);
		mPositions.mArray.init(4);

		for (s32 i = 0; i < 4; i++) {
			MR::hideJoint(this, ::joint_name[i]);

			DharmaSamboParts *pParts = new DharmaSamboParts(this, "だるまサンボパーツ", "DharmaSamboParts");
			pParts->initWithoutIter();
			pParts->setHead(i == 3);
			pParts->mOriginalPosIndex = i;
			pParts->mTranslation.set(mHomePos);
			mParts.assign(pParts, i);
			mParts.mCount++;

			FixedPosition *pPosition = new FixedPosition(this, ::joint_name[i], TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, -90.0f, -90.0f));
			mPositions.assign(pPosition, i);
			mPositions.mCount++;
		}

		// Setup nerve and try to appear
		initNerve(&NrvDharmaSambo::NrvWaitUnderGround::sInstance, 0);

		if (MR::isValidSwitchAppear(this)) {
			MR::syncStageSwitchAppear(this);
			kill();
		}
		else {
			appear();
		}
	}

	void DharmaSambo::initAfterPlacement() {
		MR::trySetMoveLimitCollision(this);
	}

	void DharmaSambo::appear() {
		LiveActor::appear();

		for (s32 i = 0; i < mParts.mCount; i++) {
			mParts.mArray.mArr[i]->appear();
		}

		setNerve(&NrvDharmaSambo::NrvHideAppear::sInstance);
	}

	void DharmaSambo::kill() {
		LiveActor::kill();

		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts* pParts = mParts.mArray.mArr[i];

			if (!MR::isDead(pParts)) {
				pParts->kill();
			}
		}
	}

	void DharmaSambo::control() {
		mAnimScaleCtrl->updateNerve();

		// Kill self if head remains and is dead
		if (mRemainingParts == 1 && MR::isDead(getHeadParts())) {
			kill();
			return;
		}

		s32 partsCount = mParts.mCount;

		if (isNerve(&NrvDharmaSambo::NrvPursue::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvAttack::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvEscapeRun::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvEscapeLand::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvStarPointerBind::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvSpinHitted::sInstance) && MR::isGreaterStep(this, 50))
		{
			// Find Parts that is closest to its attacking sensor
			f32 shortestBlowAtkDist = 99999.0;
			DharmaSamboParts *pBlowAttackedParts = NULL;

			for (s32 i = 0; i < partsCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];

				if (0.0f <= pParts->mBlowAttackedDistance && pParts->mBlowAttackedDistance < shortestBlowAtkDist) {
					pBlowAttackedParts = pParts;
					shortestBlowAtkDist = pParts->mBlowAttackedDistance;
				}
			}

			// If such a part exists, kill it
			if (pBlowAttackedParts) {
				HitSensor* pSensor = pBlowAttackedParts->getSensor("Body");
				MR::emitEffectHitBetweenSensors(this, pSensor, mBlowAttackerSensor, 0.0f, NULL);
				MR::stopSceneForDefaultHit(5);

				mRemainingParts--;
				pBlowAttackedParts->mActualPosIndex = -1;
				pBlowAttackedParts->setNerve(&NrvDharmaSamboParts::NrvBlow::sInstance);

				if (pBlowAttackedParts->mIsHead) {
					setNerve(&NrvDharmaSambo::NrvKilled::sInstance);
				}
				else {
					TVec3f distToSelfFromCenter(mTranslation - *MR::getPlayerCenterPos());
					MR::addVelocitySeparateHV(this, distToSelfFromCenter, 10.0f, 30.0f);

					MR::stopBck(this);
					setNerve(&NrvDharmaSambo::NrvSpinHitted::sInstance);

					// Shift position index for all Parts
					s32 nextPosIndex = 0;

					for (s32 i = 0; i < partsCount; i++) {
						DharmaSamboParts *pParts = mParts.mArray.mArr[i];

						if (pParts->mActualPosIndex != -1) {
							pParts->mActualPosIndex = pBlowAttackedParts == pParts ? -1 : nextPosIndex++;
						}
					}
				}
			}
		}

		// Reset blow attacked distances for all Parts
		for (s32 i = 0; i < partsCount; i++) {
			mParts.mArray.mArr[i]->mBlowAttackedDistance = -1.0f;
		}

		// Apply FixedPositions to all Parts
		if (isNerve(&NrvDharmaSambo::NrvSpinHitted::sInstance)) {
			for (s32 i = 0; i < partsCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];
				mPositions.mArray.mArr[i]->calc();

				if (pParts->mActualPosIndex != -1) {
					FixedPosition *pPosition = mPositions.mArray.mArr[pParts->mActualPosIndex];

					// Smooth falling transition to lower FixedPosition
					TVec3f trans;
					pPosition->copyTrans(&trans);
					trans -= pParts->mTranslation;
					trans *= 0.5f;
					pParts->mTranslation += trans;
					pPosition->copyRotate(&pParts->mRotation);
				}
			}
		}
		else {
			for (s32 i = 0; i < partsCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];
				mPositions.mArray.mArr[i]->calc();

				if (pParts->mActualPosIndex != -1) {
					FixedPosition *pPosition = mPositions.mArray.mArr[pParts->mActualPosIndex];
					pPosition->copyTrans(&pParts->mTranslation);
					pPosition->copyRotate(&pParts->mRotation);
				}
			}
		}

		// Try Star Pointer bind
		if (isNerve(&NrvDharmaSambo::NrvPursue::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvAttack::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvSpinHitted::sInstance) && MR::isGreaterStep(this, 50)
			|| isNerve(&NrvDharmaSambo::NrvEscapeRun::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvEscapeLand::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvEscapeWaitRun::sInstance)
			|| isNerve(&NrvDharmaSambo::NrvEscapeWaitLand::sInstance))
		{
			mResumeStarPointerBindNerve = mSpine->getCurrentNerve();

			if (isStarPointerPointing()) {
				setNerve(&NrvDharmaSambo::NrvStarPointerBind::sInstance);
			}
		}
	}

	void DharmaSambo::calcAndSetBaseMtx() {
		LiveActor::calcAndSetBaseMtx();

		TVec3f scale(mAnimScaleCtrl->mScale);
		scale.mul(mScale);
		MR::setBaseScale(this, scale);
	}

	void DharmaSambo::exeWaitUnderGround() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "HideWait");
			startActionAllParts("HideWait");

			mTranslation.set(mHomePos);
			mVelocity.zero();
		}

		MR::startActionSound(this, "EmLvDharmasamboMoveUnder", -1, -1, -1);
		pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);
		f32 distToPlayer = MR::calcDistanceToPlayer(this);

		if (MR::isGreaterStep(this, 120) && distToPlayer < 800.0f) {
			setNerve(&NrvDharmaSambo::NrvAppear::sInstance);
		}
	}

	void DharmaSambo::exeAppear() {
		if (MR::isFirstStep(this)) {
			mVelocity.zero();
			mRemainingParts = 4;
			mHideCounter = 600;

			MR::startAction(this, "Appear");
			startActionAllParts("Appear");

			if (isNerve(&NrvDharmaSambo::NrvAppearImmediately::sInstance)) {
				MR::setBckFrameAndStop(this, 59.0f);
				MR::setBckRate(this, 1.0f);
			}
		}

		if (MR::isStep(this, 60)) {
			MR::startActionSound(this, "EmSfsamboAppear", -1, -1, -1);
		}

		pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvDharmaSambo::NrvPursue::sInstance);
		}
	}

	void DharmaSambo::exePursue() {
		if (MR::isFirstStep(this) && !MR::isBckPlaying(this, "Wait")) {
			MR::startAction(this, "Wait");
			startActionAllParts("Wait");
		}
		
		MR::startActionSound(this, "EmLvDharmasamboMove", -1, -1, -1);

		bool doAttack = false;
		f32 distToPlayer = MR::calcDistanceToPlayer(this);

		// Hide if player is too far away or if too far from home
		if (2000.0f < distToPlayer || 1562500.0 < mTranslation.squared(mHomePos)) {
			setNerve(&NrvDharmaSambo::NrvHide::sInstance);
			return;
		}

		// Player too far away -> move towards player
		if (100.0f + ::hPursueEscapeDist[mRemainingParts - 1] < distToPlayer) {
			pt::moveAndTurnToPlayer(this, ::hPursueGoParam);
		}
		// Player too close -> move away from player
		else if (distToPlayer < ::hPursueEscapeDist[mRemainingParts - 1]) {
			pt::moveAndTurnToPlayer(this, ::hPursueEscapeParam);
		}
		// Player is close enough -> try attack
		else {
			pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);
			doAttack = true;
		}

		if (doAttack && MR::checkPassBckFrame(this, 120.0f)) {
			setNerve(&NrvDharmaSambo::NrvAttack::sInstance);
		}
		else {
			decCountAndTryToHide(1);
		}
	}

	void DharmaSambo::exeAttack() {
		if (MR::isFirstStep(this)) {
			if (!MR::isBckPlaying(this, "Attack")) {
				mVelocity.zero();

				MR::startAction(this, "Attack");
				startActionAllParts("Attack");
			}

			if (mRemainingParts == 2) {
				MR::setBckRate(this, 1.2f);
			}
		}

		if ((mRemainingParts == 2 ? 75.0f : 90.0f) < MR::getBckFrame(this)) {
			pt::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam);
		}
		else {
			pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);
		}

		bool hide = MR::isActionEnd(this);

		if (MR::checkPassBckFrame(this, 110.0f) && !MR::checkPassBckFrame(this, 111.0f)) {
			MR::emitEffect(getHeadParts(), "Attack");
			MR::startActionSound(this, "EmSfsamboAttack", -1, -1, -1);
		}

		if (!decCountAndTryToHide(hide) && hide) {
			setNerve(&NrvDharmaSambo::NrvPursue::sInstance);
		}
	}

	void DharmaSambo::exeSpinHitted() {
		if (MR::isFirstStep(this)) {
			if (!MR::isBckPlaying(this, "SpinReaction")) {
				MR::startAction(this, "SpinReaction");
				startBtkAllParts("Press");
			}
		}

		if (MR::isOnGround(this)) {
			pt::moveAndTurnToPlayer(this, ::hSpinHittedOnGroundParam);
		}
		else {
			pt::moveAndTurnToPlayer(this, ::hSpinHittedAirParam);
		}

		if (mRemainingParts == 1 && MR::isOnGround(this)) {
			mHideCounter = 300;
			setNerve(&NrvDharmaSambo::NrvEscapeLand::sInstance);
		}
		else {
			if (MR::checkPassBckFrame(this, 27.0f) && !MR::checkPassBckFrame(this, 28.0f)) {
				startActionAllParts("Land");
			}

			if (MR::isOnGround(this) && MR::isBckOneTimeAndStopped(this)) {
				setNerve(&NrvDharmaSambo::NrvPursue::sInstance);
			}
		}
	}

	void DharmaSambo::exeStarPointerBind() {
		if (MR::isFirstStep(this)) {
			MR::setBckRate(this, 0.0f);
			MR::setBckRate(getHeadParts(), 0.0f);

			for (s32 i = 0; i < mParts.mCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];

				if (!pParts->isNerve(&NrvDharmaSamboParts::NrvBlow::sInstance)) {
					pParts->setNerve(&NrvDharmaSamboParts::NrvStarPointerBind::sInstance);
				}
			}

			mAnimScaleCtrl->startDpdHitVibration();
		}

		MR::startDPDFreezeLevelSound(this);
		mVelocity.zero();

		if (!isStarPointerPointing()) {
			setNerve(mResumeStarPointerBindNerve);
		}
	}

	void DharmaSambo::endStarPointerBind() {
		mAnimScaleCtrl->startAnim();

		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts *pParts = mParts.mArray.mArr[i];

			if (!pParts->isNerve(&NrvDharmaSamboParts::NrvBlow::sInstance)) {
				pParts->setNerve(&NrvDharmaSamboParts::NrvDoNothing::sInstance);
			}
		}

		MR::setBckRate(this, 1.0f);
		MR::setBckRate(getHeadParts(), 1.0f);
	}

	void DharmaSambo::exeEscapeRun() {
		if (MR::isFirstStep(this)) {
			DharmaSamboParts *pHead = getHeadParts();

			if (!MR::isBckPlaying(pHead, "Run")) {
				MR::startAction(pHead, "Run");
				MR::startActionSound(this, "EmSamboheadHop", -1, -1, -1);
			}

			MR::setBckRate(pHead, 1.2f);
		}

		if (isNerve(&NrvDharmaSambo::NrvEscapeWaitRun::sInstance)) {
			if (MR::isOnGround(this)) {
				pt::moveAndTurnToPlayer(this, ::hEscapeWaitOnGroundParam);
			}
			else {
				pt::moveAndTurnToPlayer(this, ::hEscapeWaitAirParam);
			}

			decCountAndTryToHide(false);

			if (MR::isOnGround(this) && isActionEndHead()) {
				setNerve(&NrvDharmaSambo::NrvEscapeWaitLand::sInstance);
			}
		}
		else {
			TVec3f distToSelf(mTranslation - *MR::getPlayerPos());
			MR::vecKillElement(distToSelf, mGravity, &distToSelf);
			MR::normalizeOrZero(&distToSelf);

			if (MR::isOnGround(this)) {
				pt::moveAndTurnToDirection(this, distToSelf, ::hEscapeOnGroundParam);
			}
			else {
				pt::moveAndTurnToDirection(this, distToSelf, ::hEscapeAirParam);
			}

			decCountAndTryToHide(false);

			if (MR::isOnGround(this) && isActionEndHead()) {
				setNerve(&NrvDharmaSambo::NrvEscapeLand::sInstance);
			}
		}
	}

	void DharmaSambo::exeEscapeLand() {
		if (MR::isFirstStep(this)) {
			DharmaSamboParts* pHead = getHeadParts();

			if (!MR::isBckPlaying(pHead, "Land")) {
				MR::startAction(pHead, "Land");
			}

			MR::setBckRate(pHead, 1.2f);
		}

		if (isNerve(&NrvDharmaSambo::NrvEscapeWaitLand::sInstance)) {
			if (MR::isOnGround(this)) {
				pt::moveAndTurnToPlayer(this, ::hEscapeWaitOnGroundParam);
			}
			else {
				pt::moveAndTurnToPlayer(this, ::hEscapeWaitAirParam);
			}
		}
		else {
			TVec3f distToSelf(mTranslation - *MR::getPlayerPos());
			MR::vecKillElement(distToSelf, mGravity, &distToSelf);
			MR::normalizeOrZero(&distToSelf);

			pt::moveAndTurnToDirection(this, distToSelf, ::hEscapeOnGroundParam);
		}

		decCountAndTryToHide(true);

		if (isActionEndHead()) {
			if (800.0f < MR::calcDistanceToPlayer(this)) {
				MR::addVelocityJump(this, 30.0f);
				setNerve(&NrvDharmaSambo::NrvEscapeWaitRun::sInstance);
			}
			else {
				TVec3f distToSelfFromCenter(mTranslation - *MR::getPlayerCenterPos());
				MR::addVelocitySeparateHV(this, distToSelfFromCenter, 7.0f, 30.0f);
				setNerve(&NrvDharmaSambo::NrvEscapeRun::sInstance);
			}
		}
	}

	void DharmaSambo::exeTrampleDeath() {
		if (MR::isFirstStep(this)) {
			MR::stopBck(this);
			startActionHead("Down");
			MR::startActionSound(this, "EmStompedS", -1, -1, -1);
		}

		if (MR::isOnGround(this)) {
			pt::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam);
		}
		else {
			pt::moveAndTurnToPlayer(this, ::hSpinHittedAirParam);
		}

		if (isActionEndHead()) {
			kill();
		}
	}

	void DharmaSambo::exeHide() {
		if (MR::isFirstStep(this)) {
			mVelocity.zero();

			MR::startAction(this, "Hide");
			startActionAllParts("Hide");

			if (mRemainingParts == 1) {
				MR::setBckRate(this, 1.2f);
			}
		}

		if (MR::isStep(this, 30)) {
			MR::startActionSound(this, "EmDharmasamboPreHide", -1, -1, -1);
		}

		if (MR::isStep(this, 65)) {
			MR::startActionSound(this, "EmSfsamboHide", -1, -1, -1);
		}

		pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);

		if (MR::isGreaterStep(this, mRemainingParts == 1 ? 360 : 300)) {
			MR::resetPosition(this, mHomePos);
			setNerve(&NrvDharmaSambo::NrvHideAppear::sInstance);
		}
	}

	void DharmaSambo::exeHideAppear() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "HideAppear");
			MR::startActionSound(this, "EmSamboheadAppear", -1, -1, -1);

			s32 headIndex = mParts.mCount - 1;

			for (s32 i = 0; i < mParts.mCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];

				if (MR::isDead(pParts)) {
					pParts->appear();
				}

				pParts->mActualPosIndex = i;
				pParts->setHead(headIndex == i);
				pParts->setNerve(&NrvDharmaSamboParts::NrvDoNothing::sInstance);
				MR::startAction(pParts, "Wait");
				MR::setBckFrameAndStop(pParts, 5.0f);

				mPositions.mArray.mArr[i]->copyTrans(&pParts->mTranslation);
				MR::resetPosition(pParts);
			}

			startActionAllParts("HideAppear");
		}

		pt::moveAndTurnToPlayer(this, ::hOnlyTurnParam);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvDharmaSambo::NrvWaitUnderGround::sInstance);
		}
	}

	void DharmaSambo::exeKilled() {
		if (MR::isFirstStep(this)) {
			MR::invalidateClipping(this);
			MR::stopBck(this);

			for (s32 i = 0; i < mParts.mCount; i++) {
				DharmaSamboParts *pParts = mParts.mArray.mArr[i];

				if (pParts->mActualPosIndex != -1) {
					MR::stopBck(pParts);
				}

				MR::invalidateHitSensor(pParts, "Body");
			}

			MR::startAction(this, "Hit");
			mVelocity.zero();
		}

		pt::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam);

		if (MR::isActionEnd(this)) {
			kill();
		}
	}

	DharmaSamboParts* DharmaSambo::getHeadParts() {
		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts* pParts = mParts.mArray.mArr[i];

			if (pParts->mIsHead) {
				return pParts;
			}
		}

		return NULL;
	}

	bool DharmaSambo::isActionEndHead() {
		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts* pParts = mParts.mArray.mArr[i];

			if (pParts->mIsHead) {
				return MR::isActionEnd(pParts);
			}
		}

		return false;
	}

	void DharmaSambo::startActionHead(const char *pName) {
		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts* pParts = mParts.mArray.mArr[i];

			if (pParts->mIsHead) {
				MR::startAction(pParts, pName);
				return;
			}
		}
	}

	void DharmaSambo::startActionAllParts(const char *pName) {
		for (s32 i = 0; i < mParts.mCount; i++) {
			MR::startAction(mParts.mArray.mArr[i], pName);
		}
	}

	void DharmaSambo::startBtkAllParts(const char *pName) {
		for (s32 i = 0; i < mParts.mCount; i++) {
			MR::startBtk(mParts.mArray.mArr[i], pName);
		}
	}

	bool DharmaSambo::decCountAndTryToHide(bool hide) {
		mHideCounter--;

		if (hide && mHideCounter <= 0) {
			setNerve(&NrvDharmaSambo::NrvHide::sInstance);
			return true;
		}

		return false;
	}

	bool DharmaSambo::isStarPointerPointing() {
		for (s32 i = 0; i < mParts.mCount; i++) {
			DharmaSamboParts* pParts = mParts.mArray.mArr[i];

			if (!MR::isDead(pParts) && MR::attachSupportTicoToTarget(pParts)) {
				return true;
			}
		}

		return false;
	}

	DharmaSamboParts::DharmaSamboParts(DharmaSambo *pHost, const char *pName, const char *pModelName)
		: ModelObj(pName, pModelName, NULL, MR::DrawBufferType_Enemy, MR::MovementType_EnemyDecoration, MR::CategoryList_Auto, false)
	{
		mHost = pHost;
		mOriginalPosIndex = -1;
		mActualPosIndex = -1;
		mSpinHitCtrl = NULL;
		mBlowAttackedDistance = -1.0f;
		mIsHead = false;
	}

	void DharmaSamboParts::init(const JMapInfoIter &rIter) {
		mSpinHitCtrl = new SpinHitController(this);
		MR::addToAttributeGroupSearchTurtle(this);
		initNerve(&NrvDharmaSamboParts::NrvDoNothing::sInstance, 0);
	}

	void DharmaSamboParts::appear() {
		ModelObj::appear();
		mActualPosIndex = mOriginalPosIndex;
		setNerve(&NrvDharmaSamboParts::NrvDoNothing::sInstance);
	}

	void DharmaSamboParts::kill() {
		ModelObj::kill();
		MR::emitEffect(this, "Death");
		MR::startActionSound(this, "EmExplodeS", -1, -1, -1);

		if (mIsHead) {
			s32 numStarPiece = MR::getDeclareRemnantStarPieceCount(mHost);
			
			if (MR::appearStarPiece(mHost, mTranslation, numStarPiece, 10.0f, 40.0f, false)) {
				MR::startActionSound(mHost, "OjStarPieceBurst", -1, -1, -1);
			}
		}
	}

	void DharmaSamboParts::control() {
		mGravity.set(mHost->mGravity);
	}

	void DharmaSamboParts::calcAndSetBaseMtx() {
		ModelObj::calcAndSetBaseMtx();

		if (mIsHead && mHost->mRemainingParts == 1 && mHost->isNerve(&NrvDharmaSambo::NrvStarPointerBind::sInstance)) {
			TVec3f scale(mHost->mAnimScaleCtrl->mScale);
			scale.mul(mScale);
			MR::setBaseScale(this, scale);
		}
		else {
			MR::setBaseScale(this, TVec3f(1.0f, 1.0f, 1.0f));
		}
	}

	void DharmaSamboParts::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (!MR::isSensorPlayer(pReceiver)) {
			return;
		}

		if (mIsHead) {
			if (mHost->isNerve(&NrvDharmaSambo::NrvEscapeRun::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvEscapeLand::sInstance))
			{
				if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)
					|| MR::isPlayerHipDropFalling()
					|| MR::isPlayerHipDropLand())
				{
					MR::sendMsgPush(pReceiver, pSender);
				}
				else if (!MR::sendMsgEnemyAttack(pReceiver, pSender)) {
					MR::sendMsgPush(pReceiver, pSender);
				}
			}
			else if (mHost->isNerve(&NrvDharmaSambo::NrvWaitUnderGround::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvHide::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvAppear::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvAppearImmediately::sInstance))
			{
				MR::sendMsgPush(pReceiver, pSender);
			}
			else {
				if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)
					|| MR::isPlayerHipDropFalling()
					|| MR::isPlayerHipDropLand())
				{
					MR::sendMsgPush(pReceiver, pSender);
				}
				else if (mHost->isNerve(&NrvDharmaSambo::NrvAttack::sInstance)) {
					if (!MR::sendMsgEnemyAttack(pReceiver, pSender)) {
						MR::sendMsgPush(pReceiver, pSender);
					}
				}
				else {
					MR::sendMsgPush(pReceiver, pSender);
				}
			}
		}
		else {
			if (mHost->isNerve(&NrvDharmaSambo::NrvSpinHitted::sInstance) && !MR::isLessStep(this, 30)
				|| mHost->isNerve(&NrvDharmaSambo::NrvWaitUnderGround::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvStarPointerBind::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvAppearImmediately::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvHide::sInstance)
				|| mHost->isNerve(&NrvDharmaSambo::NrvHideAppear::sInstance)
				|| MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)
				|| MR::isPlayerHipDropFalling()
				|| MR::isPlayerHipDropLand())
			{
				MR::sendMsgPush(pReceiver, pSender);
			}
			else if (!MR::sendMsgEnemyAttack(pReceiver, pSender)) {
				MR::sendMsgPush(pReceiver, pSender);
			}
		}
	}

	bool DharmaSamboParts::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (mHost->isNerve(&NrvDharmaSambo::NrvKilled::sInstance)
			|| mHost->isNerve(&NrvDharmaSambo::NrvTrampleDeath::sInstance))
		{
			return false;
		}
		else if (MR::isMsgStarPieceReflect(msg)) {
			return true;
		}
		else if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
			if (!mIsHead) {
				MR::startActionSound(this, "EmDharmasamboTrample", -1, -1, -1);
				return true;
			}
			else {
				if (mHost->isNerve(&NrvDharmaSambo::NrvWaitUnderGround::sInstance)
					|| mHost->isNerve(&NrvDharmaSambo::NrvHideAppear::sInstance))
				{
					mHost->setNerve(&NrvDharmaSambo::NrvAppearImmediately::sInstance);
					MR::sendMsgAwayJump(pSender, pReceiver);
					return true;
				}
				else if (!mHost->isNerve(&NrvDharmaSambo::NrvAppear::sInstance)
					&& !mHost->isNerve(&NrvDharmaSambo::NrvAppearImmediately::sInstance)
					&& !mHost->isNerve(&NrvDharmaSambo::NrvHide::sInstance))
				{
					mHost->setNerve(&NrvDharmaSambo::NrvTrampleDeath::sInstance);
					MR::startActionSound(this, "EmDharmasamboTrample", -1, -1, -1);
					return true;
				}
				else {
					return true;
				}
			}
		}
		else if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)
			|| MR::isMsgJetTurtleAttack(msg)
			|| MR::isMsgYoshiProjectileOrEat(msg)
			|| MR::isMsgFireBallAttack(msg)
			|| MR::isMsgInvincibleAttack(msg)
			|| MR::isMsgDrillAttack(msg)
			|| MR::isMsgRockAttack(msg)
			|| MR::isMsgSnowBallAttack(msg))
		{
			TVec3f dirToSender(pSender->mPosition - pReceiver->mPosition);
			mBlowAttackedDistance = PSVECMag(dirToSender);
			mHost->mBlowAttackerSensor = pSender;

			// This is needed in order to make the shell break
			return MR::isMsgJetTurtleAttack(msg);
		}

		return false;
	}

	void DharmaSamboParts::exeBlow() {
		if (MR::isFirstStep(this)) {
			mSpinHitCtrl->start(this, *MR::getPlayerPos(), mTranslation);
			MR::startAction(this, "Blow");
			MR::startBlowHitSound(mHost);
			MR::startActionSound(this, "EmDharmasamboBlow", -1, -1, -1);
		}

		mSpinHitCtrl->execute(this);
	}

	void DharmaSamboParts::exeStarPointerBind() {
		if (MR::isFirstStep(this)) {
			MR::setBckRate(this, 0.0f);
			MR::emitEffect(this, "Touch");
		}
	}

	void DharmaSamboParts::endStarPointerBind() {
		MR::deleteEffect(this, "Touch");
		MR::setBckRate(this, 1.0f);
	}

	void DharmaSamboParts::setHead(bool isHead) {
		mIsHead = isHead;

		if (isHead) {
			MR::hideMaterial(this, "SamboMatBody_v");
			MR::showMaterial(this, "SamboMatFace_v");
			MR::showMaterial(this, "SamboMatEye_v");
			MR::showMaterial(this, "FlowerMat_v");
		}
		else {
			MR::showMaterial(this, "SamboMatBody_v");
			MR::hideMaterial(this, "SamboMatFace_v");
			MR::hideMaterial(this, "SamboMatEye_v");
			MR::hideMaterial(this, "FlowerMat_v");
		}
	}

	namespace NrvDharmaSambo {
		void NrvWaitUnderGround::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeWaitUnderGround();
		}

		void NrvAppear::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvAppearImmediately::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvPursue::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exePursue();
		}

		void NrvPursue::executeOnEnd(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			MR::deleteEffect(pActor, "WaitSmoke");
		}

		void NrvAttack::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeAttack();
		}

		void NrvHide::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeHide();
		}

		void NrvHideAppear::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeHideAppear();
		}

		void NrvSpinHitted::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeSpinHitted();
		}

		void NrvKilled::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeKilled();
		}

		void NrvStarPointerBind::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeStarPointerBind();
		}

		void NrvStarPointerBind::executeOnEnd(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->endStarPointerBind();
		}

		void NrvEscapeRun::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeEscapeRun();
		}

		void NrvEscapeLand::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeEscapeLand();
		}

		void NrvEscapeWaitRun::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeEscapeRun();
		}

		void NrvEscapeWaitLand::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeEscapeLand();
		}

		void NrvTrampleDeath::execute(Spine *pSpine) const {
			DharmaSambo *pActor = (DharmaSambo*)pSpine->mExecutor;
			pActor->exeTrampleDeath();
		}

		NrvWaitUnderGround(NrvWaitUnderGround::sInstance);
		NrvAppear(NrvAppear::sInstance);
		NrvAppearImmediately(NrvAppearImmediately::sInstance);
		NrvPursue(NrvPursue::sInstance);
		NrvAttack(NrvAttack::sInstance);
		NrvHide(NrvHide::sInstance);
		NrvHideAppear(NrvHideAppear::sInstance);
		NrvSpinHitted(NrvSpinHitted::sInstance);
		NrvKilled(NrvKilled::sInstance);
		NrvStarPointerBind(NrvStarPointerBind::sInstance);
		NrvEscapeRun(NrvEscapeRun::sInstance);
		NrvEscapeLand(NrvEscapeLand::sInstance);
		NrvEscapeWaitRun(NrvEscapeWaitRun::sInstance);
		NrvEscapeWaitLand(NrvEscapeWaitLand::sInstance);
		NrvTrampleDeath(NrvTrampleDeath::sInstance);
	}

	namespace NrvDharmaSamboParts {
		void NrvDoNothing::execute(Spine *pSpine) const {}

		void NrvBlow::execute(Spine *pSpine) const {
			DharmaSamboParts *pActor = (DharmaSamboParts*)pSpine->mExecutor;
			pActor->exeBlow();
		}

		void NrvStarPointerBind::execute(Spine *pSpine) const {
			DharmaSamboParts *pActor = (DharmaSamboParts*)pSpine->mExecutor;
			pActor->exeStarPointerBind();
		}

		void NrvStarPointerBind::executeOnEnd(Spine *pSpine) const {
			DharmaSamboParts *pActor = (DharmaSamboParts*)pSpine->mExecutor;
			pActor->endStarPointerBind();
		}

		NrvDoNothing(NrvDoNothing::sInstance);
		NrvBlow(NrvBlow::sInstance);
		NrvStarPointerBind(NrvStarPointerBind::sInstance);
	}
}
