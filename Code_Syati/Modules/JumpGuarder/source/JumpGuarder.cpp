#include "JumpGuarder.h"

/*
* Author: Aurum
* Objects: JumpGuarder
* Parameters:
* - Obj_arg1, float, 16.0f: Launch velocity
* - Obj_arg2, long, 4: Number of Topmini
* - SW_AWAKE
* - SW_A, use, read: Make active
* - SW_B, use, read: Make passive
* 
* JumpGuarder is a green, stationary Spring Topman that emits several Topmini when approached by
* the player. The head functions as a trampoline that can be used by the player to gain height.
*/

namespace pt {
	extern void clampS32(s32 min, s32 max, s32* val);

	JumpGuarder::JumpGuarder(const char *pName) : JumpEmitter(pName) {
		mBabys = NULL;
		mNumBabys = 4;
		mStaggerDelay = 0;
		mNumPendingBabys = 0;
		mLaunchVelocity = 16.0f;
	}

	void JumpGuarder::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, "JumpGuarder", false);
		mHead = MR::createPartsModelNoSilhouettedMapObj(this, "ジャンプガーダー頭", "JumpGuarderHead", (MtxPtr)&mHeadMtx);

		// Initialize sensors
		initHitSensor(2);
		MR::addHitSensorMtx(this, "Jump", ATYPE_PLAYER_AUTO_JUMP, 8, 145.0f, MR::getJointMtx(mHead, "SpringJoint3"), TVec3f(0.0f, -100.0f, 0.0f));
		MR::addHitSensorAtJoint(this, "Body", "Body", ATYPE_BEGOMAN, 8, 145.0f, TVec3f(0.0f, 35.0f, 0.0f));
		MR::validateHitSensor(this, "Body");
		MR::invalidateHitSensor(this, "Jump");

		// Initialize miscellaneous attributes
		MR::initLightCtrl(mHead);
		MR::initShadowVolumeSphere(this, 140.0f);
		MR::invalidateClipping(this);

		MR::startBckWithInterpole(this, "Down", 0);
		MR::setBckFrame(this, 0.0f);
		MR::calcAnimDirect(this);
		mJointMtx = MR::getJointMtx(this, "Body");

		JumpEmitter::init(rIter);
		MR::joinToGroupArray(this, rIter, 0, 32);

		// Read Obj_arg entries
		MR::getJMapInfoArg1NoInit(rIter, &mLaunchVelocity);
		MR::getJMapInfoArg2NoInit(rIter, &mNumBabys);
		pt::clampS32(1, 4, &mNumBabys);

		// Create BegomanBabys
		mBabys = new BegomanBaby*[mNumBabys];

		for (s32 i = 0; i < mNumBabys; i++) {
			BegomanBaby *pBaby = new BegomanBaby("ベビーベーゴマン");
			pBaby->mHost = this;
			pBaby->mTranslation.set(mTranslation);
			pBaby->initWithoutIter();
			pBaby->makeActorDead();
			mBabys[i] = pBaby;
		}

		// Setup nerve and make appeared
		initNerve(&NrvJumpGuarder::NrvHide::sInstance, 0);
		makeActorAppeared();
	}

	void JumpGuarder::control() {
		MR::attachSupportTicoToTarget(this);

		TMtx34f mtxTRS;
		MR::makeMtxTRS((MtxPtr)&mtxTRS, TVec3f(0.0f, 44.0f, 0.0f), mHead->mRotation, mScale);

		PSMTXCopy(mJointMtx, (MtxPtr)&mHeadMtx);
		PSMTXConcat((MtxPtr)&mHeadMtx, (MtxPtr)&mtxTRS, (MtxPtr)&mHeadMtx);

		// Decrement stagger delay and ensure the value does not fall below 0
		mStaggerDelay = (mStaggerDelay - 1) & (((u32)(mStaggerDelay - 1) >> 31) - 1);

		updateEventCamera();
		JumpEmitter::control();
	}

	void JumpGuarder::attackSensor(HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isSensorEnemy(pReceiver) && isHit(pReceiver->mActor)) {
			MR::sendMsgEnemyAttack(pReceiver, pSender);

			if (isNerve(&NrvJumpGuarder::NrvWait::sInstance) && !MR::isOnGround(pReceiver->mActor)) {
				MR::startAction(this, "Hit");
				MR::startActionSound(this, "EmJguarderHit", -1, -1, -1);
			}
		}
		else if (MR::isSensorPlayer(pReceiver)) {
			MR::sendMsgPush(pReceiver, pSender);

			if (isNerve(&NrvJumpGuarder::NrvWait::sInstance) || isNerve(&NrvJumpGuarder::NrvHopWait::sInstance)) {
				if (MR::isPlayerStaggering() && mStaggerDelay == 0) {
					mStaggerDelay = 60;
					MR::startAction(this, "Hit");
					MR::startActionSound(this, "EmJguarderHit", -1, -1, -1);
				}
			}
		}
	}

	bool JumpGuarder::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (MR::isMsgPlayerTrample(msg)) {
			if (pReceiver->isType(ATYPE_PLAYER_AUTO_JUMP)) {
				TVec3f upVec;
				MR::calcUpVec(&upVec, this);
				MR::setPlayerJumpVec(upVec);

				startEventCamera();
				setNerve(&NrvJumpGuarder::NrvHopJump::sInstance);
			}
			else {
				setNerve(&NrvJumpGuarder::NrvHopStart::sInstance);
			}

			return true;
		}
		else if (MR::isMsgPlayerHipDrop(msg)) {
			MR::forceJumpPlayer(-mGravity);
			return true;
		}
		else if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
			if (!isNerve(&NrvJumpGuarder::NrvHopStart::sInstance)) {
				setNerve(&NrvJumpGuarder::NrvHopStart::sInstance);
				return true;
			}
		}
		else {
			return MR::isMsgStarPieceReflect(msg);
		}

		return false;
	}

	bool JumpGuarder::receiveOtherMsg(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (msg == ACTMES_GROUP_ATTACK) {
			MR::invalidateClipping(this);
			setNerve(&NrvJumpGuarder::NrvUp::sInstance);
			return true;
		}
		else if (msg == ACTMES_GROUP_HIDE) {
			setNerve(&NrvJumpGuarder::NrvDown::sInstance);
			return true;
		}

		return false;
	}

	void JumpGuarder::exeHide() {
		updateRotate();

		if (MR::isFirstStep(this)) {
			MR::startAction(mHead, "Wait");
			MR::startBrk(mHead, "Green");
			MR::validateClipping(this);
			MR::setShadowVolumeSphereRadius(this, NULL, 110.0f);
		}
		else if (MR::enableGroupAttack(this, 2000.0f, 500.0f)) {
			MR::sendMsgToGroupMember(ACTMES_GROUP_ATTACK, this, getSensor("Body"), "Body");
		}
	}

	void JumpGuarder::exeUp() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Up");
			MR::startActionSound(this, "EmJguarderAppear", -1, -1, -1);
		}

		f32 bckMaxFrame = MR::getBckFrameMax(this);
		f32 bckFrame = MR::getBckFrame(this);
		f32 radius = 110.0f + (bckFrame / bckMaxFrame) * 30.0f;
		MR::setShadowVolumeSphereRadius(this, NULL, radius);

		if (MR::isActionEnd(this)) {
			MR::setShadowVolumeSphereRadius(this, NULL, 140.0f);
			setNerve(&NrvJumpGuarder::NrvWait::sInstance);
		}
	}

	void JumpGuarder::exeWait() {
		updateRotate();

		if (!MR::enableGroupAttack(this, 2200.0f, 500.0f)) {
			MR::sendMsgToGroupMember(ACTMES_GROUP_HIDE, this, getSensor("Body"), "Body");
		}
		else if (enableAttack()) {
			setNerve(&NrvJumpGuarder::NrvPreOpen::sInstance);
		}
	}

	void JumpGuarder::exeDown() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Down");
			MR::startActionSound(this, "EmJguarderHide", -1, -1, -1);
		}

		f32 bckMaxFrame = MR::getBckFrameMax(this);
		f32 bckFrame = MR::getBckFrame(this);
		f32 radius = 110.0f + (1.0f - (bckFrame / bckMaxFrame)) * 30.0f;
		MR::setShadowVolumeSphereRadius(this, NULL, radius);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvJumpGuarder::NrvHide::sInstance);
		}
	}

	void JumpGuarder::exeHopStart() {
		if (MR::isFirstStep(this)) {
			MR::startAction(mHead, "HopStart");
			MR::startBrk(mHead, "OnAndOff");
			MR::startAction(this, "Damage");
			MR::startActionSound(this, "EmJguarderHit", -1, -1, -1);
			getSensor("Body")->mRadius = 120.0f;
		}

		MR::startActionSound(this, "EmLvJguarderShake", -1, -1, -1);

		// If shaken during the launching phase, kill all pending BegomanBabys
		if (MR::isStep(this, 10)) {
			for (u32 i = 0; i < mNumPendingBabys; i++) {
				mPendingBabys[i]->makeActorDead();
			}

			mNumPendingBabys = 0;
		}

		if (MR::isActionEnd(mHead)) {
			setNerve(&NrvJumpGuarder::NrvHopWait::sInstance);
		}
	}

	void JumpGuarder::exeHopWait() {
		if (MR::isFirstStep(this)) {
			MR::stopBck(this);
			MR::startAction(mHead, "HopWait");
			MR::validateHitSensor(this, "Jump");
		}

		MR::startActionSound(this, "EmLvJguarderShake", -1, -1, -1);

		if (MR::isStep(this, 300)) {
			setNerve(&NrvJumpGuarder::NrvHopEnd::sInstance);
		}
	}

	void JumpGuarder::exeHopJump() {
		if (MR::isFirstStep(this)) {
			MR::startAction(mHead, "HopJump");
			MR::startActionSound(this, "EmJguarderTrample", -1, -1, -1);
		}

		if (MR::isActionEnd(mHead)) {
			setNerve(&NrvJumpGuarder::NrvHopWait::sInstance);
		}
	}

	void JumpGuarder::exeHopEnd() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "HopEnd");
			MR::startAction(mHead, "HopEnd");
			MR::startBrk(mHead, "Green");
			MR::startActionSound(this, "EmJguarderCloseSpring", -1, -1, -1);
		}

		if (MR::isActionEnd(this) && MR::isActionEnd(mHead)) {
			getSensor("Body")->mRadius = 145.0f;
			MR::invalidateHitSensor(this, "Jump");
			setNerve(&NrvJumpGuarder::NrvWait::sInstance);
		}
	}

	void JumpGuarder::exePreOpen() {
		updateRotate();

		if (!MR::enableGroupAttack(this, 2200.0f, 500.0f)) {
			MR::sendMsgToGroupMember(ACTMES_GROUP_HIDE, this, getSensor("Body"), "Body");
		}

		if (MR::isFirstStep(this)) {
			setNerve(&NrvJumpGuarder::NrvOpen::sInstance);
		}
	}

	void JumpGuarder::exeOpen() {
		if (MR::isLessStep(this, 70)) {
			updateRotate();
		}

		// Collect and prepare new BegomanBabys to be launched
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Open");
			MR::startActionSound(this, "EmJguarderShutterOpen", -1, -1, -1);

			// Collect dead BegomanBabys to "revive" them
			mNumPendingBabys = 0;

			for (s32 i = 0; i < mNumBabys; i++) {
				if (MR::isDead(mBabys[i])) {
					mPendingBabys[mNumPendingBabys++] = mBabys[i];
				}
			}

			MtxPtr headBaseMtx = (MtxPtr)mHead->getBaseMtx();
			TVec3f ydir, zdir;
			MR::extractMtxYDir(headBaseMtx, &ydir);
			MR::extractMtxZDir(headBaseMtx, &zdir);

			for (s32 i = 0; i < mNumPendingBabys; i++) {
				BegomanBaby* baby = mPendingBabys[i];

				MR::rotateVecDegree(&zdir, ydir, 360.0f / mNumPendingBabys);
				baby->mTranslation.set(mTranslation + zdir * 64.0f);
				baby->mVelocity.setAll(0.0f);

				appearBabyFromGuarder(baby);
			}
		}
		// Keep BegomanBabys inside until launch
		else if (MR::isLessStep(this, 70)) {
			MtxPtr headBaseMtx = (MtxPtr)mHead->getBaseMtx();
			TVec3f ydir, zdir;
			MR::extractMtxYDir(headBaseMtx, &ydir);
			MR::extractMtxZDir(headBaseMtx, &zdir);

			for (s32 i = 0; i < mNumPendingBabys; i++) {
				BegomanBaby* baby = mPendingBabys[i];

				MR::rotateVecDegree(&zdir, ydir, 360.0f / mNumPendingBabys);
				baby->mTranslation.set(mTranslation + zdir * 64.0f);
				baby->mVelocity.setAll(0.0f);
			}
		}
		// Finally, launch BegomanBabys
		else if (MR::isStep(this, 70)) {
			MtxPtr headBaseMtx = (MtxPtr)mHead->getBaseMtx();
			TVec3f ydir, zdir;
			MR::extractMtxYDir(headBaseMtx, &ydir);
			MR::extractMtxZDir(headBaseMtx, &zdir);

			for (s32 i = 0; i < mNumPendingBabys; i++) {
				BegomanBaby* baby = mPendingBabys[i];

				MR::rotateVecDegree(&zdir, ydir, 360.0f / mNumPendingBabys);
				baby->mVelocity.set(zdir * mLaunchVelocity);
			}

			MR::startActionSound(this, "EmJguarderLaunchBaby", -1, -1, -1);
		}
		// Invalidate pending BegomanBabys
		else if (MR::isStep(this, 74)) {
			mNumPendingBabys = 0;
		}

		if (MR::isActionEnd(this)) {
			setNerve(&NrvJumpGuarder::NrvClose::sInstance);
		}
	}

	void JumpGuarder::exeClose() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Close");
		}

		MR::startActionSound(this, "EmLvJguarderShutterClose", -1, -1, -1);

		if (MR::isActionEnd(this)) {
			setNerve(&NrvJumpGuarder::NrvInter::sInstance);
		}
	}

	void JumpGuarder::exeInter() {
		updateRotate();

		if (!MR::enableGroupAttack(this, 2200.0f, 500.0f)) {
			MR::sendMsgToGroupMember(ACTMES_GROUP_HIDE, this, getSensor("Body"), "Body");
		}

		setNerve(&NrvJumpGuarder::NrvWait::sInstance);
	}

	bool JumpGuarder::enableAttack() {
		for (s32 i = 0; i < mNumBabys; i++) {
			if (MR::isDead(mBabys[i])) {
				return getNerveStep() % 360 == 0;
			}
		}
		return false;
	}

	/*
	* We have to check if the actor that hits this JumpGuarder's sensors is none of the
	* pending children inside of it. This is necessary to prevent these BegomanBabys from
	* causing EmJguarderHit "spam".
	*/
	bool JumpGuarder::isHit(const LiveActor *pActor) const {
		for (u32 i = 0; i < mNumPendingBabys; i++) {
			if (mPendingBabys[i] == pActor) {
				return false;
			}
		}

		return true;
	}

	void JumpGuarder::appearBabyFromGuarder(BegomanBaby *pBaby) {
		pBaby->BegomanBase::appear();
		pBaby->setNerve(&NrvBegomanBaby::HostTypeNrvLaunchFromGuarder::sInstance);
	}

	namespace NrvJumpGuarder {
		void NrvHide::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeHide();
		}

		void NrvUp::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeUp();
		}

		void NrvWait::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvDown::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeDown();
		}

		void NrvHopStart::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeHopStart();
		}

		void NrvHopWait::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeHopWait();
		}

		void NrvHopJump::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeHopJump();
		}

		void NrvHopEnd::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeHopEnd();
		}

		void NrvPreOpen::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exePreOpen();
		}

		void NrvOpen::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeOpen();
		}

		void NrvClose::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeClose();
		}

		void NrvInter::execute(Spine *pSpine) const {
			JumpGuarder *pActor = (JumpGuarder*)pSpine->mExecutor;
			pActor->exeInter();
		}

		NrvHide(NrvHide::sInstance);
		NrvUp(NrvUp::sInstance);
		NrvWait(NrvWait::sInstance);
		NrvDown(NrvDown::sInstance);
		NrvHopStart(NrvHopStart::sInstance);
		NrvHopWait(NrvHopWait::sInstance);
		NrvHopJump(NrvHopJump::sInstance);
		NrvHopEnd(NrvHopEnd::sInstance);
		NrvPreOpen(NrvPreOpen::sInstance);
		NrvOpen(NrvOpen::sInstance);
		NrvClose(NrvClose::sInstance);
		NrvInter(NrvInter::sInstance);
	}
}
