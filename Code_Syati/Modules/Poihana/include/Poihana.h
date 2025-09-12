#pragma once

#include "syati.h"

#define POIHANA_BEHAVIOR_NORMAL 0
#define POIHANA_BEHAVIOR_SLEEP 1
#define POIHANA_BEHAVIOR_NEW_HOME 2

class ActorCameraInfo;

namespace pt {
	class Poihana : public LiveActor {
	public:
		Poihana(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void initAfterPlacement();
		virtual void control();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPush(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveOtherMsg(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void exeNonActive();
		void endNonActive();
		void exeWait();
		void exeWalkAround();
		void exeSleepStart();
		void exeSleep();
		void exeGetUp();
		void exeSearch();
		void exeChasePlayer();
		void exeShootUpCharge();
		void exeShootUp();
		void endShootUp();
		void exeGoBack();
		void exeShock();
		void exeSwoonLand();
		void exeSwoon();
		void exeRecover();
		void exeShake();
		void exeDrown();
		void exeHide();
		void exeAppear();
		void exeDPDSwoon();
		void endDPDSwoon();

		void updateBinderSensor();
		void updateTrampleBinderSensor();
		bool tryToStartBind(HitSensor *pSender);
		void updateBindActorMtx();
		void endBind();
		void startBound();
		void calcBound();
		void contactMario(HitSensor *pSender, HitSensor *pReceiver);
		void controlVelocity();
		void calcMyGravity();
		bool tryNonActive();
		bool tryDrown();
		bool tryDPDSwoon();
		bool tryShock();
		bool tryHipDropShock();
		bool isNerveTypeWalkOrWait() const;
		bool isNeedForBackHome() const;
		bool isNeedForGetUp() const;
		bool isBackAttack(HitSensor *pMySensor) const;

		AnimScaleController* mAnimScaleCtrl;
		WalkerStateBindStarPointer* mBindStarPointer;
		ActorCameraInfo* mCamInfo;
		LiveActor* mBindedActor;
		f32 mLaunchIntensity;
		f32 mActiveRange;
		TVec3f mRespawnPos;
		TVec3f mHomePos;
		TVec3f mFrontVec;
		s32 mBoundTimer;
		s32 mRandDir;
		s32 mBehavior;
		bool mCanDrown;
		bool mIsActive;
		ModelObj* mWaterColumn;
	};

	namespace NrvPoihana {
		ENDABLE_NERVE(NrvNonActive);
		NERVE(NrvWait);
		NERVE(NrvWalkAround);
		NERVE(NrvSleepStart);
		NERVE(NrvSleep);
		NERVE(NrvGetUp);
		NERVE(NrvSearch);
		NERVE(NrvChasePlayer);
		NERVE(NrvShootUpCharge);
		ENDABLE_NERVE(NrvShootUp);
		NERVE(NrvGoBack);
		NERVE(NrvShock);
		NERVE(NrvSwoon);
		NERVE(NrvSwoonLand);
		NERVE(NrvRecover);
		NERVE(NrvShake);
		NERVE(NrvDrown);
		NERVE(NrvHide);
		NERVE(NrvAppear);
		ENDABLE_NERVE(NrvDPDSwoon);
	};
};
