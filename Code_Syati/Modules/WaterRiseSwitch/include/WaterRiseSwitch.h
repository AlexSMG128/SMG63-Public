#pragma once

#include "syati.h"

#define EXT_SCENE_OBJ_WATER_RISE_HOLDER 71
#define MAX_GROUPS 4

namespace pt {
	class WaterRiseHolder : public NameObj {
		public:
		WaterRiseHolder(const char* pName);

		LiveActorGroup* mGroups[MAX_GROUPS];
	};

	NameObj* createWaterRiseHolder();
	WaterRiseHolder* getWaterRiseHolder();
	
	class WaterRiseSwitch : public LiveActor {
	public:
		WaterRiseSwitch(const char *pName);
		virtual void init(const JMapInfoIter& rIter);
        virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual void control();
		void exeRiseWater();

		s32 mWaterRiseGroup;
		f32 mSpeed;
		f32 mOffsetY;
		f32 mCurrentBckFrame;
		bool mIsRiseActive;
		bool mIsValidDist;
    };
};