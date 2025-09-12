#pragma once

#include "syati.h"

namespace pt {
	class JumpGuarder : public JumpEmitter {
	public:
		JumpGuarder(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void control();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveOtherMsg(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void exeHide();
		void exeUp();
		void exeWait();
		void exeDown();
		void exeHopStart();
		void exeHopWait();
		void exeHopJump();
		void exeHopEnd();
		void exePreOpen();
		void exeOpen();
		void exeClose();
		void exeInter();
		bool enableAttack();
		bool isHit(const LiveActor *pActor) const;
		void appearBabyFromGuarder(BegomanBaby *pBaby);

		BegomanBaby** mBabys;
		s32 mNumBabys;
		s32 mStaggerDelay;
		BegomanBaby* mPendingBabys[4];
		s32 mNumPendingBabys;
		f32 mLaunchVelocity;
	};

	namespace NrvJumpGuarder {
		NERVE(NrvHide);
		NERVE(NrvUp);
		NERVE(NrvWait);
		NERVE(NrvDown);
		NERVE(NrvHopStart);
		NERVE(NrvHopWait);
		NERVE(NrvHopJump);
		NERVE(NrvHopEnd);
		NERVE(NrvPreOpen);
		NERVE(NrvOpen);
		NERVE(NrvClose);
		NERVE(NrvInter);
	};
};
