#pragma once

#include "syati.h"

namespace pt {
	void moveAndTurnToPlayer(LiveActor *pActor, const MR::ActorMoveParam &rParam);
	void moveAndTurnToDirection(LiveActor *pActor, const TVec3f &rDir, const MR::ActorMoveParam &rParam);
	void turnToDirectionUpFront(LiveActor *pActor, TVec3f rUp, TVec3f rFront);
	void turnToDirectionGravityFront(LiveActor *pActor, TVec3f rFront);

	void initRailToNearestAndRepositionWithGravity(LiveActor *pActor);
	void getTransRotateFromRailWithGravity(LiveActor *pActor);
	bool isPlayerFrontSide(const LiveActor *pActor);

	AnimScaleController* createSamboAnimScaleController(AnimScaleParam *pAnimScaleParam);

	void* loadArcAndFile(const char *pArc, const char *pFile, JKRHeap *pHeap);
	void initShadowVolumeBox(LiveActor* pActor, const TVec3f& rPos);

	LiveActor* getSpecificActorFromGroup(LiveActor* pActor, const char* pName);

	NameObj *createObjByName(const char *pName);
	NameObj *createObjByNameAndInitWithoutIter(const char *pName);
};
