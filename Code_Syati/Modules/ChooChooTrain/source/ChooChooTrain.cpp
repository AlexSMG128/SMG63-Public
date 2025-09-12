#include "ChooChooTrain.h"

/*
* Author: Aurum
* Objects: ChooChooTrain class
* Parameters:
*  - Obj_arg0, int, 3: Number of train parts
*  - Obj_arg1, float, 5.0f: Movement speed
*  - SW_A, use, read: Accelerates when on, decelerates when off
*  - Rail, need: For movement
*/

namespace pt {
	extern void turnToDirectionGravityFront(LiveActor* pActor, TVec3f rFront);

	ChooChooTrainParts::ChooChooTrainParts(ChooChooTrain *pHost, const char *pModelName)
		: ModelObj("汽車ポッポ客車", pModelName, NULL, MR::CategoryList_Auto, MR::MovementType_CollisionMapObj, MR::CalcAnimType_CollisionMapObj, false)
	{
		mHost = pHost;
	}

	ChooChooTrain::ChooChooTrain(const char *pName) : LiveActor(pName) {
		mRailClipping.zero();
		mSpeed = 5.0f;
		mMaxSpeed = 5.0f;
		mWhistleTimer = 0;
		mAccelTime = 240;
	}

	void ChooChooTrain::init(const JMapInfoIter &rIter) {
		MR::processInitFunction(this, rIter, "ChooChooTrain", false);
		MR::setSensorType(this, "Body", ATYPE_MAP_OBJ_MOVE_COLLISION);
		MR::onCalcGravity(this);

		MR::moveCoordToNearestPos(this, mTranslation);
		MR::moveTransToCurrentRailPos(this);

		// Get Obj_args
		mWhistleTimer = MR::getRandom((s32)180, (s32)600);

		s32 arg = -1;
		MR::getJMapInfoArg0NoInit(rIter, &arg);
		s32 numParts = arg > 0 ? arg : 3;

		arg = -1;
		MR::getJMapInfoArg1NoInit(rIter, &arg);

		if (arg > 0) {
			mSpeed = arg;
			mMaxSpeed = arg;
		}

		// Initialize parts, their positions and animations
		mParts.mArray.init(numParts);
		mParts.mCount = 0;

		f32 restoreRailCoord = MR::getRailCoord(this);
		MR::reverseRailDirection(this);

		const char *objName;
		char modelName[128];
		MR::getObjectName(&objName, rIter);
		snprintf(modelName, 128, "%sBody", objName);

		for (s32 i = 0; i < numParts; i++) {
			ChooChooTrainParts *parts = new ChooChooTrainParts(this, modelName);
			parts->initWithoutIter();
			MR::initCollisionParts(parts, modelName, getSensor("Body"), 0);
			MR::invalidateClipping(parts);
			MR::onCalcGravity(parts);
			MR::setScale(parts, mScale);

			MR::moveCoord(this, 1080.0f * mScale.y);
			MR::moveTransToOtherActorRailPos(parts, this);

			parts->makeActorAppeared();
			MR::startAction(parts, "Run");

			mParts.assign(parts, i);
			mParts.mCount++;
		}

		MR::reverseRailDirection(this);
		MR::setRailCoord(this, restoreRailCoord);

		// Initialize nerve and switch functors
		if (MR::isValidSwitchA(this)) {
			MR::listenStageSwitchOnOffA(this, MR::Functor(this, ChooChooTrain::accel), MR::Functor(this, ChooChooTrain::deccel));
			initNerve(&NrvChooChooTrain::NrvWait::sInstance, 0);
		}
		else {
			initNerve(&NrvChooChooTrain::NrvMove::sInstance, 0);
		}

		MR::initAndSetRailClipping(&mRailClipping, this, 100.0f, 500.0f);
		MR::startAction(this, "Run");
		makeActorAppeared();
	}

	void ChooChooTrain::startClipped() {
		LiveActor::startClipped();

		for (s32 i = 0; i < mParts.mCount; i++) {
			mParts.mArray.mArr[i]->startClipped();
		}
	}

	void ChooChooTrain::endClipped() {
		LiveActor::endClipped();

		for (s32 i = 0; i < mParts.mCount; i++) {
			mParts.mArray.mArr[i]->endClipped();
		}
	}

	void ChooChooTrain::control() {
		// Move main part
		TVec3f direction;

		MR::moveCoord(this, mSpeed);
		MR::moveTransToCurrentRailPos(this);
		direction = MR::getRailDirection(this);
		pt::turnToDirectionGravityFront(this, direction);

		// "Move backwards" to get trans-rotate for other parts
		f32 restoreRailCoord = MR::getRailCoord(this);
		MR::reverseRailDirection(this);

		for (s32 i = 0; i < mParts.mCount; i++) {
			ChooChooTrainParts *parts = mParts.mArray.mArr[i];

			MR::moveCoord(this, 1080.0f * mScale.y);
			MR::moveTransToOtherActorRailPos(parts, this);

			direction = -MR::getRailDirection(this);
			pt::turnToDirectionGravityFront(parts, direction);
		}

		// Restore rail movement
		MR::reverseRailDirection(this);
		MR::setRailCoord(this, restoreRailCoord);
	}

	void ChooChooTrain::accel() {
		if (isNerve(&NrvChooChooTrain::NrvWait::sInstance)) {
			setNerve(&NrvChooChooTrain::NrvAccel::sInstance);
		}
	}

	void ChooChooTrain::deccel() {
		if (isNerve(&NrvChooChooTrain::NrvMove::sInstance)) {
			setNerve(&NrvChooChooTrain::NrvDecel::sInstance);
		}
	}

	void ChooChooTrain::setWheelRunRate(f32 rate) {
		MR::setBckRate(this, rate);

		for (s32 i = 0; i < mParts.mCount; i++) {
			MR::setBckRate(mParts.mArray.mArr[i], rate);
		}
	}

	void ChooChooTrain::playMoveLevelSound(f32 rate) {
		s32 step = getNerveStep();

		if (rate > 0.75f || (rate > 0.25f && (step % 10) == 0) || (rate > 0.0f && (step % 20) == 0)) {
			MR::startActionSound(this, "OjLvToySlMove", -1, -1, -1);

			for (s32 i = 0; i < mParts.mCount; i += 2) {
				MR::startActionSound(mParts.mArray.mArr[i], "OjLvToySlMove", -1, -1, -1);
			}
		}
	}

	void ChooChooTrain::exeAccel() {
		if (MR::isFirstStep(this)) {
			MR::startActionSound(this, "OjToySlWhistle", -1, -1, -1);
		}

		f32 rate = MR::calcNerveEaseInRate(this, mAccelTime);
		mSpeed = mMaxSpeed * rate;
		setWheelRunRate(rate);
		playMoveLevelSound(rate);

		if (MR::isStep(this, mAccelTime)) {
			setNerve(&NrvChooChooTrain::NrvMove::sInstance);
		}
	}

	void ChooChooTrain::exeMove() {
		if (MR::isFirstStep(this)) {
			setWheelRunRate(1.0f);
			mSpeed = mMaxSpeed;
		}

		playMoveLevelSound(1.0f);

		if (mWhistleTimer < 0) {
			MR::startActionSound(this, "OjToySlWhistle", -1, -1, -1);
			mWhistleTimer = MR::getRandom((s32)180, (s32)600);
		}
		else {
			mWhistleTimer--;
		}
	}

	void ChooChooTrain::exeDecel() {
		f32 rate = 1.0f - MR::calcNerveEaseInRate(this, mAccelTime);
		mSpeed = mMaxSpeed * rate;
		setWheelRunRate(rate);
		playMoveLevelSound(rate);

		if (MR::isStep(this, mAccelTime)) {
			setNerve(&NrvChooChooTrain::NrvWait::sInstance);
		}
	}

	namespace NrvChooChooTrain {
		void NrvWait::execute(Spine *pSpine) const {
			ChooChooTrain *pActor = (ChooChooTrain*)pSpine->mExecutor;

			if (MR::isFirstStep(pActor)) {
				pActor->setWheelRunRate(0.0f);
				pActor->mSpeed = 0.0f;
			}
		}

		void NrvAccel::execute(Spine *pSpine) const {
			ChooChooTrain *pActor = (ChooChooTrain*)pSpine->mExecutor;
			pActor->exeAccel();
		}

		void NrvMove::execute(Spine *pSpine) const {
			ChooChooTrain *pActor = (ChooChooTrain*)pSpine->mExecutor;
			pActor->exeMove();
		}

		void NrvDecel::execute(Spine *pSpine) const {
			ChooChooTrain *pActor = (ChooChooTrain*)pSpine->mExecutor;
			pActor->exeDecel();
		}

		NrvWait(NrvWait::sInstance);
		NrvAccel(NrvAccel::sInstance);
		NrvMove(NrvMove::sInstance);
		NrvDecel(NrvDecel::sInstance);
	}
}
