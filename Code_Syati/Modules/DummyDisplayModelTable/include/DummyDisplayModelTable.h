#pragma once

#include "syati.h"

namespace BlueCoinUtil {
    extern bool isBlueCoinGotCurrentFile(u16);
};

class DummyDisplayModelExt : public DummyDisplayModel {
public:
    DummyDisplayModelExt(LiveActor *pHost, const DummyDisplayModelInfo *pInfo, int i, s32 l1, s32 l2);
    s32 mRotateType;
    s32 mSetupType;
    s32 mNoLightCtrl;
    s32 mBehaviorType;
};

namespace pt {
    extern void* loadArcAndFile(const char *pArc, const char *pFile, JKRHeap *pHeap);
    
    s32 calcTargetLine(JMapInfo table, s32 modelId);
    s32 createInfoFromTable(DummyDisplayModelInfo* pInfo, JMapInfo table, s32 modelId);
};

namespace MR {
    bool isStarCollected(const NameObj*, s32, bool*);
}