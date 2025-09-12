#include "WaterLeakPipe.h"

/*
* Author: Aurum
* Objects: WaterLeakPipe
* Parameters:
*  - Obj_arg0, float, 500.0f: Pipe height
*  - SW_AWAKE
* 
* The leaking water pipe from Freezeflame Galaxy and Deep Dark Galaxy. At the top of the pike is a splashing water
* spout. If this is touched while the player is using the Ice Power-up, a platform made of ice appears. The player can
* stand on the platform while the Power-up is still active. If the player goes off the platform or looses their
* Power-up, the platform disappears again. It is worth noting that this object does not come with its own collision mesh
* at all.
*/

namespace pt {
	extern void initRailToNearestAndRepositionWithGravity(LiveActor *);
	IceStepNoSlip::IceStepNoSlip(MtxPtr pMtx)
		: ModelObj("アイス床", "IceStepNoSlip", pMtx, MR::DrawBufferType_IndirectMapObjStrongLight, MR::CategoryList_Auto, MR::CategoryList_Auto, false)
	{

	}

	void IceStepNoSlip::init(const JMapInfoIter &rIter) {
		ModelObj::init(rIter);
		initNerve(&NrvIceStepNoSlip::NrvAppear::sInstance, 0);
	}

	void IceStepNoSlip::appear() {
		ModelObj::appear();
		setNerve(&NrvIceStepNoSlip::NrvAppear::sInstance);
	}

	void IceStepNoSlip::exeAppear() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "Start");
		}
	}

	void IceStepNoSlip::exeBreak() {
		if (MR::isFirstStep(this)) {
			MR::startAction(this, "End");
		}

		if (MR::isActionEnd(this)) {
			kill();
		}
	}

	WaterLeakPipe::WaterLeakPipe(const char *pName) : LiveActor(pName) {
		mIceStep = NULL;
		mPipeLength = 500.0f;
		mIceStepMtx = NULL;
	}

	void WaterLeakPipe::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, false);

		// Initialize clipping
		TVec3f upVec;
		MR::calcUpVec(&upVec, this);
		JMAVECScaleAdd(upVec, mTranslation, mClippingOffset, 0.5f * mPipeLength);
		MR::setClippingTypeSphere(this, 0.75f * mPipeLength, &mClippingOffset);

		// Initialize ice platform
		mIceStepMtx = (MtxPtr)MR::getJointMtx(this, "Top");
		MR::getJMapInfoArg0NoInit(rIter, &mPipeLength);
		initPipeHeight();

		mIceStep = new IceStepNoSlip(mIceStepMtx);
		mIceStep->initWithoutIter();
		mIceStep->makeActorDead();

		// Setup nerve and make appeared
		initNerve(&NrvWaterLeakPipe::NrvWait::sInstance, 0);
		makeActorAppeared();
	}

	void WaterLeakPipe::calcAnim() {
		// Dummy function. This prevents resetting the resized joints.
	}

	bool WaterLeakPipe::receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver) {
		if (!MR::isPlayerElementModeIce() || isNerve(&NrvWaterLeakPipe::NrvFreeze::sInstance)) {
			return false;
		}

		if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
			TVec3f upVec;
			MR::calcUpVec(&upVec, this);

			// Check if player is below splash
			TVec3f playerOffset = *MR::getPlayerPos() - mTranslation;
			f32 dot = playerOffset.dot(upVec);

			if (dot < 0.0f) {
				return false;
			}

			// Check if player is moving above splash
			dot = MR::getPlayerVelocity()->dot(upVec);

			if (dot >= 0.0f) {
				return false;
			}

			setNerve(&NrvWaterLeakPipe::NrvFreeze::sInstance);
		}

		return false;
	}

	void WaterLeakPipe::exeWait() {
		if (MR::isFirstStep(this)) {
			MR::emitEffect(this, "Splash");
		}

		MR::startActionSound(this, "OjLvWaterLeak", -1, -1, -1);
	}

	void WaterLeakPipe::exeFreeze() {
		if (MR::isFirstStep(this) && MR::isDead(mIceStep)) {
			MR::forceDeleteEffect(this, "Splash");

			mIceStep->appear();

			MR::startActionSound(this, "OjIceFloorFreeze", -1, -1, -1);
			MR::invalidateClipping(this);
		}
		else if (MR::isOnPlayer(mIceStep) && MR::isPlayerElementModeIce()) {
			setNerve(&NrvWaterLeakPipe::NrvFreeze::sInstance);
		}
		else if (MR::isStep(this, 15)) {
			MR::startActionSound(this, "OjIceFloorMelt", -1, -1, -1);
			MR::validateClipping(this);

			mIceStep->setNerve(&NrvIceStepNoSlip::NrvBreak::sInstance);
			setNerve(&NrvWaterLeakPipe::NrvWait::sInstance);
		}
	}

	void WaterLeakPipe::initPipeHeight() {
		// Calculate top position
		TVec3f upVec, topPos;
		MR::calcUpVec(&upVec, this);
		JMAVECScaleAdd(upVec, mTranslation, topPos, mPipeLength);

		MR::setMtxTrans(mIceStepMtx, topPos);
		calcAndSetBaseMtx();
	}

	namespace NrvIceStepNoSlip {
		void NrvAppear::execute(Spine *pSpine) const {
			IceStepNoSlip *pActor = (IceStepNoSlip*)pSpine->mExecutor;
			pActor->exeAppear();
		}

		void NrvBreak::execute(Spine *pSpine) const {
			IceStepNoSlip *pActor = (IceStepNoSlip*)pSpine->mExecutor;
			pActor->exeBreak();
		}

		NrvAppear(NrvAppear::sInstance);
		NrvBreak(NrvBreak::sInstance);
	}

	namespace NrvWaterLeakPipe {
		void NrvWait::execute(Spine *pSpine) const {
			WaterLeakPipe *pActor = (WaterLeakPipe*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvFreeze::execute(Spine *pSpine) const {
			WaterLeakPipe *pActor = (WaterLeakPipe*)pSpine->mExecutor;
			pActor->exeFreeze();
		}

		NrvWait(NrvWait::sInstance);
		NrvFreeze(NrvFreeze::sInstance);
	}
}
