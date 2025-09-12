#pragma once

#include "syati.h"

namespace pt {
	class CocoSamboHead : public PartsModel {
	public:
		CocoSamboHead(LiveActor *pHost);

		virtual void init(const JMapInfoIter &rIter);
		virtual void kill();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void updateFrontVecToPlayer(const TVec3f &rGravity);
		void exeFall();
		void exeFallLand();
		void exeSwoon();
		void exeSwoonEnd();
		void exeBlow();

		TVec3f mFrontVec;
		f32 mHitBackVelocity;
	};

	class CocoSambo : public LiveActor {
	public:
		CocoSambo(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void appear();
		virtual void kill();
		virtual void control();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		bool tryToFallHead(const HitSensor *pHit1, const HitSensor *pHit2);
		bool tryToPress();
		bool tryToBlow();
		bool tryStarPieceReflect();
		void dirToPlayer(f32);
		bool tryDpdPointing(const Nerve *pAfterDpdNerve);
		bool isStarPointerPointing();
		void emitDeath();

		void exeHideWait();
		void exeAppear();
		void exeWait();
		void exeHide();
		void exeAttack();
		void exeAttackInterval();
		void exeFallHead();
		void exeFallHeadHide();
		void exeRecoverWait();
		void exeRecover();
		void exePressY();
		void exeBlow();
		void exeDpdPointing();
		void endDpdPointing();

		CocoSamboHead* mHead;
		TVec3f mHitEffectPos;
		TVec3f mFrontVec;
		TVec3f mUpVec;
		f32 mAppearRange;
		f32 mAttackRange;
		AnimScaleParam* mAnimScaleParam;
		AnimScaleController* mAnimScaleCtrl;
		const Nerve* mAfterDpdNerve;
		bool mHasItems;
		bool mNoHideFlower;
	};

	namespace NrvCocoSamboHead {
		NERVE(NrvHeadConnectedBody);
		NERVE(NrvHeadFall);
		NERVE(NrvHeadFallLand);
		NERVE(NrvHeadSwoon);
		NERVE(NrvHeadSwoonEnd);
		NERVE(NrvHeadBlow);
	};

	namespace NrvCocoSambo {
		NERVE(NrvHideWait);
		NERVE(NrvAppear);
		NERVE(NrvWait);
		NERVE(NrvHide);
		NERVE(NrvAttack);
		NERVE(NrvAttackInterval);
		NERVE(NrvFallHead);
		NERVE(NrvFallHeadHide);
		NERVE(NrvRecoverWait);
		NERVE(NrvRecover);
		NERVE(NrvPressY);
		NERVE(NrvBlow);
		ENDABLE_NERVE(NrvDpdPointing);
	};
};
