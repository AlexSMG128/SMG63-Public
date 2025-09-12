#pragma once

#include "MoguStone.h"

namespace pt {
	class Mogu : public LiveActor {
	public:
		Mogu(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void kill();
		virtual void endClipped();
		virtual void control();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void exeHideWait();
		void exeHide();
		void exeAppear();
		void exeSearch();
		void exeThrow();
		void tearDownThrow();
		void exeSwoonStart();
		void exeSwoon();
		void exeSwoonEnd();
		void exeHipDropReaction();
		void exeStampDeath();
		void exeHitBlow();
		void exeDPDSwoon();
		void endDPDSwoon();

		bool isPlayerExistUp() const;
		bool isNearPlayerHipDrop() const;
		bool isNerveTypeInactive() const;
		bool tryPunchHitted(HitSensor *pSender, HitSensor *pReceiver);

		AnimScaleController* mAnimScaleCtrl;
		WalkerStateBindStarPointer* mBindStarPointer;
		FixedPosition* mArmPos;
		MoguStone* mStone;
		ModelObj* mHole;
		TVec3f mFrontVec;
		TVec3f mUpVec;
		bool mCalcStoneGravity;
		bool mIsCannonFleetSight;
	};

	namespace NrvMogu {
		NERVE(NrvHideWait);
		NERVE(NrvHide);
		NERVE(NrvAppear);
		NERVE(NrvSearch);
		NERVE(NrvTurn);
		ENDABLE_NERVE(NrvThrow);
		NERVE(NrvSwoonStart);
		NERVE(NrvSwoonEnd);
		NERVE(NrvSwoon);
		NERVE(NrvHipDropReaction);
		NERVE(NrvStampDeath);
		NERVE(NrvHitBlow);
		ENDABLE_NERVE(NrvDPDSwoon);
	};
};
