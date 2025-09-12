#include "Poihana.h"

/*
* Authors: Aurum, Someone
* Objects: Poihana
* Parameters:
*  - Obj_arg0, float, 1000.0f: Launch intensity
*  - Obj_arg1, float, 3500.0f: Active range
*  - Obj_arg2, int, 0: Color
*   *: Blue
*   1: Red
*   2: Green
*   3: Purple
*  - Obj_arg3, long, 0: Behavior
*   *: Walk around
*   1: Sleeping
*   2: Walk around and reset home
*  - Obj_arg4, bool, false: Can drown?
*  - Obj_arg7, bool, false: Use small binder
*  - SW_AWAKE
* 
* By far one of the most ambitious objects in this pack and presumably one of the most requested ones. The Cataquacks
* from Gold Leaf Galaxy can be used in SMG2 now!
*/

namespace pt {
	Poihana::Poihana(const char *pName) : LiveActor(pName) {
		mAnimScaleCtrl = NULL;
		mBindStarPointer = NULL;
		mCamInfo = NULL;
		mBindedActor = NULL;
		mLaunchIntensity = 50.0f;
		mActiveRange = 3500.0f;
		mBoundTimer = 0;
		mRandDir = 0;
		mBehavior = POIHANA_BEHAVIOR_NORMAL;
		mCanDrown = false;
		mIsActive = false;
		mWaterColumn = NULL;
	}

	void Poihana::init(const JMapInfoIter &rIter) {
		MR::initDefaultPos(this, rIter);
		MR::processInitFunction(this, rIter, false);
		MR::connectToSceneEnemy(this);

		// Initialize sensors
		initHitSensor(2);
		MR::addHitSensorPriorBinder(this, "Binder", 8, 125.0f, TVec3f(0.0f, 130.0f, 120.0f));
		MR::addHitSensorAtJoint(this, "Body", "Body", ATYPE_KILLER_TARGET_ENEMY, 8, 70.0f, TVec3f(0.0f, 0.0f, 0.0f));

		// Initialize binder
		bool useSmallBinder = false;
		MR::getJMapInfoArg7NoInit(rIter, &useSmallBinder);

		if (useSmallBinder) {
			initBinder(100.0f, 100.0f, 0);
		}
		else {
			initBinder(150.0f, 150.0f, 0);
		}

		mIsActive = true;
		MR::initActorCamera(this, rIter, &mCamInfo);
		MR::setGroupClipping(this, rIter, 0x20);
		initEffectKeeper(1, NULL, false);
		initSound(4, "Poihana", false, TVec3f(0.0f, 0.0f, 0.0f));

		// Initialize 2P behavior
		MR::initStarPointerTarget(this, 150.0f, TVec3f(0.0f, 100.0f, 0.0f));
		mAnimScaleCtrl = new AnimScaleController(NULL);
		mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);

		MR::initShadowVolumeSphere(this, 80.0f);
		MR::addToAttributeGroupSearchTurtle(this);

		mRespawnPos.set(mTranslation);
		mHomePos.set(mTranslation);

		// Calculate launch intensity, thanks to Shibbo for helping me on that one
		f32 launchIntensity = 1000.0f;
		MR::getJMapInfoArg0NoInit(rIter, &launchIntensity);
		launchIntensity *= 2.0;

		if (launchIntensity > 0.0f) {
			f32 reciprocal = __frsqrte(launchIntensity);
			f32 factor = reciprocal * launchIntensity;
			launchIntensity = -(factor * reciprocal - 3.0f) * factor * 0.5f;
		}

		mLaunchIntensity = -launchIntensity;

		// Setup color
		s32 color = 0;
		MR::getJMapInfoArg2NoInit(rIter, &color);
		MR::startBtpAndSetFrameAndStop(this, "ColorChange", color);

		// Setup behaviors
		MR::getJMapInfoArg1NoInit(rIter, &mActiveRange);
		MR::getJMapInfoArg3NoInit(rIter, &mBehavior);
		MR::getJMapInfoArg4NoInit(rIter, &mCanDrown);

		if (mCanDrown) {
			mWaterColumn = MR::createModelObjMapObj("エフェクト水柱", "WaterColumn", (MtxPtr)getBaseMtx());
			mWaterColumn->mScale.setAll(2.0f);
		}

		MR::tryCreateMirrorActor(this, NULL);
		MR::useStageSwitchAwake(this, rIter);

		if (mBehavior == POIHANA_BEHAVIOR_SLEEP) {
			initNerve(&NrvPoihana::NrvSleep::sInstance, 0);
		}
		else {
			initNerve(&NrvPoihana::NrvWait::sInstance, 0);
		}

		makeActorAppeared();
	}

	void Poihana::initAfterPlacement() {
		TMtx34f baseMtx;
		MR::makeMtxUpNoSupportPos((TPos3f*)&baseMtx, -mGravity, mTranslation);
		MR::setBaseTRMtx(this, *(TPos3f*)&baseMtx);
		MR::calcFrontVec(&mFrontVec, this);
		MR::trySetMoveLimitCollision(this);
	}

	void Poihana::control() {
		if (!isNerve(&NrvPoihana::NrvHide::sInstance)) {
			mAnimScaleCtrl->updateNerve();
			calcBound();
			controlVelocity();
			calcMyGravity();

			if (!tryDrown() && !tryDPDSwoon())
				tryHipDropShock();
		}
	}

	void Poihana::calcAndSetBaseMtx() {
		TMtx34f baseMtx;
		MR::calcMtxFromGravityAndZAxis((TPos3f*)&baseMtx, this, mGravity, mFrontVec);

		if (isNerveTypeWalkOrWait()) {
			MR::blendMtx((MtxPtr)getBaseMtx(), (MtxPtr)&baseMtx, 0.3f, (MtxPtr)&baseMtx);
		}

		MR::setBaseTRMtx(this, *(TPos3f*)&baseMtx);
		MR::updateBaseScale(this, mAnimScaleCtrl);
	}

	void Poihana::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorPlayer(pReceiver) || MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
			if (MR::isSensorEnemy(pSender)) {
				if (MR::isSensorPlayer(pReceiver)) {
					contactMario(pSender, pReceiver);
				}

				if (MR::sendMsgPush(pReceiver, pSender)) {
					if (MR::isSensorPlayer(pReceiver) && (isNerve(&NrvPoihana::NrvShock::sInstance)
						|| isNerve(&NrvPoihana::NrvSwoonLand::sInstance)
						|| isNerve(&NrvPoihana::NrvSwoon::sInstance)
						|| isNerve(&NrvPoihana::NrvRecover::sInstance)))
					{
						return;
					}

					TVec3f pushVelocity(mTranslation - pReceiver->mActor->mTranslation);
					MR::normalizeOrZero(&pushVelocity);

					if (mVelocity.dot(pushVelocity) < 0.0f) {
						f32 dot = pushVelocity.dot(mVelocity) * -1.0f;
						JMAVECScaleAdd(pushVelocity, mVelocity, mVelocity, dot);
					}
				}
			}
		}
	}

	bool Poihana::receiveMsgPush(HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
			TVec3f pushOffset(mTranslation - pSender->mActor->mTranslation);
			MR::normalizeOrZero(&pushOffset);
			JMAVECScaleAdd(pushOffset, mVelocity, mVelocity, 1.5f);

			return true;
		}

		return false;
	}

	bool Poihana::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgStarPieceAttack(msg)) {
			return true;
		}
		else if (MR::isMsgStarPieceReflect(msg)) {
			mAnimScaleCtrl->startHitReaction();
			return true;
		}

		if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
			if (isNerve(&NrvPoihana::NrvShock::sInstance)
				|| isNerve(&NrvPoihana::NrvSwoonLand::sInstance)
				|| isNerve(&NrvPoihana::NrvSwoon::sInstance)
				|| isNerve(&NrvPoihana::NrvRecover::sInstance)
				|| isNerve(&NrvPoihana::NrvSleepStart::sInstance)
				|| isNerve(&NrvPoihana::NrvSleep::sInstance)
				|| isNerve(&NrvPoihana::NrvGetUp::sInstance))
			{
				startBound();
				MR::startActionSound(this, "EvPoihanaTrample", -1, -1, -1);

				if (MR::isMsgPlayerHipDrop(msg)) {
					MR::sendMsgAwayJump(pSender, pReceiver);
				}

				return true;
			}
			else {
				updateTrampleBinderSensor();
			}
		}

		if (MR::isMsgPlayerHitAll(msg) && tryShock()) {
			MR::stopSceneForDefaultHit(3);
			return true;
		}

		return false;
	}

	bool Poihana::receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		return MR::isMsgExplosionAttack(msg) && tryShock();
	}

	bool Poihana::receiveOtherMsg(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgAutoRushBegin(msg) && MR::isSensorPlayer(pSender)) {
			if (isNerve(&NrvPoihana::NrvShootUpCharge::sInstance)) {
				return false;
			}
			else if (MR::isOnGroundPlayer()) {
				if (isNerve(&NrvPoihana::NrvShootUp::sInstance)) {
					if (getNerveStep() < 30) {
						return tryToStartBind(pSender);
					}
				}
				else if (isNerve(&NrvPoihana::NrvShootUpCharge::sInstance) || !MR::isNear(pSender, pReceiver, 100.0f)) {
					return false;
				}
				else {
					setNerve(&NrvPoihana::NrvShootUpCharge::sInstance);
				}
			}
			else if (tryToStartBind(pSender)) {
				setNerve(&NrvPoihana::NrvShootUp::sInstance);
				return true;
			}
		}
		else if (MR::isMsgUpdateBaseMtx(msg) && mBindedActor) {
			updateBindActorMtx();
			return true;
		}

		return false;
	}

	void Poihana::exeNonActive() {
		if (MR::isFirstStep(this)) {
			mVelocity.zero();
			MR::offBind(this);
			MR::offCalcShadow(this, NULL);
			MR::offCalcAnim(this);

			mIsActive = false;

			MR::invalidateHitSensors(this);
		}

		if (MR::isNearPlayerAnyTime(this, mActiveRange)) {
			if (mBehavior == POIHANA_BEHAVIOR_SLEEP) {
				setNerve(&NrvPoihana::NrvSleep::sInstance);
			}
			else {
				setNerve(&NrvPoihana::NrvWait::sInstance);
			}
		}
	}

	void Poihana::endNonActive() {
		MR::onBind(this);
		MR::onCalcShadow(this, NULL);
		MR::onCalcAnim(this);

		mIsActive = true;

		updateBinderSensor();
		MR::validateHitSensor(this, "Binder");
		MR::validateHitSensors(this);
	}

	void Poihana::exeWait() {
		if (MR::isFirstStep(this)) {
			MR::startBckNoInterpole(this, "Wait");
			updateBinderSensor();
		}

		if (MR::isNearPlayer(this, 800.0f)) {
			setNerve(&NrvPoihana::NrvSearch::sInstance);
		}
		else if (MR::isGreaterStep(this, 180)) {
			setNerve(&NrvPoihana::NrvWalkAround::sInstance);
		}
		else {
			tryNonActive();
		}
	}

	void Poihana::exeWalkAround() {
		if (MR::isFirstStep(this)) {
			if (!MR::isBckPlaying(this, "Walk")) {
				MR::startAction(this, "Walk");
			}

			mRandDir = MR::getRandom((s32)-2, (s32)2);
		}

		MR::rotateVecDegree(&mFrontVec, mGravity, mRandDir);
		JMAVECScaleAdd(mFrontVec, mVelocity, mVelocity, 0.5f);

		if (isNeedForBackHome()) {
			setNerve(&NrvPoihana::NrvGoBack::sInstance);
		}

		else if (MR::isNearPlayer(this, 800.0f)) {
			setNerve(&NrvPoihana::NrvSearch::sInstance);
		}

		else if (MR::isGreaterStep(this, 120)) {
			if (mBehavior == POIHANA_BEHAVIOR_SLEEP) {
				setNerve(&NrvPoihana::NrvSleepStart::sInstance);
			}
			else {
				setNerve(&NrvPoihana::NrvWait::sInstance);
			}
		}
	}

	void Poihana::exeSleepStart() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "SleepStart");
			MR::startActionSound(this, "EvPoihanaSleepStart", -1, -1, -1);
			MR::invalidateHitSensor(this, "Binder");
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPoihana::NrvSleep::sInstance);
		}
	}

	void Poihana::exeSleep() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Sleep");
			MR::invalidateHitSensor(this, "Binder");
		}

		if (isNeedForGetUp()) {
			setNerve(&NrvPoihana::NrvGetUp::sInstance);
		}
		else {
			tryNonActive();
		}
	}

	void Poihana::exeGetUp() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "GetUp");
			MR::startActionSound(this, "EvPoihanaWakeup", -1, -1, -1);
			MR::startActionSound(this, "EmPoihanaWakeup", -1, -1, -1);
		}

		if (MR::isActionEnd(this)) {
			updateBinderSensor();
			setNerve(&NrvPoihana::NrvWait::sInstance);
		}
	}

	void Poihana::exeSearch() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Search");
			MR::startActionSound(this, "EvPoihanaFind", -1, -1, -1);
		}

		MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFrontVec, *MR::getPlayerPos(), 4.0f);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPoihana::NrvChasePlayer::sInstance);
		}
	}

	void Poihana::exeChasePlayer() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Run");
			updateBinderSensor();
		}

		MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFrontVec, *MR::getPlayerPos(), 4.0f);
		JMAVECScaleAdd(mFrontVec, mVelocity, mVelocity, 0.5f);

		if (isNeedForBackHome()) {
			setNerve(&NrvPoihana::NrvGoBack::sInstance);
		}
	}

	void Poihana::exeShootUpCharge() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "ThrowStart");
		}

		f32 dot = mGravity.dot(mVelocity);
		mVelocity.scale(dot, mGravity);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPoihana::NrvShootUp::sInstance);
		}
	}

	void Poihana::exeShootUp() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Throw");
			MR::startActionSound(this, "EvPoihanaShootUp", -1, -1, -1);
			MR::startActorCameraNoTarget(this, mCamInfo, -1);
			MR::invalidateClipping(this);
		}

		f32 dot = mGravity.dot(mVelocity);
		mVelocity.scale(dot, mGravity);

		if (MR::isStep(this, 2)) {
			endBind();
			MR::invalidateHitSensor(this, "Binder");
		}

		if (MR::isStep(this, 30)) {
			updateBinderSensor();
		}

		if (MR::isActionEnd(this)) {
			if (mBehavior == POIHANA_BEHAVIOR_NEW_HOME) {
				mHomePos.set(mTranslation);
				setNerve(&NrvPoihana::NrvWalkAround::sInstance);
			}
			else {
				setNerve(&NrvPoihana::NrvGoBack::sInstance);
			}
		}
	}

	void Poihana::endShootUp() {
		endBind();
		updateBinderSensor();

		MR::endActorCamera(this, mCamInfo, true, -1);
        MR::validateClipping(this);
	}

	void Poihana::exeGoBack() {
		if (MR::isFirstStep(this) && !MR::isBckPlaying(this, "Walk")) {
			MR::startAction(this, "Walk");
		}

		MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFrontVec, mHomePos, 2.0f);
		JMAVECScaleAdd(mFrontVec, mVelocity, mVelocity, 0.5f);

		if (MR::isNearPlayer(this, 800.0f) && MR::isGreaterStep(this, 120)) {
			setNerve(&NrvPoihana::NrvSearch::sInstance);
		}
		else if (MR::isNear(this, mHomePos, 100.0f)) {
			setNerve(&NrvPoihana::NrvWait::sInstance);
		}
	}

	void Poihana::exeShock() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "PunchDamage");
			MR::startBlowHitSound(this);
			MR::invalidateHitSensor(this, "Binder");
		}

		if (MR::isOnGround(this) && MR::isGreaterStep(this, 12)) {
			setNerve(&NrvPoihana::NrvSwoon::sInstance);
		}
	}

	void Poihana::exeSwoonLand() {
		if (MR::isFirstStep(this)) {
			MR::startBckNoInterpole(this, "SwoonLand");
			MR::startActionSound(this, "EvPoihanaSwoon", -1, -1, -1);
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPoihana::NrvSwoon::sInstance);
		}
	}

	void Poihana::exeSwoon() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Swoon");
		}

		MR::startActionSound(this, "EmLvSwoonS", -1, -1, -1);

		if (MR::isStep(this, 110)) {
			setNerve(&NrvPoihana::NrvRecover::sInstance);
		}
	}

	void Poihana::exeRecover() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Recover");
			MR::startActionSound(this, "EmPoihanaRecover", -1, -1, -1);
			MR::startActionSound(this, "EvPoihanaRecover", -1, -1, -1);
			mScale.setAll(1.0f);
		}

		if (MR::isActionEnd(this)) {
			updateBinderSensor();
			setNerve(&NrvPoihana::NrvWait::sInstance);
		}
	}

	// Needs to be reviewed
	void Poihana::exeShake() {
		s32 step = getNerveStep();
		f32 _f31 = 0.2f - 0.01f * step;
		f32 scale = sin(36.0f * step * 0.01745329251f) * _f31 + 1.0f;
		mScale.setAll(scale);

		if (MR::isStep(this, 20)) {
			mScale.setAll(1.0f);
			setNerve(&NrvPoihana::NrvSearch::sInstance);
		}
	}

	void Poihana::exeDrown() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Drown");
			MR::invalidateClipping(this);
			MR::invalidateHitSensors(this);
			MR::startActionSound(this, "EmFallIntoWaterS", -1, -1, -1);

			mWaterColumn->appear();
			MR::tryStartAllAnim(mWaterColumn, "Splash");
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvPoihana::NrvHide::sInstance);
		}
	}

	void Poihana::exeHide() {
		if (MR::isFirstStep(this)) {
			MR::startActionSound(this, "EmExplodeSWater", -1, -1, -1);
			MR::emitEffect(this, "DeathWater");
			MR::hideModel(this);
		}

		if (MR::isStep(this, 150)) {
			setNerve(&NrvPoihana::NrvAppear::sInstance);
		}
	}

	void Poihana::exeAppear() {
		if (MR::isFirstStep(this)) {
			MR::resetPosition(this, mRespawnPos);
			MR::emitEffect(this, "Appear");
		}

		if (MR::isLessStep(this, 60)) {
			MR::startActionSound(this, "EmLvPoihanaReviveEffect", -1, -1, -1);
		}

		if (MR::isStep(this, 60)) {
			MR::showModel(this);
			MR::startAction(this, "Appear");
			MR::startActionSound(this, "EmPoihanaReviveAppear", -1, -1, -1);
		}
		else if (MR::isActionEnd(this) && MR::isGreaterStep(this, 60)) {
			MR::validateClipping(this);
			MR::validateHitSensors(this);
			updateBinderSensor();
			setNerve(&NrvPoihana::NrvWait::sInstance);
		}
	}

	void Poihana::exeDPDSwoon() {
		if (MR::isFirstStep(this)) {
			MR::invalidateHitSensor(this, "Binder");
			mAnimScaleCtrl->startDpdHitVibration();
		}

		MR::startDPDFreezeLevelSound(this);
		MR::updateActorStateAndNextNerve(this, (ActorStateBaseInterface*)mBindStarPointer, &NrvPoihana::NrvWait::sInstance);
	}

	void Poihana::endDPDSwoon() {
		mBindStarPointer->kill();
		updateBinderSensor();
	}

	void Poihana::updateBinderSensor() {
		HitSensor *senBinder = getSensor("Binder");
		senBinder->mPosition.set(0.0f, 130.0f, 120.0f);
		senBinder->mRadius = 125.0f;
		senBinder->validate();
	}

	void Poihana::updateTrampleBinderSensor() {
		HitSensor *senBinder = getSensor("Binder");
		senBinder->mPosition.set(0.0f, 150.0f, 0.0f);
		senBinder->mRadius = 200.0f;
		senBinder->validate();
	}

	bool Poihana::tryToStartBind(HitSensor* pSender) {
		if (mBindedActor == NULL && !MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
			MR::tryRumblePadMiddle(this, 0);
			mBindedActor = pSender->mActor;
			MR::startBckPlayer("Rise", NULL);
			return true;
		}

		return false;
	}

	void Poihana::updateBindActorMtx() {
		TMtx34f binderMtx;
		MR::makeMtxTR((MtxPtr)&binderMtx, mBindedActor);
		MR::setBaseTRMtx(mBindedActor, (MtxPtr)&binderMtx);
	}

	void Poihana::endBind() {
		if (MR::isPlayerInRush()) {
			TVec3f jumpVec;
			jumpVec.scale(mLaunchIntensity, mGravity);
			MR::endBindAndPlayerJump(this, jumpVec, 0);
			MR::startActionSound(this, "PmHeliJump", -1, -1, -1);
		}

		mBindedActor = NULL;
	}

	void Poihana::startBound() {
		mBoundTimer = 0;
		mScale.setAll(1.0f);
	}

	/*
	* This function calculates Poihana's scale for 40 frames after being trampled. This is
	* used to simulate the "vibrating" visual effect. This is not 1:1 the same as in SMG1,
	* but it looks VERY similar and appears to be even more efficient compared to SMG1's
	* unusually complicated calculations.
	*/
	void Poihana::calcBound() {
		if (mBoundTimer != -1) {
			f32 scale = 1.0f;

			if (mBoundTimer < 40) {
				scale = 0.05f * sin(0.393f * mBoundTimer) + 1.0f;
				mBoundTimer++;
			}
			else {
				mBoundTimer = -1;
			}

			mScale.setAll(scale);
		}
	}

	void Poihana::contactMario(HitSensor *pSender, HitSensor *pReceiver) {
		if (isNerveTypeWalkOrWait() || isNerve(&NrvPoihana::NrvShootUpCharge::sInstance) || isNerve(&NrvPoihana::NrvShootUp::sInstance)) {
			if (isBackAttack(pReceiver) && MR::sendMsgEnemyAttackFlipWeak(pReceiver, pSender)) {
				setNerve(&NrvPoihana::NrvShake::sInstance);
			}
		}
		else if (isNerve(&NrvPoihana::NrvSleep::sInstance)) {
			setNerve(&NrvPoihana::NrvGetUp::sInstance);
		}
	}

	// Needs review
	void Poihana::controlVelocity() {
		if (!mIsActive)
			return;

		// Calculate front vector
		TVec3f gravity;
		if (MR::isBindedGround(this)) {
			gravity.set(-MR::getGroundNormal(this));
		}
		else {
			gravity.set(mGravity);
		}

		TVec3f frontVec(mFrontVec);
		MR::turnVecToPlane(&mFrontVec, frontVec, gravity);

		// Calculate velocity
		if (MR::isBindedGround(this)) {
			f32 dot = mFrontVec.dot(mVelocity) * -1.0f;

			TVec3f addVel;
			JMAVECScaleAdd(mFrontVec, mVelocity, addVel, dot);
			addVel.scale(0.8f);

			mVelocity.scale(mFrontVec.dot(mVelocity), mFrontVec);
			mVelocity.add(addVel);

			if (mVelocity.dot(gravity) > 0.0f) {
				dot = gravity.dot(mVelocity) * -1.0f;
				JMAVECScaleAdd(gravity, mVelocity, mVelocity, dot);
			}

			mVelocity.scale(0.95f);
		}

		JMAVECScaleAdd(gravity, mVelocity, mVelocity, 2.0f);

		if (!isNerve(&NrvPoihana::NrvShock::sInstance)) {
			f32 magVel = isNerve(&NrvPoihana::NrvChasePlayer::sInstance) ? 10.0f : 5.0f;

			if (PSVECMag(mVelocity) > magVel) {
				f32 squared = mVelocity.squared();

				if (squared > 0.0000038146973f) {
					mVelocity.scale(JGeometry::TUtil<f32>::inv_sqrt(squared));
				}
			}

			if (MR::isNearZero(mVelocity, 0.001f)) {
				mVelocity.zero();
			}
		}
	}

	void Poihana::calcMyGravity() {
		if (!mIsActive)
			return;

		TVec3f upVec, gravityPos;
		MR::calcUpVec(&upVec, this);
		JMAVECScaleAdd(upVec, mTranslation, gravityPos, 20.0f);
		MR::calcGravity(this, gravityPos);
	}

	bool Poihana::tryNonActive() {
		if (MR::isStageStateScenarioOpeningCamera() || MR::isNearPlayerAnyTime(this, mActiveRange) || !MR::isBindedGround(this)) {
			return false;
		}

		setNerve(&NrvPoihana::NrvNonActive::sInstance);
		return true;
	}

	bool Poihana::tryDrown() {
		if (!mCanDrown) {
			return false;
		}

		if (isNerve(&NrvPoihana::NrvAppear::sInstance) || isNerve(&NrvPoihana::NrvDrown::sInstance)) {
			return false;
		}

		if (!MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
			return false;
		}

		setNerve(&NrvPoihana::NrvDrown::sInstance);
		return true;
	}

	bool Poihana::tryDPDSwoon() {
		if (isNerveTypeWalkOrWait() && mBindStarPointer->tryStartPointBind()) {
			setNerve(&NrvPoihana::NrvDPDSwoon::sInstance);
			return true;
		}

		return false;
	}

	bool Poihana::tryShock() {
		if (isNerve(&NrvPoihana::NrvDrown::sInstance)
			|| isNerve(&NrvPoihana::NrvHide::sInstance)
			|| isNerve(&NrvPoihana::NrvAppear::sInstance)
			|| isNerve(&NrvPoihana::NrvShock::sInstance)
			|| isNerve(&NrvPoihana::NrvSwoonLand::sInstance))
		{
			return false;
		}

		mVelocity.scale(-30.0f, mGravity);
		setNerve(&NrvPoihana::NrvShock::sInstance);
		return true;
	}

	bool Poihana::tryHipDropShock() {
		if (!MR::isNearPlayerAnyTime(this, 500.0f)) {
			return false;
		}

		if (MR::isPlayerHipDropLand()) {
			return tryShock();
		}

		return false;
	}

	bool Poihana::isNerveTypeWalkOrWait() const {
		return (isNerve(&NrvPoihana::NrvWait::sInstance)
			|| isNerve(&NrvPoihana::NrvSearch::sInstance)
			|| isNerve(&NrvPoihana::NrvWalkAround::sInstance)
			|| isNerve(&NrvPoihana::NrvChasePlayer::sInstance)
			|| isNerve(&NrvPoihana::NrvGoBack::sInstance));
	}

	bool Poihana::isNeedForBackHome() const {
		if (isNerve(&NrvPoihana::NrvWalkAround::sInstance)) {
			return MR::isNear(this, mHomePos, 350.0f) == 0;
		}

		if (!isNerve(&NrvPoihana::NrvChasePlayer::sInstance)) {
			return false;
		}

		if (mBehavior == POIHANA_BEHAVIOR_NEW_HOME) {
			return !MR::isNearPlayer(this, 1100.0f);
		}

		if (!MR::isNear(this, mHomePos, 2000.0f) || !MR::isNearPlayer(this, 1110.0f)) {
			return true;
		}

		return false;
	}

	bool Poihana::isNeedForGetUp() const {
		if (MR::isNearPlayer(this, 500.0f)) {
			f32 mag = PSVECMag(*MR::getPlayerVelocity());

			if (mag < 10.0f && !MR::isPlayerSwingAction()) {
				return false;
			}

			return true;
		}

		return false;
	}

	bool Poihana::isBackAttack(HitSensor *pMySensor) const {
		TVec3f frontVec;
		MR::calcFrontVec(&frontVec, this);
		JGeometry::negateInternal(frontVec, frontVec);

		TVec3f sensorRelative(pMySensor->mPosition - mTranslation);
		return sensorRelative.dot(frontVec) > 0.0f;
	}

	namespace NrvPoihana {
		void NrvNonActive::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeNonActive();
		}

		void NrvNonActive::executeOnEnd(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->endNonActive();
		}

		void NrvWait::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvWalkAround::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeWalkAround();
		}

		void NrvSleepStart::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeSleepStart();
		}

		void NrvSleep::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeSleep();
		}

		void NrvGetUp::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeGetUp();
		}

		void NrvSearch::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeSearch();
		}

		void NrvChasePlayer::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeChasePlayer();
		}

		void NrvShootUpCharge::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeShootUpCharge();
		}

		void NrvShootUp::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeShootUp();
		}

		void NrvShootUp::executeOnEnd(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->endShootUp();
		}

		void NrvGoBack::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeGoBack();
		}

		void NrvShock::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeShock();
		}

		void NrvSwoon::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeSwoon();
		}

		void NrvSwoonLand::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeSwoonLand();
		}

		void NrvRecover::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeRecover();
		}

		void NrvShake::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeShake();
		}

		void NrvDrown::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeDrown();
		}

		void NrvHide::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeHide();
		}

		void NrvAppear::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvDPDSwoon::execute(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->exeDPDSwoon();
		}

		void NrvDPDSwoon::executeOnEnd(Spine *pSpine) const {
			Poihana *pActor = (Poihana*)pSpine->mExecutor;
			pActor->endDPDSwoon();
		}

		NrvNonActive(NrvNonActive::sInstance);
		NrvWait(NrvWait::sInstance);
		NrvWalkAround(NrvWalkAround::sInstance);
		NrvSleepStart(NrvSleepStart::sInstance);
		NrvSleep(NrvSleep::sInstance);
		NrvGetUp(NrvGetUp::sInstance);
		NrvSearch(NrvSearch::sInstance);
		NrvChasePlayer(NrvChasePlayer::sInstance);
		NrvShootUpCharge(NrvShootUpCharge::sInstance);
		NrvShootUp(NrvShootUp::sInstance);
		NrvGoBack(NrvGoBack::sInstance);
		NrvShock(NrvShock::sInstance);
		NrvSwoon(NrvSwoon::sInstance);
		NrvSwoonLand(NrvSwoonLand::sInstance);
		NrvRecover(NrvRecover::sInstance);
		NrvShake(NrvShake::sInstance);
		NrvDrown(NrvDrown::sInstance);
		NrvHide(NrvHide::sInstance);
		NrvAppear(NrvAppear::sInstance);
		NrvDPDSwoon(NrvDPDSwoon::sInstance);
	}
}
