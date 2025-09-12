#pragma once

#include "Game/Enemy/Onimasu.h"

class OnimasuPivot : public Onimasu {
public:
    OnimasuPivot(const char*);

    void initFromRailPoint();
    void incrementNextPoint();
    void collectRailPointInfo();
    int getNextPointNo() const;
    int getLastPointNo() const;
    TVec3f* getLastPointNormal() const;
    TVec3f* getNextPointNormal() const;
    void startMoveInner();
    void updatePoseInner();

    // Offsets are SMG1 offsets
    s32 mNextPointNo; // 0x104
    TVec3f* mRailPoints; // 0x108
    s32 mRailPointsLength; // 0x10C
    TQuat4f _110;
    TQuat4f _120;
};