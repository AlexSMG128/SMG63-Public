#pragma once

#include "syati.h"

// ObjArg0 Color Frame
// ObjArg1 Texture Frame
// ObjArg2 Behaviour
// ObjArg3 NpcGoods Equipment
// ObjArg4 Instance Settings

class SimpleNPC : public NPCActor {
public:
	SimpleNPC(const char* pName);

	virtual void init(const JMapInfoIter& rIter);

	virtual void initNPCData(const JMapInfoIter& rIter);
	virtual void initBehaviourData(const JMapInfoIter& rIter);

	TakeOutStar* mTakeOutStar;
};

namespace NrvSimpleNPC
{
	NERVE(NrvNull);
	NERVE(NrvTalk);
	NERVE(NrvWait);
	NERVE(NrvReaction);
}

namespace MR {
	void getParamDataS32(s32* pDest, const JMapInfo* pCsvData, s32 ParamID, s32 Row);
	void getParamDataF32(f32* pDest, const JMapInfo* pCsvData, s32 ParamID, s32 Row);
}