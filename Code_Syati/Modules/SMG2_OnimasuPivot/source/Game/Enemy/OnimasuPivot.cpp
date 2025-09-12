#include "Game/Enemy/OnimasuPivot.h"

#include "JSystem/JGeometry/TUtil.h"
#include "Game/Util/RailUtil.h"
#include "Game/Util/MathUtil.h"

OnimasuPivot::OnimasuPivot(const char* pName) : Onimasu(pName) {
    mNextPointNo = 0;
    mRailPoints = 0;
    mRailPointsLength = 0;
    ((TVec4f)_110).set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    ((TVec4f)_120).set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
}

void OnimasuPivot::initFromRailPoint() {
    s32 numRailPoint = MR::getRailPointNum(this) / 2;
    mRailPoints = new TVec3f[numRailPoint];
    mRailPointsLength = numRailPoint;
}

inline TVec3f getRailPointPos(LiveActor* pActor, s32 n) {
    TVec3f ret;
    MR::calcRailPointPos(&ret, pActor, n);
    return ret;
}

void OnimasuPivot::startMoveInner() {
    TRot3f stack_50;
    TVec3f stack_44;
    MR::calcRailPointPos(&stack_44, this, getLastPointNo() * 2 + 1);

    TVec3f stack_38 = getRailPointPos(this, getLastPointNo() * 2) - stack_44;
    TVec3f stack_2C = getRailPointPos(this, mNextPointNo * 2) - stack_44;

    if (MR::isSameDirection(stack_38, stack_2C, 0.01f)) {
        stack_50.identity();
        stack_50.setRotate(_D4, 0.00314159295521677f); //3.14159295521676540374755859375E-3
        stack_50.mult(stack_38, stack_38);
    }
    MR::makeQuatFromVec(&_110, stack_38, _D4);
    MR::makeQuatFromVec(&_120, stack_2C, _D4);
}

/* nonmatching */
void OnimasuPivot::updatePoseInner() {
    f32 nextTimePoint = getTimeToNextPoint();
    f32 ratio = mParamScale->getNerveStepRate() / nextTimePoint;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    }

    TQuat4f stack_20(_110);
    stack_20.slerp(_120, ratio);
    TVec3f stack_14(TVec3f(0.0f));


    f32 f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10,
        f11, f12, f13, f14, f15, f16, f17, f18, f19,
        f20, f21, f22, f23, f24, f25, f26, f27, f28,
        f29, f30, f31;



    f3 = stack_20.x;
    f1 = stack_20.z;
    f0 = f3 * f3;
    f6 = stack_20.y;
    f2 = stack_20.w;
    f7 = f3 * f1;
    f8 = 2.0f;
    f4 = f6 * f1;
    f5 = f2 * f6;
    f1 = 1.0f;
    f3 = f2 * f3;
    f2 = f8 * f0;
    f0 = f6 * f6;
    f6 = f8 * f5;
    f5 = f8 * f4;
    f4 = f8 * f3;
    f3 = f1 - f2;
    f7 = f8 * f7;
    f0 = f8 * f0;
    f2 = f5 - f4;
    f1 = f7 + f6;
    f3 = f3 - f0;

    stack_14.set(f1, f2, f3);


    f32 squareMag = stack_14.squared();
    if (squareMag <= 0.0000038146973f) {
    }
    else
    {
        f1 = JGeometry::TUtil<f32>::inv_sqrt(squareMag);
        f3 = f1 * 565.68542f;
        stack_14 *= f3;
    }
    TVec3f stack_8;
    MR::calcRailPointPos(&stack_8, this, getLastPointNo() * 2 + 1);
    stack_14 += stack_8;
    mTranslation.set(stack_14);
}

int OnimasuPivot::getNextPointNo() const {
    return mNextPointNo * 2;
}

int OnimasuPivot::getLastPointNo() const {
    int ret = mNextPointNo - 1;
    if (ret < 0) {
        ret = MR::getRailPointNum(this) / 2 - 1;
    }
    return ret;
}

TVec3f* OnimasuPivot::getLastPointNormal() const {
    u32 idx = getLastPointNo();
    return &mRailPoints[idx];
}

TVec3f* OnimasuPivot::getNextPointNormal() const {
    return &mRailPoints[mNextPointNo];
}

void OnimasuPivot::incrementNextPoint() {
    mNextPointNo++;
    if (mNextPointNo == MR::getRailPointNum(this) / 2) {
        mNextPointNo = 0;
    }
}

void OnimasuPivot::collectRailPointInfo() {
    for (int i = 0; i < mRailPointsLength; i++) {
        TVec3f stack_14(TVec3f(0.0f));
        TVec3f stack_8(TVec3f(0.0f));
        OnimasuFunction::getPolygonOnRailPoint(&stack_8, &stack_14, this, i * 2);
        mRailPoints[i].set(stack_14);
    }
}