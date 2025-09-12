#pragma once

#include "syati.h"

namespace pt {
	class IceStepNoSlip : public ModelObj {
	public:
		IceStepNoSlip(MtxPtr pMtx);

		virtual void init(const JMapInfoIter &rIter);
		virtual void appear();

		void exeAppear();
		void exeBreak();
	};

	class WaterLeakPipe : public LiveActor {
	public:
		WaterLeakPipe(const char *pName);

		virtual void init(const JMapInfoIter &rIter);
		virtual void calcAnim();
		virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

		void exeWait();
		void exeFreeze();
		void initPipeHeight();

		IceStepNoSlip* mIceStep;
		f32 mPipeLength;
		MtxPtr mIceStepMtx;
		TVec3f mClippingOffset;
	};

	namespace NrvIceStepNoSlip {
		NERVE(NrvAppear);
		NERVE(NrvBreak);
	};

	namespace NrvWaterLeakPipe {
		NERVE(NrvWait);
		NERVE(NrvFreeze);
	};
};
