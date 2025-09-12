#pragma once

#include "revolution/types.h"

// Hash Listing so I don't forget...
// 
// BattleData.bcsv
// - FollowerGroupId (group id | -1 to disable)
// - AllowFollowerReappear (o or 1)
// - PowerUpDemo (name)
// - Barrier (name)
// - MovementType (0 = Normal Move | 1 = Teleport Move)
//
// BeamPatternLvX.bcsv
// - Beam
// - Count

class KameckBeamHolder;
class LiveActor;

struct BossKameckBattleBeamEntry {
	s32 mBeamType;
	s32 mCount;
	KameckBeamHolder* mBeamHolderRef; // This is a REFERENCE to a KameckBeamHolder that is owned by BossKameck. This is to prevent unneccesary duplicates... UNLESS you're accessing this FROM a BossKameck... in which case it's the master... Thank goodness Nintendo just completely destroys the scene heap...
};

struct BossKameckBattlePattern {
	s32 mFollowerGroupId;
	bool mFollowerAllowRespawn;
	const char* mPowerUpAnimName;
	LiveActor* mBarrier;
	s32 mMovementType;

	BossKameckBattleBeamEntry** mBeamInfo;
	s32 mBeamInfoCount;
	s32 mBeamInfoCurrentIdx;
};

struct BossKameckBarrierEntry {
	const char* mName;
	LiveActor* mActor;
};