#pragma once

#include "syati.h"

class PaintingEntranceObj : public LiveActor {
public:
	PaintingEntranceObj(const char*);

	virtual void init(const JMapInfoIter&);

	virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

	CollisionParts* mEntranceCollision;
	ActorCameraInfo* mCameraInfo;
	s32 mCooldown;
};

namespace NrvPaintingEntranceObj {
	NERVE(NrvWait);
	NERVE(NrvEnter);
	NERVE(NrvExit);
}