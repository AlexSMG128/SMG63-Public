#pragma once

#include "syati.h"

namespace pt {
	class DharmaSamboParts;

	class DharmaSambo : public LiveActor {
	public:
		DharmaSambo(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void initAfterPlacement();
		virtual void appear();
		virtual void kill();
		virtual void control();
		virtual void calcAndSetBaseMtx();

		void exeWaitUnderGround();
		void exeAppear();
		void exePursue();
		void exeAttack();
		void exeSpinHitted();
		void exeStarPointerBind();
		void endStarPointerBind();
		void exeEscapeRun();
		void exeEscapeLand();
		void exeTrampleDeath();
		void exeHide();
		void exeHideAppear();
		void exeKilled();

		DharmaSamboParts* getHeadParts();
		bool isActionEndHead();
		void startActionHead(const char *pName);
		void startActionAllParts(const char *pName);
		void startBtkAllParts(const char *pName);
		bool decCountAndTryToHide(bool hide);
		bool isStarPointerPointing();

		const Nerve* mResumeStarPointerBindNerve;
		TVec3f mHomePos;
		s32 mHideCounter;
		s32 mRemainingParts;
		MR::Vector<MR::AssignableArray<DharmaSamboParts*> > mParts;
		MR::Vector<MR::AssignableArray<FixedPosition*> > mPositions;
		HitSensor* mBlowAttackerSensor;
		AnimScaleController* mAnimScaleCtrl;
		AnimScaleParam* mAnimScaleParam;
		f32 mAppearRange;
	};

	class DharmaSamboParts : public ModelObj {
	public:
		DharmaSamboParts(DharmaSambo *pHost, const char *pName, const char *pModelName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void appear();
		virtual void kill();
		virtual void control();
		virtual void calcAndSetBaseMtx();
		virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
		virtual bool receiveMsgPlayerAttack(u32, HitSensor *pSender, HitSensor *pReceiver);

		void exeBlow();
		void exeStarPointerBind();
		void endStarPointerBind();
		void setHead(bool isHead);

		DharmaSambo* mHost;
		s32 mOriginalPosIndex;
		s32 mActualPosIndex;
		SpinHitController* mSpinHitCtrl;
		f32 mBlowAttackedDistance;
		bool mIsHead;
	};

	namespace NrvDharmaSambo {
		NERVE(NrvWaitUnderGround);
		NERVE(NrvAppear);
		NERVE(NrvAppearImmediately);
		ENDABLE_NERVE(NrvPursue);
		NERVE(NrvAttack);
		NERVE(NrvHide);
		NERVE(NrvHideAppear);
		NERVE(NrvSpinHitted);
		NERVE(NrvKilled);
		ENDABLE_NERVE(NrvStarPointerBind);
		NERVE(NrvEscapeRun);
		NERVE(NrvEscapeLand);
		NERVE(NrvEscapeWaitRun);
		NERVE(NrvEscapeWaitLand);
		NERVE(NrvTrampleDeath);
	};

	namespace NrvDharmaSamboParts {
		NERVE(NrvDoNothing);
		NERVE(NrvBlow);
		ENDABLE_NERVE(NrvStarPointerBind);
	};
};
