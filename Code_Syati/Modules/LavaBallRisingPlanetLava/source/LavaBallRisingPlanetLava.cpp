#include "LavaBallRisingPlanetLava.h"

/*
* Author: Aurum
* Objects: ScaleMapObj class
* Parameters:
*  - Obj_arg0, float, 1000.0f: Minimum scale
*  - Obj_arg1, float, 1100.0f: Maximum scale
*  - Obj_arg2, long, 120: Wait delay
*  - Obj_arg3, long, 240: Scale up time
*  - Obj_arg4, long, 240: Scale down time
*  - Obj_arg5, list, 0: Initial behavior
*   *: Wait small
*   1: Scale up
*   2: Wait big
*   3: Scale down
*  - Obj_arg6, bool, false: Scale only once
* 
* The scaling lava from Melty Molten Galaxy was my first ported object since it appeared to be very easy to port. Since
* its SMG1 counterpart has fixed scales and times, I had to make them editable using a wide set of Obj_arg settings. To
* make this even more flexible, I also added this class as a ProductMapObjDataTable class.
*/

namespace pt {
	LavaBallRisingPlanetLava::LavaBallRisingPlanetLava(const char *pName) : MapObjActor(pName) {
		mMinScale = 1000.0f;
		mMaxScale = 1100.0f;
		mWaitTime = 120;
		mScaleUpTime = 240;
		mScaleDownTime = 240;
		mInitNerve = NERVE_WAIT_SMALL;
		mSavedStep = 0;
		mScaleOnce = false;
		mDoScale = true;
	}

	void LavaBallRisingPlanetLava::init(const JMapInfoIter &rIter) {
		// Initialize object name
		MapObjActor::init(rIter);

		// Load Obj_arg parameters
		MR::getJMapInfoArg0NoInit(rIter, &mMinScale);
		MR::getJMapInfoArg1NoInit(rIter, &mMaxScale);
		MR::getJMapInfoArg2NoInit(rIter, &mWaitTime);
		MR::getJMapInfoArg3NoInit(rIter, &mScaleUpTime);
		MR::getJMapInfoArg4NoInit(rIter, &mScaleDownTime);
		MR::getJMapInfoArg5NoInit(rIter, &mInitNerve);
		MR::getJMapInfoArg6NoInit(rIter, &mScaleOnce);

		// To account for fractions, the Obj_args have to be divided by 1000
		mMinScale /= 1000.0f;
		mMaxScale /= 1000.0f;

		// Depending on the initial nerve type we have to set the proper nerve and scale
		Nerve *nerve;
		f32 scale;

		switch (mInitNerve) {
		default: // NERVE_WAIT_SMALL
			nerve = &NrvLavaBallRisingPlanetLava::NrvWaitSmall::sInstance;
			scale = mMinScale;
			break;
		case NERVE_SCALE_UP:
			nerve = &NrvLavaBallRisingPlanetLava::NrvScaleUp::sInstance;
			scale = mMinScale;
			break;
		case NERVE_WAIT_BIG:
			nerve = &NrvLavaBallRisingPlanetLava::NrvWaitBig::sInstance;
			scale = mMaxScale;
			break;
		case NERVE_SCALE_DOWN:
			nerve = &NrvLavaBallRisingPlanetLava::NrvScaleDown::sInstance;
			scale = mMaxScale;
			break;
		}

		// Initialize MapActor
		MapObjActorInitInfo initInfo;
		initInfo.setupConnectToScene();
		initInfo.setupNerve(nerve);
		initialize(rIter, initInfo);

		// Apply initial scale
		mScale.setAll(scale);
	}

	void LavaBallRisingPlanetLava::initCaseUseSwitchA(const MapObjActorInitInfo &rInitInfo) {
		mDoScale = false;
		MR::listenStageSwitchOnOffA(this, MR::Functor(this, &onScale), MR::Functor(this, &offScale));
	}

	void LavaBallRisingPlanetLava::initCaseUseSwitchB(const MapObjActorInitInfo &rInitInfo) {
		MR::listenStageSwitchOnOffB(this, MR::Functor(this, &offScale), MR::Functor(this, &onScale));
	}

	void LavaBallRisingPlanetLava::onScale() {
		mSpine->mStep = mSavedStep;
		mDoScale = true;
	}

	void LavaBallRisingPlanetLava::offScale() {
		mSavedStep = mSpine->mStep;
		mDoScale = false;
	}

	void LavaBallRisingPlanetLava::exeWait() {
		MR::startActionSound(this, "Loop", -1, -1, -1);

		if (mDoScale && MR::isStep(this, mWaitTime)) {
			if (isNerve(&NrvLavaBallRisingPlanetLava::NrvWaitSmall::sInstance)) {
				setNerve(&NrvLavaBallRisingPlanetLava::NrvScaleUp::sInstance);
			}
			else {
				setNerve(&NrvLavaBallRisingPlanetLava::NrvScaleDown::sInstance);
			}
		}
	}

	void LavaBallRisingPlanetLava::exeScaleUp() {
		MR::startActionSound(this, "Loop", -1, -1, -1);

		if (mDoScale) {
			MR::startActionSound(this, "Scale", -1, -1, -1);

			mScale.setAll(MR::calcNerveValue(this, mScaleUpTime, mMinScale, mMaxScale));

			if (MR::isStep(this, mScaleUpTime)) {
				setNerve(&NrvLavaBallRisingPlanetLava::NrvWaitBig::sInstance);
				mDoScale = !mScaleOnce;
			}
		}
	}

	void LavaBallRisingPlanetLava::exeScaleDown() {
		MR::startActionSound(this, "Loop", -1, -1, -1);

		if (mDoScale) {
			MR::startActionSound(this, "Scale", -1, -1, -1);

			mScale.setAll(MR::calcNerveValue(this, mScaleDownTime, mMaxScale, mMinScale));

			if (MR::isStep(this, mScaleDownTime)) {
				setNerve(&NrvLavaBallRisingPlanetLava::NrvWaitSmall::sInstance);
				mDoScale = !mScaleOnce;
			}
		}
	}

	namespace NrvLavaBallRisingPlanetLava {
		void NrvWaitSmall::execute(Spine *pSpine) const {
			LavaBallRisingPlanetLava *pActor = (LavaBallRisingPlanetLava*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvWaitBig::execute(Spine *pSpine) const {
			LavaBallRisingPlanetLava *pActor = (LavaBallRisingPlanetLava*)pSpine->mExecutor;
			pActor->exeWait();
		}

		void NrvScaleUp::execute(Spine *pSpine) const {
			LavaBallRisingPlanetLava *pActor = (LavaBallRisingPlanetLava*)pSpine->mExecutor;
			pActor->exeScaleUp();
		}

		void NrvScaleDown::execute(Spine *pSpine) const {
			LavaBallRisingPlanetLava *pActor = (LavaBallRisingPlanetLava*)pSpine->mExecutor;
			pActor->exeScaleDown();
		}

		NrvWaitSmall(NrvWaitSmall::sInstance);
		NrvWaitBig(NrvWaitBig::sInstance);
		NrvScaleUp(NrvScaleUp::sInstance);
		NrvScaleDown(NrvScaleDown::sInstance);
	}
}
