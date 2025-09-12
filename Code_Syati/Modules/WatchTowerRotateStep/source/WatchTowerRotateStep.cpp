#include "WatchTowerRotateStep.h"

/*
* Authors: Someone, Aurum (shibbo provided assistance)
* Objects: WatchTowerRotateStep
* Parameters:
*  - Obj_arg0, float, 300.0f: Rotation speed
*  - cutscene
*  - SW_AWAKE
* 
* The rotating windmill with four platforms from Honeyhive Galaxy. The four platforms are attached to the pegs at each
* blade. Unlike SMG1, the rotation speed can be changed through Obj_args now. It is also compatible with an optional
* cutscene setup.
*/

namespace pt {
	WatchTowerRotateStep::WatchTowerRotateStep(const char *pName) : LiveActor(pName) {
		mRotSpeed = 300.0f;
	}

	void WatchTowerRotateStep::init(const JMapInfoIter &rIter) {
		MR::initDefaultPos(this, rIter);
		MR::processInitFunction(this, rIter, false);
		MR::connectToSceneMapObj(this);

		// Initialize sensors and collision
		initHitSensor(1);
		HitSensor *senBody = MR::addBodyMessageSensorMapObj(this);
		MR::initCollisionParts(this, "WatchTowerRotateStep", senBody, NULL);

		// Initialize effects, sounds and clipping
		initEffectKeeper(0, NULL, false);
		initSound(4, "WatchTowerRotateStep", false, TVec3f(0.0f, 0.0f, 0.0f));
		MR::setClippingTypeSphereContainsModelBoundingBox(this, 1500.0f);

		// Try to register cutscene setup
		bool registerDemoCast = MR::tryRegisterDemoCast(this, rIter);
		if (registerDemoCast) {
			MR::registerDemoActionNerve(this, &NrvWatchTowerRotateStep::NrvMoveStart::sInstance, NULL);
		}

		// Initialize rotation and lift
		MR::getJMapInfoArg0NoInit(rIter, &mRotSpeed);
		mRotSpeed /= 1000.0f;
		MR::calcUpVec(&mRotateDeg, this);

		initLift(rIter);

		if (registerDemoCast) {
			initNerve(&NrvWatchTowerRotateStep::NrvWait::sInstance, 0);
		}
		else {
			initNerve(&NrvWatchTowerRotateStep::NrvMove::sInstance, 0);
		}

		MR::useStageSwitchAwake(this, rIter);
		makeActorAppeared();
	}

	void WatchTowerRotateStep::calcAndSetBaseMtx() {
		TVec3f frontVec;
		TMtx34f baseMtx;

		MR::calcFrontVec(&frontVec, this);
		MR::makeMtxFrontUpPos((TPos3f*)&baseMtx, frontVec, mRotateDeg, mTranslation);
		MR::setBaseTRMtx(this, *(TPos3f*)&baseMtx);
	}

	void WatchTowerRotateStep::initLift(const JMapInfoIter &rIter) {
		for (s32 i = 0; i < 4; i++) {
			PartsModel *platform = new PartsModel(this, "物見の塔リフト", "WatchTowerRotateStepLift", MR::getJointMtx(this, i + 1), -1, false);
			mParts[i] = platform;

			// Calculate initial base matrix and disable calculating its own base matrix
			platform->mCalcOwnMatrix = false;

			MR::initCollisionParts(platform, "WatchTowerRotateStepLift", getSensor(NULL), NULL);

			// Initialize shadow
			platform->initShadowControllerList(1);
			MR::addShadowVolumeBox(platform, "Body", TVec3f(600.0f, 200.0f, 400.0f), (MtxPtr)platform->getBaseMtx());
			MR::setShadowVolumeStartDropOffset(platform, "WatchTowerRotateStepLift", 300.0f);
			MR::setShadowDropLength(platform, "WatchTowerRotateStepLift", 370.0f);

			if (MR::isDemoCast(this, NULL)) {
				MR::tryRegisterDemoCast(platform, rIter);
			}

			MR::useStageSwitchAwake(platform, rIter);

			platform->initWithoutIter();
		}
	}

	void WatchTowerRotateStep::exeMove() {
		TVec3f frontVec;
		MR::calcFrontVec(&frontVec, this);
		MR::rotateVecDegree(&mRotateDeg, frontVec, mRotSpeed);

		attachLift();

		MR::startActionSound(this, "OjLvWatchTowerRotate", -1, -1, -1);
	}

	void WatchTowerRotateStep::exeMoveStart() {
		if (MR::isFirstStep(this)) {
			MR::startSystemSE("SE_SY_READ_RIDDLE_S", -1, -1);
			MR::startActionSound(this, "OjWatchTowerStart", -1, -1, -1);
		}

		MR::startActionSound(this, "OjLvWatchTowerRotate", -1, -1, -1);

		TVec3f frontVec;
		f32 rotateAngle = MR::getEaseInValue(getNerveStep(), 0.0f, mRotSpeed, 180.0f);
		MR::calcFrontVec(&frontVec, this);
		MR::rotateVecDegree(&mRotateDeg, frontVec, rotateAngle);

		attachLift();

		if (MR::isStep(this, 180)) {
			setNerve(&NrvWatchTowerRotateStep::NrvMove::sInstance);
		}
	}

	void WatchTowerRotateStep::attachLift() {
		for (s32 i = 0; i < 4; i++) {
			MtxPtr jointMtx = MR::getJointMtx(this, i + 1);
			PartsModel *platform = mParts[i];
			MR::extractMtxTrans(jointMtx, &platform->mTranslation);
		}
	}

	namespace NrvWatchTowerRotateStep {
		void NrvMove::execute(Spine *pSpine) const {
			WatchTowerRotateStep *pActor = (WatchTowerRotateStep*)pSpine->mExecutor;
			pActor->exeMove();
		}

		void NrvMoveStart::execute(Spine *pSpine) const {
			WatchTowerRotateStep *pActor = (WatchTowerRotateStep*)pSpine->mExecutor;
			pActor->exeMoveStart();
		}

		void NrvWait::execute(Spine *pSpine) const {}

		NrvMove(NrvMove::sInstance);
		NrvMoveStart(NrvMoveStart::sInstance);
		NrvWait(NrvWait::sInstance);
	}
}
