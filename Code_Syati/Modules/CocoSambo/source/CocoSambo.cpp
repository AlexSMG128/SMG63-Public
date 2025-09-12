#include "CocoSambo.h"

/*
* Author: Aurum
* Objects: CocoSambo
* Parameters:
* - Obj_arg0, float, 1800.0f: Appearance range
* - Obj_arg1, bool, false: Disable items
* - Obj_arg2, bool, false: Show flower in ground
* - Obj_arg3, float, -100.0f: Hitback velocity
* - SW_AWAKE
* - SW_DEAD, use, write: Activated when killed
* 
* A stationary and thorny Pokey that attacks the player by smacking its body and head. It can be
* defeated using a wide range of attacks such as coconuts, shells, various projectiles and more.
*/

namespace {
	const char* sSensorTargetJoint[4] = { "Spine1", "Spine2", "Spine3", "Head" };
}

namespace pt {
	extern AnimScaleController* createSamboAnimScaleController(AnimScaleParam* pAnimScaleParam);
	
	CocoSamboHead::CocoSamboHead(LiveActor *pHost) : PartsModel(pHost, "ココサンボ[頭]", "CocoSamboHead", NULL, 18, false) {
		mFrontVec.set(0.0f, 0.0f, 1.0f);
		mHitBackVelocity = -100.0f;
	}

	void CocoSamboHead::init(const JMapInfoIter &rIter) {
		// Initialize sensors
		TVec3f sensorOffset(60.0f * mScale.x, 0.0f, 0.0f);
		initHitSensor(2);
		MR::addHitSensorAtJoint(this, "Body", "Head", ATYPE_SAMBO_HEAD, 8, 100.0f * mScale.x, sensorOffset);
		MR::addHitSensorAtJoint(this, "Trample", "Head", ATYPE_SAMBO_HEAD, 8, 150.0f * mScale.x, sensorOffset);

		// Initialize miscellaneous attributes
		initEffectKeeper(8, NULL, false);
		initSound(8, "CocoSambo", NULL, TVec3f(0.0f, 0.0f, 0.0f));
		MR::invalidateClipping(this);

		initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, -90.0f, -90.0f), "Head");

		// Setup nerve and make appeared
		initNerve(&NrvCocoSamboHead::NrvHeadConnectedBody::sInstance, 0);
		makeActorAppeared();
	}

	void CocoSamboHead::kill() {
		MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
		LiveActor::kill();
		MR::emitEffect(this, "DeathSmokeHead");
	}

	void CocoSamboHead::calcAndSetBaseMtx() {
		if (mCalcOwnMatrix) {
			PartsModel::calcAndSetBaseMtx();
		}
		else {
			TVec3f gravity;
			MR::calcGravityVector(this, &gravity, NULL, 0);
			gravity.negate();

			TMtx34f baseMtx;

			if (MR::isSameDirection(mFrontVec, gravity, 0.01f)) {
				MR::makeMtxUpNoSupportPos((TPos3f*)&baseMtx, gravity, mTranslation);
			}
			else {
				MR::makeMtxUpFrontPos((TPos3f*)&baseMtx, gravity, mFrontVec, mTranslation);
			}

			MR::setBaseTRMtx(this, *(TPos3f*)&baseMtx);
		}
	}

	void CocoSamboHead::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (pSender == getSensor("Body")
			&& !mHost->isNerve(&NrvCocoSambo::NrvPressY::sInstance)
			&& !mHost->isNerve(&NrvCocoSambo::NrvBlow::sInstance)
			&& !mHost->isNerve(&NrvCocoSambo::NrvDpdPointing::sInstance))
		{
			if (MR::isSensorPlayer(pReceiver)) {
				if (!mHost->isNerve(&NrvCocoSambo::NrvFallHead::sInstance)
					&& !mHost->isNerve(&NrvCocoSambo::NrvFallHeadHide::sInstance)
					&& !mHost->isNerve(&NrvCocoSambo::NrvRecoverWait::sInstance))
				{
					if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
						MR::sendMsgPush(pReceiver, pSender);
						return;
					}
				}

				if (mHost->isNerve(&NrvCocoSambo::NrvAppear::sInstance)) {
					MR::sendMsgJump(pReceiver, pSender);
				}
				else {
					MR::sendMsgPush(pReceiver, pSender);
				}
			}
			else if (MR::isSensorType(pReceiver, ATYPE_KURIBO)) {
				if (mHost->isNerve(&NrvCocoSambo::NrvAttack::sInstance)) {
					if (MR::sendMsgToEnemyAttackTrample(pReceiver, pSender)) {
						return;
					}
				}

				MR::sendMsgPush(pReceiver, pSender);
			}
			else if (MR::isSensorType(pReceiver, ATYPE_COCO_NUT)) {
				MR::sendMsgPush(pReceiver, pSender);
			}
		}
	}

	bool CocoSamboHead::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (isNerve(&NrvCocoSamboHead::NrvHeadFallLand::sInstance)
			|| isNerve(&NrvCocoSamboHead::NrvHeadSwoon::sInstance)
			|| (isNerve(&NrvCocoSamboHead::NrvHeadSwoonEnd::sInstance) && MR::isLessStep(this, 60)))
		{
			if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
				return ((CocoSambo*)mHost)->tryToPress();
			}

			if (MR::isMsgPlayerHitAll(msg)) {
				return ((CocoSambo*)mHost)->tryToBlow();
			}
		}

		if (MR::isMsgStarPieceReflect(msg)) {
			return ((CocoSambo*)mHost)->tryStarPieceReflect();
		}

		return false;
	}

	bool CocoSamboHead::receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorType(pSender, ATYPE_COCO_NUT)
			|| MR::isSensorType(pSender, ATYPE_KURIBO)
			|| MR::isSensorType(pSender, ATYPE_BEGOMAN))
		{
			return ((CocoSambo*)mHost)->tryToFallHead(pReceiver, pSender);
		}

		return false;
	}

	void CocoSamboHead::updateFrontVecToPlayer(const TVec3f &rGravity) {
		TVec3f otherDir;
		JGeometry::negateInternal(rGravity, otherDir);

		TVec3f dirToPlayer = *MR::getPlayerPos() - mTranslation;

		if (MR::isNearZero(dirToPlayer, 0.001f) || MR::isSameDirection(otherDir, dirToPlayer, 0.01f)) {
			if (MR::getMaxAbsElementIndex(otherDir) == 2) {
				dirToPlayer.set(0.0f, 1.0f, 0.0f);
			}
			else {
				dirToPlayer.set(0.0f, 0.0f, 1.0f);
			}
		}

		MR::normalize(&otherDir);
		MR::vecKillElement(dirToPlayer, otherDir, &mFrontVec);
		MR::normalize(&mFrontVec);
	}

	void CocoSamboHead::exeFall() {
		TVec3f gravity(mHost->mGravity);
		TVec3f _80 = gravity * 2.0f;

		if (MR::isFirstStep(this)) {
			mFixedPos->copyTrans(&mTranslation);
			mRotation.zero();

			mCalcOwnMatrix = false;

			MR::startAction(this, "Fall");
			updateFrontVecToPlayer(gravity);

			TVec3f _74 = mHost->mTranslation - mTranslation;
			TVec3f upVec;
			JGeometry::negateInternal(gravity, upVec);

			MR::vecKillElement(_74, upVec, &_74);

			mVelocity = _74 * (1.0f / 45.0f) + ((upVec * upVec.dot(_74)) * 2.0f - ((_80 * 45.0f) * 45.0f)) * (1.0f / (90.0f));
		}

		if (MR::isStep(this, 45)) {
			mTranslation.set(mHost->mTranslation);
			mVelocity.zero();
			setNerve(&NrvCocoSamboHead::NrvHeadFallLand::sInstance);
		}
		else {
			mVelocity += _80;
		}
	}

	void CocoSamboHead::exeFallLand() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Land");
			MR::startActionSound(this, "EmSfsamboHeadLand", -1, -1, -1);
			MR::tryRumblePadWeak(this, 0);
			MR::shakeCameraWeak();
		}

		MR::setNerveAtBckStopped(this, &NrvCocoSamboHead::NrvHeadSwoon::sInstance);
	}

	void CocoSamboHead::exeSwoon() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Swoon");
		}

		if (MR::isStep(this, 180)) {
			setNerve(&NrvCocoSamboHead::NrvHeadSwoonEnd::sInstance);
		}
	}

	void CocoSamboHead::exeSwoonEnd() {
		if (MR::isFirstStep(this)) {
			mVelocity.zero();
			MR::startAction(this, "SwoonEnd");
		}

		if (MR::isStep(this, 95)) {
			MR::invalidateShadow(mHost, NULL);
			MR::tryRumblePadMiddle(this, 0);
			MR::shakeCameraNormal();
		}

		MR::setNerveAtBckStopped(this, &NrvCocoSamboHead::NrvHeadConnectedBody::sInstance);
	}

	void CocoSamboHead::exeBlow() {
		if (MR::isFirstStep(this)) {
			MR::calcGravity(this);
			updateFrontVecToPlayer(mGravity);

			TVec3f upVec;
			TMtx34f rotMtx;
			JGeometry::negateInternal(mGravity, upVec);
			PSMTXIdentity((MtxPtr)&rotMtx);
			MR::makeMtxUpFront((TPos3f*)&rotMtx, upVec, mFrontVec);

			TVec3f blowVelocity(0.0f, 20.0f, mHitBackVelocity);
			((TRot3f*)&rotMtx)->mult33(blowVelocity, mVelocity);

			MR::startAction(this, "Blow");
		}

		mVelocity.add(mGravity);
	}

	CocoSambo::CocoSambo(const char *pName) : LiveActor(pName) {
		mHead = NULL;
		mHitEffectPos.zero();
		mFrontVec.set(0.0f, 0.0f, 1.0f);
		mUpVec.set(0.0f, 1.0f, 0.0f);
		mAppearRange = 1800.0f;
		mAttackRange = 700.0f;
		mAnimScaleCtrl = NULL;
		mAfterDpdNerve = NULL;
		mHasItems = false; // negated in init
		mNoHideFlower = false;
	}

	void CocoSambo::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, "CocoSamboBody", false);

		// Initialize up and front vectors
		TMtx34f rotateMtx;
		MR::makeMtxRotate((MtxPtr)&rotateMtx, mRotation);
		MR::extractMtxYDir((MtxPtr)&rotateMtx, &mUpVec);
		MR::extractMtxZDir((MtxPtr)&rotateMtx, &mFrontVec);

		// Initialize sensors
		initHitSensor(3);
		
		TVec3f sensorOffset(40.0f * mScale.x, 0.0f, 0.0f);
		f32 sensorRadius = 80.0f * mScale.x;
		
		for (s32 i = 0; i < 3; i++) {
			const char* sensorAndJointName = sSensorTargetJoint[i];
			MtxPtr jointMtx = (MtxPtr)MR::getJointMtx(this, sensorAndJointName);
			MR::addHitSensorMtx(this, sensorAndJointName, ATYPE_SAMBO_BODY, 8, sensorRadius, jointMtx, sensorOffset);
		}

		// Initialize head
		mHead = new CocoSamboHead(this);
		mHead->initWithoutIter();
		MR::getJMapInfoArg3NoInit(rIter, &mHead->mHitBackVelocity);

		// Initialize 2P behavior
		mAnimScaleParam = new AnimScaleParam();
		mAnimScaleCtrl = pt::createSamboAnimScaleController(mAnimScaleParam);
		mHead->initActorStarPointerTarget(120.0f * mScale.x, NULL, (MtxPtr)MR::getJointMtx(this, "Head"), TVec3f(-50.0f, 40.0f, 0.0f));

		// Initialize miscellaneous attributes
		MR::setEffectHostSRT(this, "Hit", &mHitEffectPos, NULL, NULL);
		mAttackRange = 700.0f * mScale.x;

		MR::getJMapInfoArg0NoInit(rIter, &mAppearRange);
		MR::getJMapInfoArg2NoInit(rIter, &mNoHideFlower);
		MR::calcGravity(this);
		MR::addToAttributeGroupSearchTurtle(this);

		// Initialize items
		MR::getJMapInfoArg1NoInit(rIter, &mHasItems);
		mHasItems = !mHasItems;

		if (mHasItems) {
			MR::declareCoin(this, 1);
			MR::declareStarPiece(this, 5);
		}

		initNerve(&NrvCocoSambo::NrvHideWait::sInstance, 0);
		appear();
	}

	void CocoSambo::appear() {
		LiveActor::appear();
		setNerve(&NrvCocoSambo::NrvHideWait::sInstance);
	}

	void CocoSambo::kill() {
		mHead->kill();

		MR::invalidateShadow(this, NULL);
		emitDeath();

		LiveActor::kill();
	}

	void CocoSambo::control() {
		mAnimScaleCtrl->update();
	}

	void CocoSambo::calcAndSetBaseMtx() {
		TMtx34f baseMtx;
		MR::makeMtxUpFrontPos((TPos3f*)&baseMtx, mUpVec, mFrontVec, mTranslation);
		MR::setBaseTRMtx(this, (MtxPtr)&baseMtx);

		MR::updateBaseScale(this, mAnimScaleCtrl);
	}

	void CocoSambo::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (!(isNerve(&NrvCocoSambo::NrvPressY::sInstance)
			|| isNerve(&NrvCocoSambo::NrvBlow::sInstance)
			|| isNerve(&NrvCocoSambo::NrvFallHead::sInstance)
			|| isNerve(&NrvCocoSambo::NrvFallHeadHide::sInstance)
			|| isNerve(&NrvCocoSambo::NrvRecoverWait::sInstance)))
		{
			if (MR::isSensorPlayer(pReceiver)) {
				if (!isNerve(&NrvCocoSambo::NrvDpdPointing::sInstance)) {
					MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
				}
				MR::sendMsgPush(pReceiver, pSender);
			}
			else if (MR::isSensorEnemy(pReceiver)) {
				MR::sendMsgPush(pReceiver, pSender);
			}
		}
	}

	bool CocoSambo::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgStarPieceReflect(msg)) {
			return tryStarPieceReflect();
		}
		else if (MR::isMsgJetTurtleAttack(msg)
			|| MR::isMsgYoshiProjectileOrEat(msg)
			|| MR::isMsgFireBallAttack(msg)
			|| MR::isMsgInvincibleAttack(msg)
			|| MR::isMsgRockAttack(msg)
			|| MR::isMsgSnowBallAttack(msg))
		{
			return tryToFallHead(pReceiver, pSender);
		}

		return false;
	}

	bool CocoSambo::receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorType(pSender, ATYPE_COCO_NUT)
			|| MR::isSensorType(pSender, ATYPE_KURIBO)
			|| MR::isSensorType(pSender, ATYPE_BEGOMAN))
		{
			return tryToFallHead(pReceiver, pSender);
		}

		return false;
	}

	bool CocoSambo::tryToFallHead(const HitSensor *pSender, const HitSensor *pReceiver) {
		if (isNerve(&NrvCocoSambo::NrvAppear::sInstance)
			|| isNerve(&NrvCocoSambo::NrvWait::sInstance)
			|| isNerve(&NrvCocoSambo::NrvHide::sInstance)
			|| isNerve(&NrvCocoSambo::NrvAttack::sInstance)
			|| isNerve(&NrvCocoSambo::NrvAttackInterval::sInstance)
			|| isNerve(&NrvCocoSambo::NrvDpdPointing::sInstance))
		{
			MR::deleteEffectAll(this);
			JMAVECLerp(pSender->mPosition, pReceiver->mPosition, mHitEffectPos, 0.75f);
			setNerve(&NrvCocoSambo::NrvFallHead::sInstance);

			return true;
		}

		return false;
	}

	bool CocoSambo::tryToPress() {
		if (!isNerve(&NrvCocoSambo::NrvPressY::sInstance) && !isNerve(&NrvCocoSambo::NrvBlow::sInstance)) {
			MR::deleteEffectAll(this);
			setNerve(&NrvCocoSambo::NrvPressY::sInstance);

			return true;
		}

		return false;
	}

	bool CocoSambo::tryToBlow() {
		if (!isNerve(&NrvCocoSambo::NrvPressY::sInstance) && !isNerve(&NrvCocoSambo::NrvBlow::sInstance)) {
			MR::deleteEffectAll(this);
			setNerve(&NrvCocoSambo::NrvBlow::sInstance);

			return true;
		}

		return false;
	}

	bool CocoSambo::tryStarPieceReflect() {
		if (!isNerve(&NrvCocoSambo::NrvPressY::sInstance) && !isNerve(&NrvCocoSambo::NrvBlow::sInstance)) {
			if (!isNerve(&NrvCocoSambo::NrvDpdPointing::sInstance)) {
				mAnimScaleCtrl->startHitReaction();
			}

			return true;
		}

		return false;
	}

	void CocoSambo::dirToPlayer(f32 var) {
		TVec3f playerOffset = *MR::getPlayerPos() - mTranslation;

		if (!MR::isNearZero(playerOffset, 0.001f) && !MR::isSameDirection(mUpVec, playerOffset, 0.01f)) {
			MR::normalize(&playerOffset);

			TVec3f _8;
			MR::vecKillElement(playerOffset, mUpVec, &_8);
			MR::normalize(&_8);

			if (var < 0.0f)
				var = -var;

			MR::turnVecToVecCos(&mFrontVec, mFrontVec, _8, sin(var * 45.52f), mUpVec, 0.02f);
		}
	}

	bool CocoSambo::tryDpdPointing(const Nerve *pAfterDpdNerve) {
		if (isStarPointerPointing()) {
			mAfterDpdNerve = pAfterDpdNerve;
			setNerve(&NrvCocoSambo::NrvDpdPointing::sInstance);
			return true;
		}

		return false;
	}

	bool CocoSambo::isStarPointerPointing() {
		return MR::attachSupportTicoToTarget(this) || MR::attachSupportTicoToTarget(mHead);
	}

	void CocoSambo::emitDeath() {
		MR::emitEffect(this, "DeathSmokeSpine1");
		MR::emitEffect(this, "DeathSmokeSpine2");
		MR::emitEffect(this, "DeathSmokeSpine3");
		MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
	}

	void CocoSambo::exeHideWait() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "HideWait");

			if (mNoHideFlower) {
				MR::startAction(mHead, "Appear");
				MR::setBckFrameAndStop(mHead, 0.0f);
			}
			else {
				MR::startAction(mHead, "HideWait");
			}

			MR::invalidateShadowAll(this);
			MR::invalidateHitSensors(mHead);
		}

		if (MR::isNearPlayer(this, mAppearRange)) {
			setNerve(&NrvCocoSambo::NrvAppear::sInstance);
		}
	}

	void CocoSambo::exeAppear() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Appear");
			MR::startAction(mHead, "Appear");
			MR::validateHitSensors(mHead);

			dirToPlayer(180.0f);

			MR::showMaterial(mHead, "SanboNeedleMat_v");
		}

		if (MR::isStep(this, 60)) {
			MR::emitEffect(this, "CocoSamboSmoke");
			MR::startActionSound(this, "EmSfsamboAppear", -1, -1, -1);
			MR::validateShadow(this, NULL);

			MR::tryRumblePadStrong(this, 0);
			MR::shakeCameraNormal();
		}

		if (MR::isStep(this, 105)) {
			MR::tryRumblePadWeak(this, 0);
			MR::shakeCameraWeak();
		}

		dirToPlayer(1.0f);

		if (MR::isGreaterStep(this, 105) && !tryDpdPointing(&NrvCocoSambo::NrvWait::sInstance)) {
			MR::setNerveAtBckStopped(this, &NrvCocoSambo::NrvWait::sInstance);
		}
	}

	void CocoSambo::exeWait() {
		if (MR::isFirstStep(this)) {
			if (!MR::isBckPlaying(this, "Wait")) {
				MR::startAction(this, "Wait");
				MR::startAction(mHead, "Wait");
			}

			MR::deleteEffectAll(this);
		}

		dirToPlayer(1.0f);

		if (!tryDpdPointing(&NrvCocoSambo::NrvWait::sInstance)) {
			if (MR::isNearPlayer(this, mAttackRange)) {
				setNerve(&NrvCocoSambo::NrvAttack::sInstance);
			}
			else if (!MR::isNearPlayer(this, 100.0f + mAppearRange)) {
				setNerve(&NrvCocoSambo::NrvHide::sInstance);
			}
		}
	}

	void CocoSambo::exeHide() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Hide");
			MR::startAction(mHead, "Hide");
		}

		if (MR::isStep(this, 65)) {
			MR::emitEffect(this, "CocoSamboSmoke");
			MR::startActionSound(this, "EmSfsamboHide", -1, -1, -1);
			MR::invalidateShadow(this, NULL);
			MR::tryRumblePadMiddle(this, 0);
			MR::shakeCameraWeak();
		}

		MR::setNerveAtBckStopped(this, &NrvCocoSambo::NrvHideWait::sInstance);
	}

	void CocoSambo::exeAttack() {
		if (MR::isFirstStep(this) && !MR::isBckPlaying(this, "Attack")) {
			MR::startAction(this, "Attack");
			MR::startAction(mHead, "Attack");
		}

		if (MR::getBckFrame(this) < 30.0f) {
			dirToPlayer(1.0f);
		}

		if (!tryDpdPointing(&NrvCocoSambo::NrvAttack::sInstance)) {
			if (MR::checkPassBckFrame(this, 115.0f)) {
				MR::emitEffect(mHead, "CocoSamboAttack");
				MR::tryRumblePadStrong(this, 0);
				MR::shakeCameraNormal();
			}
			else {
				MR::setNerveAtBckStopped(this, &NrvCocoSambo::NrvAttackInterval::sInstance);
			}
		}
	}

	void CocoSambo::exeAttackInterval() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Wait");
			MR::startAction(mHead, "Wait");
		}

		dirToPlayer(1.0f);

		if (!tryDpdPointing(&NrvCocoSambo::NrvWait::sInstance)) {
			MR::setNerveAtStep(this, &NrvCocoSambo::NrvWait::sInstance, 60);
		}
	}

	void CocoSambo::exeFallHead() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Hit");

			mHead->setNerve(&NrvCocoSamboHead::NrvHeadFall::sInstance);

			MR::emitEffect(this, "Hit");
			MR::startActionSound(this, "EmSfsamboDamage", -1, -1, -1);
			MR::startActionSound(this, "EmSfsamboBlowHead", -1, -1, -1);
			MR::tryRumblePadMiddle(this, 0);
			MR::hideMaterial(mHead, "SanboNeedleMat_v");
		}

		if (MR::isStep(this, 2)) {
			MR::stopScene(10);
		}

		if (MR::isStep(this, 3)) {
			MR::shakeCameraNormal();
		}

		if (MR::isActionEnd(this)) {
			emitDeath();

			setNerve(&NrvCocoSambo::NrvFallHeadHide::sInstance);
		}
	}

	void CocoSambo::exeFallHeadHide() {
		if (MR::isFirstStep(this)) {
			MR::startBckNoInterpole(this, "HideWait");
		}

		if (mHead->isNerve(&NrvCocoSamboHead::NrvHeadConnectedBody::sInstance)) {
			setNerve(&NrvCocoSambo::NrvRecoverWait::sInstance);
		}
	}

	void CocoSambo::exeRecoverWait() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Recover");
			MR::setBckRate(this, 0.0f);
			MR::invalidateHitSensors(mHead);
		}

		MR::setNerveAtStep(this, &NrvCocoSambo::NrvRecover::sInstance, 90);
	}

	void CocoSambo::exeRecover() {
		if (MR::isFirstStep(this)) {
			MR::setBckRate(this, 1.0f);
			MR::startAction(mHead, "Recover");
			mHead->mCalcOwnMatrix = true;
			MR::showMaterial(mHead, "SanboNeedleMat_v");
		}

		dirToPlayer(180.0f);

		MR::setNerveAtBckStopped(this, &NrvCocoSambo::NrvAppear::sInstance);
	}

	void CocoSambo::exePressY() {
		if (MR::isFirstStep(this)) {
			MR::setBckRate(this, 0.0f);
			MR::startAction(mHead, "PressY");
			MR::startActionSound(this, "EmStompedS", -1, -1, -1);
			MR::tryRumblePadMiddle(this, 0);
		}

		if (MR::isStep(this, 35)) {
			if (mHasItems) {
				MR::appearCoinPop(this, mTranslation, 1);
				mHasItems = false;
			}

			kill();
		}
	}

	void CocoSambo::exeBlow() {
		if (MR::isFirstStep(this)) {
			MR::tryRumblePadVeryStrong(this, 0);
			MR::startBlowHitSound(this);
			mHead->setNerve(&NrvCocoSamboHead::NrvHeadBlow::sInstance);
		}

		if (MR::isStep(this, 1)) {
			MR::stopScene(6);
		}

		if (MR::isStep(this, 2)) {
			MR::shakeCameraNormal();
		}

		if (MR::isStep(this, 20)) {
			if (mHasItems) {
				MR::appearStarPiece(this, mHead->mTranslation, 5, 10.0, 50.0, false);
				mHasItems = false;
			}

			kill();
		}
	}

	void CocoSambo::exeDpdPointing() {
		if (MR::isFirstStep(this)) {
			MR::setBckRate(this, 0.0f);
			MR::setBckRate(mHead, 0.0f);
			MR::emitEffect(this, "Touch1");
			MR::emitEffect(this, "Touch2");
			MR::emitEffect(this, "Touch3");
			MR::emitEffect(mHead, "Touch");
			mAnimScaleCtrl->startDpdHitVibration();
		}

		MR::startDPDFreezeLevelSound(this);

		if (!isStarPointerPointing()) {
			setNerve(mAfterDpdNerve);
		}
	}

	void CocoSambo::endDpdPointing() {
		MR::setBckRate(this, 1.0f);
		MR::setBckRate(mHead, 1.0f);
		MR::deleteEffect(this, "Touch1");
		MR::deleteEffect(this, "Touch2");
		MR::deleteEffect(this, "Touch3");
		MR::deleteEffect(mHead, "Touch");
		mAnimScaleCtrl->stopAndReset();
	}

	namespace NrvCocoSamboHead {
		void NrvHeadConnectedBody::execute(Spine *pSpine) const {}

		void NrvHeadFall::execute(Spine *pSpine) const {
			CocoSamboHead *pActor = (CocoSamboHead*)pSpine->mExecutor;
			pActor->exeFall();
		}

		void NrvHeadFallLand::execute(Spine *pSpine) const {
			CocoSamboHead *pActor = (CocoSamboHead*)pSpine->mExecutor;
			pActor->exeFallLand();
		}

		void NrvHeadSwoon::execute(Spine *pSpine) const {
			CocoSamboHead *pActor = (CocoSamboHead*)pSpine->mExecutor;
			pActor->exeSwoon();
		}

		void NrvHeadSwoonEnd::execute(Spine *pSpine) const {
			CocoSamboHead *pActor = (CocoSamboHead*)pSpine->mExecutor;
			pActor->exeSwoonEnd();
		}

		void NrvHeadBlow::execute(Spine *pSpine) const {
			CocoSamboHead *pActor = (CocoSamboHead*)pSpine->mExecutor;
			pActor->exeBlow();
		}

		NrvHeadConnectedBody(NrvHeadConnectedBody::sInstance);
		NrvHeadFall(NrvHeadFall::sInstance);
		NrvHeadFallLand(NrvHeadFallLand::sInstance);
		NrvHeadSwoon(NrvHeadSwoon::sInstance);
		NrvHeadSwoonEnd(NrvHeadSwoonEnd::sInstance);
		NrvHeadBlow(NrvHeadBlow::sInstance);
	}

	namespace NrvCocoSambo {
		void NrvHideWait::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeHideWait();
		}

		void NrvAppear::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvWait::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvHide::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeHide();
		}

		void NrvAttack::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeAttack();
		}

		void NrvAttackInterval::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeAttackInterval();
		}

		void NrvFallHead::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeFallHead();
		}

		void NrvFallHeadHide::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeFallHeadHide();
		}

		void NrvRecoverWait::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeRecoverWait();
		}

		void NrvRecover::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeRecover();
		}

		void NrvPressY::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exePressY();
		}

		void NrvBlow::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeBlow();
		}

		void NrvDpdPointing::execute(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->exeDpdPointing();
		}

		void NrvDpdPointing::executeOnEnd(Spine *pSpine) const {
			CocoSambo *pActor = (CocoSambo*)pSpine->mExecutor;
			pActor->endDpdPointing();
		}

		NrvHideWait(NrvHideWait::sInstance);
		NrvAppear(NrvAppear::sInstance);
		NrvWait(NrvWait::sInstance);
		NrvHide(NrvHide::sInstance);
		NrvAttack(NrvAttack::sInstance);
		NrvAttackInterval(NrvAttackInterval::sInstance);
		NrvFallHead(NrvFallHead::sInstance);
		NrvFallHeadHide(NrvFallHeadHide::sInstance);
		NrvRecoverWait(NrvRecoverWait::sInstance);
		NrvRecover(NrvRecover::sInstance);
		NrvPressY(NrvPressY::sInstance);
		NrvBlow(NrvBlow::sInstance);
		NrvDpdPointing(NrvDpdPointing::sInstance);
	}
}
