#pragma once

#include "Game/LiveActor/LiveActorGroup.h"
#include "Game/Enemy/Kameck.h"

class BossKameckFollowerGroup : public NameObj {
public:
	BossKameckFollowerGroup(const char* pName);

	virtual void init(const JMapInfoIter& rIter);

	void startDemoAppear();
	void endDemoAppear();
	void deadForceAll();
	void reviveAll();

	s32 getLivingActorNum() const;

	s32 mId;
	DeriveActorGroup<Kameck>* mHolder;
};