#pragma once

#include "syati.h"

namespace pt {
	class BallBeamer : public LiveActor {
	public:
		BallBeamer(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void control();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void syncSwitchOnA();
		void syncSwitchOffA();
		void syncSwitchOnB();
		void setupAttack();
		void exeWait();
		void exeAttack();
		void exeInter();
		void exeBind();

		WalkerStateBindStarPointer* mBindStarPointer;
		ActorStateParamScale* mParamScale;
		RingBeam** mBeams;
		s32 mNumMaxBeams;
		s32 mAttackDuration;
		f32 mActiveRange;
		bool mActive;
		TMtx34f mHeadMtx;
	};

	namespace NrvBallBeamer {
		NERVE(NrvWait);
		NERVE(NrvAttack);
		NERVE(NrvInter);
		NERVE(NrvBind);
	};
};
