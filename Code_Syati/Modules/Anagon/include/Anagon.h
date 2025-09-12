#pragma once

#include "syati.h"

namespace pt {
	class Anagon : public LiveActor {
	public:
		Anagon(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void makeActorAppeared();
		virtual void kill();
		virtual void control();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void exeSwim();
		void exeBind();
		void endBind();
		void exeBreak();
		void exeDead();

		bool isStarPointerPointing();

		LiveActor* mStarPointerTargets[3];
		AnimScaleController* mAnimScaleCtrl;
	};

	namespace NrvAnagon {
		NERVE(NrvSwim);
		ENDABLE_NERVE(NrvBind);
		NERVE(NrvBreak);
		NERVE(NrvDead);
	};
};
