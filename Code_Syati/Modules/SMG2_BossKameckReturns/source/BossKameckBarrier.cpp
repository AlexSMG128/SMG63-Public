#include "BossKameckBarrier.h"

#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActorSensorUtil.h"
#include "Game/Util/ActorInitUtil.h"
#include "Game/Util/LiveActorUtil.h"
#include "Game/Util/ObjUtil.h"
#include "Game/Util/JMapUtil.h"

#include "Game/LiveActor/HitSensor.h"

BossKameckBarrier::BossKameckBarrier(const char* pName) : LiveActor(pName) {
}

void BossKameckBarrier::init(const JMapInfoIter& rIter) {
    const char* pObjectName;
    MR::getObjectName(&pObjectName, rIter);
    MR::initDefaultPos(this, rIter);
    initModelManagerWithAnm(pObjectName, NULL, NULL, false);
    MR::connectToSceneEnemy(this);
    initHitSensor(1);
    TVec3f offs;
    offs.x = 0.0f;
    offs.y = 0.0f;
    offs.z = 0.0f;
    MR::addHitSensor(this, "Body", ATYPE_DELETED_MAPOBJ_57, 0, 0.0f, offs);
    MR::initCollisionParts(this, pObjectName, getSensor("Body"), NULL);
    initNerve(&NrvBossKameckBarrier::BossKameckBarrierNrvWait::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void BossKameckBarrier::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

BossKameckBarrier::~BossKameckBarrier() {

}

namespace NrvBossKameckBarrier {
    void BossKameckBarrierNrvWait::execute(Spine* pSpine) const {
        ((BossKameckBarrier*)pSpine->mExecutor)->exeWait();
    }
    BossKameckBarrierNrvWait(BossKameckBarrierNrvWait::sInstance);
};