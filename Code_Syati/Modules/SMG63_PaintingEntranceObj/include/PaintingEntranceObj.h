#pragma once

#include "syati.h"

class PaintingEntranceObj : public LiveActor {
public:
	PaintingEntranceObj(const char*);
	virtual void init(const JMapInfoIter&);
	virtual void initAfterPlacement();
	void exeWait();
	void exeEnter();
	void exeExit();

	ActorCameraInfo* mCameraInfo;
	s32 mTargetDest;
	s32 mZoneID;
};

namespace NrvPaintingEntranceObj {
	NERVE(NrvWait);
	NERVE(NrvEnter);
	NERVE(NrvExit);
}