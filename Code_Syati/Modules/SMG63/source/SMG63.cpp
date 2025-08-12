#include "syati.h"

void SnowBallDieInWater(LiveActor* pActor, const TVec3f& rPos1, const TVec3f& rPos2) {
	TPos3f* pPos = ((TPos3f*)pActor+0x3);
	MR::makeMtxUpNoSupportPos(pPos, rPos1, rPos2);
	TVec3f vec;
	pPos->getTrans(vec);
	if (MR::isInWater(vec)) {
		pActor->kill();
		MR::emitEffect(pActor, "Break");
		MR::startActionSound(pActor, "Break", -1, -1, -1);
	}
}

kmWrite32(0x80301C64, 0x7FA3EB78);
kmCall(0x80301C6C, SnowBallDieInWater);


kmWrite32(0x803CB1F0, 0x60000000);

const char* KoopaFight1LoadResources() {
	return "BowserLV1Galaxy";
}

kmCall(0x803EC5A4, KoopaFight1LoadResources);


bool is63StageKoopaVs() {
	return MR::isEqualStageName("BowserLV1Galaxy") || MR::isEqualStageName("BowserLV2Galaxy") || MR::isEqualStageName("BowserLV3Galaxy");
}

kmCall(0x803B7A1C, is63StageKoopaVs);