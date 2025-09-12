#include "ScrewSwitchBase.h"

/*
* Author: Aurum
* Objects: ScrewSwitch, ScrewSwitchBase, ValveSwitch
* Parameters:
*  All objects:
*  - cutscene
*  - SW_A, need, write: Activated when screwed
*  - SW_AWAKE
*  ScrewSwitchReverse:
*  - Obj_arg0, bool, false: Force jump?
*  - Obj_arg7, float, 1000.0f: Shadow length
*  ValveSwitch:
*  - Obj_arg0, bool, false: Reverse direction?
*
* In SMG1, there are three types of screw switches that do not appear in SMG2 at all. Since they
* all share nearly identical code, I created a base class to reduce duplicated code. All of them
* are now compatible with SW_AWAKE and demo functionality.
* Each screw implementation requires two hit sensors named Body and Binder.
*/

namespace pt {
	ScrewSwitchBase::ScrewSwitchBase(const char* pName) : LiveActor(pName) {
		mBindedActor = NULL;
		mMapObjConnector = new MapObjConnector(this);
	}

	void ScrewSwitchBase::init(const JMapInfoIter& rIter) {
		MR::initDefaultPos(this, rIter);
		MR::processInitFunction(this, rIter, false);
		MR::connectToSceneMapObjDecorationStrongLight(this);

		// The actual sensors and object-specific properties are set up in a different function
		initScrew(rIter);

		// Initialize light and effects
		MR::initLightCtrl(this);
		initEffectKeeper(0, NULL, false);

		// Initialize switches and demo cast
		MR::useStageSwitchWriteA(this, rIter);
		MR::useStageSwitchAwake(this, rIter);
		MR::tryRegisterDemoCast(this, rIter);

		initNerve(&NrvScrewSwitchBase::NrvWait::sInstance, 0);
		makeActorAppeared();
	}

	void ScrewSwitchBase::initAfterPlacement() {
		mMapObjConnector->attachToUnder();
	}

	void ScrewSwitchBase::calcAndSetBaseMtx() {
		LiveActor::calcAndSetBaseMtx();
		mMapObjConnector->connect();
	}

	bool ScrewSwitchBase::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
		if (MR::isMsgRushBegin(msg) && MR::isSensorPlayer(pSender) && MR::isOnPlayer(getSensor("Binder"))) {
			mBindedActor = pSender->mActor;
			MR::startActionSound(mBindedActor, "PvTwistStart", -1, -1, -1);
			MR::startActionSound(mBindedActor, "PmSpinAttack", -1, -1, -1);

			setNerve(&NrvScrewSwitchBase::NrvAdjust::sInstance);
			return true;
		}
		else if (MR::isMsgUpdateBaseMtx(msg) && mBindedActor && isNerve(&NrvScrewSwitchBase::NrvScrew::sInstance)) {
			updateBindActorMtx();
			return true;
		}
		else {
			return MR::isMsgRushCancel(msg);
		}
	}

	void ScrewSwitchBase::exeEnd() {
		if (MR::isFirstStep(this)) {
			MR::invalidateHitSensors(this);

			if (MR::isPlayerInRush()) {
				MR::endBindAndPlayerWait(this);
				mBindedActor = NULL;
			}
		}
	}

	void ScrewSwitchBase::updateBindActorMtx() {
		TMtx34f mtxTR;
		PSMTXCopy((MtxPtr)mBindedActor->getBaseMtx(), (MtxPtr)&mtxTR);
		HitSensor* senBinder = getSensor("Binder");
		mtxTR.mMtx[0][3] = senBinder->mPosition.x;
		mtxTR.mMtx[1][3] = senBinder->mPosition.y;
		mtxTR.mMtx[2][3] = senBinder->mPosition.z;
		MR::setBaseTRMtx(mBindedActor, (MtxPtr)&mtxTR);
	}

	ScrewSwitch::ScrewSwitch(const char* pName) : ScrewSwitchBase(pName) {
		mScrewCollision = NULL;
	}

	void ScrewSwitch::initScrew(const JMapInfoIter& rIter) {
		// Initialize sensors
		initHitSensor(2);
		MR::addBodyMessageSensorMapObj(this);
		MR::addHitSensorAtJoint(this, "Binder", "Screw", ATYPE_BINDER, 8, 150.0f, mGravity * -150.0f);

		// Initialize collision
		MR::initCollisionParts(this, "ScrewCol", getSensor("Binder"), MR::getJointMtx(this, "Screw"));
		mScrewCollision = MR::createCollisionPartsFromLiveActor(this, "ScrewReceiveCol", getSensor("Body"), MR::getJointMtx(this, "ScrewReceive"), MR::CollisionScaleType_NotUsingScale);
		MR::validateCollisionParts(mScrewCollision);

		// Initialize sounds
		initSound(4, "ScrewSwitch", false, TVec3f(0.0f, 0.0f, 0.0f));
	}

	void ScrewSwitch::control() {
		mScrewCollision->setMtx();
	}

	void ScrewSwitch::exeWait() {
		if (MR::isFirstStep(this)) {
			MR::startBrkAndSetFrameAndStop(this, "ScrewSwitchOn", 0.0f);
		}
	}

	void ScrewSwitch::exeAdjust() {
		if (MR::isFirstStep(this)) {
			MR::startBrkAndSetFrameAndStop(this, "ScrewSwitchOn", 0.0f);
		}

		JMAVECLerp(mBindedActor->mTranslation, getSensor("Binder")->mPosition, mBindedActor->mTranslation, getNerveStep() / 3.0f);

		if (MR::isStep(this, 3)) {
			setNerve(&NrvScrewSwitchBase::NrvScrew::sInstance);
		}
	}

	void ScrewSwitch::exeScrew() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "ScrewSwitchOn");
			MR::startBrk(this, "ScrewSwitchOn");
			MR::startBckPlayer("ScrewSwitchOn", NULL);
		}

		if (MR::isLessStep(this, 30)) {
			MR::startActionSound(this, "OjLvScrewSwitchMove", -1, -1, -1);
		}

		if (MR::isStep(this, 30)) {
			MR::startActionSound(this, "OjScrewSwitchOn", -1, -1, -1);
			MR::tryRumblePadVeryStrong(this, 0);
			MR::shakeCameraNormal();
		}

		if (MR::isActionEnd(this)) {
			MR::onSwitchA(this);
			setNerve(&NrvScrewSwitchBase::NrvEnd::sInstance);
		}
	}

	ScrewSwitchReverse::ScrewSwitchReverse(const char* pName) : ScrewSwitchBase(pName) {
		mForceJump = false;
	}

	void ScrewSwitchReverse::initScrew(const JMapInfoIter& rIter) {
		// Initialize sensors
		initHitSensor(2);
		MR::addBodyMessageSensorMapObj(this);
		MR::addHitSensorAtJoint(this, "Binder", "Screw", ATYPE_BINDER, 8, 120.0f, mGravity * -130.0f);

		// Initialize collision
		MR::initCollisionParts(this, "ScrewReverseCol", getSensor("Binder"), MR::getJointMtx(this, "Screw"));

		// Initialize sounds
		initSound(5, "ScrewSwitchReverse", false, TVec3f(0.0f, 0.0f, 0.0f));

		// Initialize shadow
		f32 shadowLength = 1000.0f;
		MR::getJMapInfoArg7NoInit(rIter, &shadowLength);

		MR::initShadowVolumeCylinder(this, 100.0f);
		MR::setShadowDropPosition(this, NULL, mTranslation);
		MR::setShadowDropLength(this, NULL, shadowLength);

		MR::getJMapInfoArg0NoInit(rIter, &mForceJump);
	}

	void ScrewSwitchReverse::exeWait() {}

	void ScrewSwitchReverse::exeAdjust() {
		JMAVECLerp(mBindedActor->mTranslation, getSensor("Binder")->mPosition, mBindedActor->mTranslation, getNerveStep() / 3.0f);

		if (MR::isStep(this, 3)) {
			setNerve(&NrvScrewSwitchBase::NrvScrew::sInstance);
		}
	}

	void ScrewSwitchReverse::exeScrew() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "ScrewSwitchReverseOn");
			MR::startBckPlayer("ScrewSwitchReverseOn", NULL);
		}

		if (MR::isLessStep(this, 25)) {
			MR::startActionSound(this, "OjLvScrSwitchRMove", -1, -1, -1);
		}

		if (MR::isStep(this, 25)) {
			MR::startActionSound(this, "OjScrSwitchROn", -1, -1, -1);
			MR::tryRumblePadVeryStrong(this, 0);
			MR::shakeCameraNormal();
			MR::invalidateHitSensors(this);
			MR::invalidateCollisionParts(this);

			if (MR::isPlayerInRush()) {
				endBind();
			}
		}

		if (MR::isActionEnd(this)) {
			MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
			MR::onSwitchA(this);
			kill();
		}
	}

	void ScrewSwitchReverse::endBind() {
		TVec3f jumpVec;
		MR::calcUpVec(&jumpVec, this);
		jumpVec *= 28.0f;

		if (mForceJump) {
			MR::endBindAndPlayerForceJump(this, jumpVec, 0);
		}
		else {
			MR::endBindAndPlayerJump(this, jumpVec, 0);
		}

		mBindedActor = NULL;
	}

	ValveSwitch::ValveSwitch(const char* pName) : ScrewSwitchBase(pName) {
		PSMTXIdentity((MtxPtr)&mCollisionMtx);
		mIsReverse = false;
	}

	void ValveSwitch::initScrew(const JMapInfoIter& rIter) {
		// Initialize sensors
		initHitSensor(2);
		MR::addHitSensorMapObj(this, "Body", 8, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));
		MR::addHitSensorAtJoint(this, "Binder", "Valve", ATYPE_BINDER, 8, 150.0f, mGravity * -75.0f);

		// Initialize collision
		PSMTXCopy(MR::getJointMtx(this, "Valve"), (MtxPtr) & mCollisionMtx);
		MR::initCollisionParts(this, "ValveCol", getSensor("Binder"), (MtxPtr)&mCollisionMtx);

		// Initialize sounds
		initSound(4, "ValveSwitch", false, TVec3f(0.0f, 0.0f, 0.0f));

		MR::getJMapInfoArg0NoInit(rIter, &mIsReverse);
	}

	void ValveSwitch::control() {
		PSMTXCopy(MR::getJointMtx(this, "Valve"), (MtxPtr)&mCollisionMtx);
		MR::makeMtxWithoutScale((TPos3f*) & mCollisionMtx, *((TPos3f*)&mCollisionMtx));
	}

	void ValveSwitch::exeWait() {
		if (MR::isFirstStep(this)) {
			MR::startBrk(this, "ValveSwitchOn");

			if (mIsReverse) {
				MR::startAction(this, "ValveSwitchReverseOn");
			}
			else {
				MR::startAction(this, "ValveSwitchOn");
			}

			MR::setBrkFrameAndStop(this, 0.0f);
			MR::setBckFrameAndStop(this, 0.0f);
		}
	}

	void ValveSwitch::exeAdjust() {
		if (MR::isFirstStep(this)) {
			MR::startBrkAndSetFrameAndStop(this, "ValveSwitchOn", 0.0f);
		}

		JMAVECLerp(mBindedActor->mTranslation, getSensor("Binder")->mPosition, mBindedActor->mTranslation, getNerveStep() / 3.0f);

		if (MR::isStep(this, 3)) {
			setNerve(&NrvScrewSwitchBase::NrvScrew::sInstance);
		}
	}

	void ValveSwitch::exeScrew() {
		if (MR::isFirstStep(this)) {
			if (mIsReverse) {
				MR::startAction(this, "ValveSwitchReverseOn");
			}
			else {
				MR::startAction(this, "ValveSwitchOn");
			}

			MR::startBrk(this, "ValveSwitchOn");
			MR::startBckPlayer("ScrewSwitchOn", NULL);
		}

		if (MR::isLessStep(this, 30)) {
			MR::startActionSound(this, "OjLvValveSwitchMove", -1, -1, -1);
		}

		if (MR::isStep(this, 30)) {
			MR::startActionSound(this, "OjValveSwitchOn", -1, -1, -1);
			MR::tryRumblePadVeryStrong(this, 0);
			MR::shakeCameraNormal();
		}

		if (MR::isActionEnd(this)) {
			MR::onSwitchA(this);
			setNerve(&NrvScrewSwitchBase::NrvEnd::sInstance);
		}
	}

	namespace NrvScrewSwitchBase {
		void NrvWait::execute(Spine* pSpine) const {
			ScrewSwitchBase* pActor = (ScrewSwitchBase*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvAdjust::execute(Spine* pSpine) const {
			ScrewSwitchBase* pActor = (ScrewSwitchBase*)pSpine->mExecutor;
			pActor->exeAdjust();
		}

		void NrvScrew::execute(Spine* pSpine) const {
			ScrewSwitchBase* pActor = (ScrewSwitchBase*)pSpine->mExecutor;
			pActor->exeScrew();
		}

		void NrvEnd::execute(Spine* pSpine) const {
			ScrewSwitchBase* pActor = (ScrewSwitchBase*)pSpine->mExecutor;
			pActor->exeEnd();
		}

		NrvWait(NrvWait::sInstance);
		NrvAdjust(NrvAdjust::sInstance);
		NrvScrew(NrvScrew::sInstance);
		NrvEnd(NrvEnd::sInstance);
	}
}