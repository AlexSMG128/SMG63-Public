#pragma once

#include "syati.h"

#define NERVE_WAIT_SMALL 0
#define NERVE_SCALE_UP 1
#define NERVE_WAIT_BIG 2
#define NERVE_SCALE_DOWN 3

namespace pt {
	class LavaBallRisingPlanetLava : public MapObjActor {
	public:
		LavaBallRisingPlanetLava(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void initCaseUseSwitchA(const MapObjActorInitInfo &rInitInfo);
		virtual void initCaseUseSwitchB(const MapObjActorInitInfo &rInitInfo);

		void onScale();
		void offScale();
		void exeWait();
		void exeScaleUp();
		void exeScaleDown();

		f32 mMinScale;
		f32 mMaxScale;
		s32 mWaitTime;
		s32 mScaleUpTime;
		s32 mScaleDownTime;
		s32 mInitNerve;
		s32 mSavedStep;
		bool mScaleOnce;
		bool mDoScale;
	};

	namespace NrvLavaBallRisingPlanetLava {
		NERVE(NrvWaitSmall);
		NERVE(NrvWaitBig);
		NERVE(NrvScaleUp);
		NERVE(NrvScaleDown);
	};
};
