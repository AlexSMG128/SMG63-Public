#pragma once

#include "syati.h"

// ======== GENERAL PURPOSE ========

NERVE(YoshiEatExtNrvEaten);
YoshiEatExtNrvEaten(YoshiEatExtNrvEaten::sInstance);
void YoshiEatExtNrvEaten::execute(Spine* pSpine) const {}

bool receiveMessageEaten(LiveActor* pAct, u32 msg, NerveExecutor* pExec) {
    if (MR::isMsgYoshiStartTongue(msg))
    {
        MR::stopBck(pAct);
        MR::invalidateHitSensors(pAct);
        MR::invalidateClipping(pAct);

        if (pAct->mSpine != NULL)
            pAct->setNerve(&YoshiEatExtNrvEaten::sInstance);

        if (pExec)
            pExec->setNerve(&YoshiEatExtNrvEaten::sInstance);

        return true;
    }

    if (MR::isMsgYoshiLoopTongue(msg))
        return true;

    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        MR::zeroVelocity(pAct);
        MR::offBind(pAct);
        pAct->makeActorDead();
        return true;
    }

    return false;
}










// ======== SamboHead ========

#include "ModuleData_SamboHead_Ext.h"

void initTargetForSamboHead(SamboHead_Ext* pSamba, const JMapInfoIter& rIter) {
    MR::useStageSwitchSyncAppear(pSamba, rIter);
    pSamba->mYoshiTarget = MR::createYoshiLockOnTarget(pSamba, "Center", TVec3f(0.0f));
}
kmCall(0x801F8148, initTargetForSamboHead);

bool trySamboHeadEat(SamboHead_Ext* pSambo) {
    if (!pSambo->isNerve(&YoshiEatExtNrvEaten::sInstance) && pSambo->isStarPointerBindable()) {
        YoshiFunction::tryYoshiEat(pSambo->mYoshiTarget);
        return true;
    }
    else
        return false;
}
kmCall(0x801F82B0, trySamboHeadEat);

bool tryYoshiEatMsgSamboHead() {
    register LiveActor* pAct;
    register u32 msg;
    __asm
    {
        mr pAct, r28
        mr msg, r29
    }

    return receiveMessageEaten(pAct, msg, 0);
}
kmCall(0x801F9354, tryYoshiEatMsgSamboHead);










// ======== BasaBasa ========

#include "ModuleData_BasaBasa_Ext.h"

void initBasaBasaYoshiEatTarget(BasaBasa_Ext* pAct, const JMapInfoIter& rIter) {
    MR::useStageSwitchAwake(pAct, rIter);
    pAct->mYoshiTarget = MR::createYoshiLockOnTarget(pAct, "JointRoot", TVec3f(0.0f));
}
kmCall(0x80156F24, initBasaBasaYoshiEatTarget);

bool updateBasaBasaYoshiEatTarget(BasaBasa_Ext* pAct) {
    if (!pAct->isNerve(&YoshiEatExtNrvEaten::sInstance) && pAct->isNrvEnableStun())
        YoshiFunction::tryYoshiEat(pAct->mYoshiTarget);

    return pAct->tryClippingAndResetPos();
}
kmCall(0x80158044, updateBasaBasaYoshiEatTarget);

bool receiveMsgBasaBasa(BasaBasa_Ext* pAct, u32 msg) {
    return receiveMessageEaten(pAct, msg, pAct->mAnimScaleCtrl);
}
kmCall(0x80158700, receiveMsgBasaBasa);










// ======== KameckTurtle ========

#include "ModuleData_KameckTurtle_Ext.h"

void initKameckTurtleYoshiEatTarget(KameckTurtle_Ext* pAct, const Nerve* pNerve, s32 l) {
    pAct->initNerve(pNerve, l);
    MR::initStarPointerTarget(pAct, 100.0f, TVec3f(0.0f));
    pAct->mYoshiTarget = MR::createYoshiLockOnTarget(pAct, (MtxPtr)NULL, TVec3f(0.0f));
}
kmCall(0x801A8DC4, initKameckTurtleYoshiEatTarget);

void updateKameckTurtleYoshiEatTarget(TQuat4f* quat, KameckTurtle_Ext* pAct, const TVec3f& vec, f32 f1, f32 f2) {

    MR::blendQuatFromGroundAndFront(quat, pAct, vec, f1, f2);

    if (!MR::isDead(pAct) && !MR::isHiddenModel(pAct) && pAct->isNerve(&NrvKameckTurtle::KameckTurtleNrvRun::sInstance))
    {
        YoshiFunction::tryYoshiLick(pAct->mYoshiTarget);
    }
}
kmBranch(0x801A8F24, updateKameckTurtleYoshiEatTarget);

// Casually fully reimplementing this
bool receiveKameckTurtle(KameckTurtle_Ext* pAct, u32 msg, HitSensor* sensorA, HitSensor* sensorB) {
    if (msg == ACTMES_IS_PULL_ENABLE)
        return true;
    if (MR::isMsgItemPull(msg))
    {
        pAct->setNerve(&NrvKameckTurtle::KameckTurtleNrvWait::sInstance);
        pAct->appearJetTurtle(true);
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg))
    {
        pAct->setNerve(&NrvKameckTurtle::KameckTurtleNrvWait::sInstance);
        pAct->appearJetTurtle(false);
        return true;
    }
    return false;
}
kmBranch(0x801A90B0, receiveKameckTurtle);

bool isWaitNerve_JetTurtle(LiveActor* pAct, const Nerve* pn)
{
    return pAct->isNerve(pn) || pAct->isNerve(&NrvJetTurtle::JetTurtleNrvWait2::sInstance);
}

kmCall(0x802B97D8, isWaitNerve_JetTurtle);