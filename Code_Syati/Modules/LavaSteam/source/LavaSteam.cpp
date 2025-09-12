#include "LavaSteam.h"

LavaSteam::LavaSteam(const char* pName) : LiveActor(pName) {
    mInterval = 90;
    mSignTime = 90;
    mSteamTime = 90;
    _9C = TVec3f(0.0f, 1.0f, 0.0f);
    mEffectSRTVec = TVec3f(1.0f);
    mSteamForeverMode = false;
}

void LavaSteam::init(const JMapInfoIter& rIter) {
    bool signOnActivate = false;
    bool offMode = false;
    MR::processInitFunction(this, rIter, false);
    MR::getJMapInfoArg0NoInit(rIter, &mInterval);
    MR::getJMapInfoArg1NoInit(rIter, &mSignTime);
    MR::getJMapInfoArg2NoInit(rIter, &mSteamTime);
    MR::getJMapInfoArg3NoInit(rIter, &signOnActivate);
    MR::getJMapInfoArg4NoInit(rIter, &mSteamForeverMode);
    MR::getJMapInfoArg5NoInit(rIter, &offMode);

    MR::setEffectHostSRT(this, "Sign", &mTranslation, &mRotation, &mEffectSRTVec);

    HitSensor* pSensor = getSensor("body");

    MR::setClippingTypeSphere(this, 250.0f, &pSensor->mPosition);

    Nerve* pNerve = &NrvLavaSteam::HostTypeWait::sInstance;
    initNerve(pNerve, 0);
    
    void (LavaSteam::* pFunc)() = &LavaSteam::startSteam;

    if (signOnActivate) {
        pFunc = &LavaSteam::startSteamSign;
    }
    else {
        if (offMode) {
            pNerve = &NrvLavaSteam::HostTypeWait::sInstance;
            pFunc = &LavaSteam::setOff;
        }
        else {
            pNerve = &NrvLavaSteam::HostTypeWaitForSwitchOn::sInstance;
        }
    }
    
    if (MR::isExistStageSwitchA(rIter)) {
        setNerve(pNerve);
        MR::listenStageSwitchOnA(this, MR::Functor(this, pFunc));
    }
    else if (MR::tryRegisterDemoCast(this, rIter)) {
        setNerve(pNerve);
        MR::registerDemoActionFunctor(this, MR::Functor(this, pFunc), 0);
    }
    else if (mSteamForeverMode)
        setNerve(&NrvLavaSteam::HostTypeSteam::sInstance);
    
    if (MR::isExistStageSwitchB(rIter))
        MR::listenStageSwitchOnB(this, MR::Functor(this, &LavaSteam::setOff));

    makeActorAppeared();
}

void LavaSteam::initAfterPlacement() {
    TRot3f mtx;
    mtx.identity();
    TVec3f vec = mRotation*0.017453292f;

    f32 vecz = vec.z;
    f32 vecy = vec.y;
    f32 vecx = vec.x;

    f32 v11 = cos(vecz); 
    f32 v12 = cos(vecy);
    f32 v13 = cos(vecx);
    f32 v15 = sin(vecy);
    f32 v14 = sin(vecz); 
    f32 v16 = sin(vecx);

    mtx.mMtx[0][0] = v12 * v11;
    mtx.mMtx[2][1] = v16 * v12;
    mtx.mMtx[1][0] = v12 * v14;
    mtx.mMtx[1][1] = (v13 * v11) + (v16 * v15 * v14);
    mtx.mMtx[0][1] = ( v16 * v15 * v11) - (v13 * v14);
    mtx.mMtx[0][2] = (v13 * v11 * v15) + (v16 * v14);
    mtx.mMtx[2][0] = -v15;
    mtx.mMtx[2][2] = v12 * v13;
    mtx.mMtx[1][2] = (v13 * v14 * v15) - (v16 * v11);

    _9C.set(mtx.mMtx[0][1], mtx.mMtx[1][1], mtx.mMtx[2][1]);
    MR::normalize(&_9C);
}


void LavaSteam::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f stack_64;
    TVec3f stack_58;
    f32 stack_2C, stack_54;
    
    if (!isNerve(&NrvLavaSteam::HostTypeWait::sInstance) && !isNerve(&NrvLavaSteam::HostTypeWaitForSwitchOn::sInstance) && !isNerve(&NrvLavaSteam::HostTypeOff::sInstance)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            f32 rad = pReceiver->mRadius;
            f32 f31 = rad*0.69999999f;
            
            if (isNerve(&NrvLavaSteam::HostTypeSteam::sInstance)) {
                TVec3f stack_30 =_9C*400.0f;
                TVec3f stack_3C = TVec3f(mTranslation);
                
                JMathInlineVEC::PSVECAdd2((Vec*)&stack_3C, (Vec*)&stack_30, (Vec*)&stack_3C);
                
                TVec3f stack_48;
                stack_48.set(stack_3C);

                stack_54 = 70.0f;

                TVec3f stack_20;
                stack_20.set(pReceiver->mPosition);
                
                f32 dist = JMathInlineVEC::PSVECSquareDistance((Vec*)&stack_48, (Vec*)&stack_20);
                f32 s = (stack_54+f31);
                stack_2C = f31;
                
                if ((dist <= s*s) & 3 && MR::sendMsgEnemyAttackFire(pReceiver, pSender))
                    return;
            }
        
            if (isNerve(&NrvLavaSteam::HostTypeSteam::sInstance)) {
                TVec3f stack_14 = _9C*330.0f;

                stack_58.set(mTranslation);
                stack_64.set(stack_14);

                TVec3f stack_8;
                JMathInlineVEC::PSVECSubtract((Vec*)&pReceiver->mPosition, (Vec*)&stack_58, (Vec*)&stack_8);
                f32 sq1 = stack_8.squared();
                f32 sq2 = stack_64.squared();
                f32 _64dot8 = stack_64.dot(stack_8);
                f32 ff1 = sq2*sq1;
                f32 ff2 = _64dot8/sq2;
                f32 squ3 = JGeometry::TUtil<f32>::sqrt((ff1-(_64dot8*_64dot8))/sq2);

                if (0.0f <= ff2 && ff2 <= 1.0f && squ3 < (10.0f+f31) && MR::sendMsgEnemyAttackFire(pReceiver, pSender))
                    return;
            }
        }
    }
}

void LavaSteam::startClipped() {
    LiveActor::startClipped();
    
    if (!mSteamForeverMode)
        MR::deleteEffectAll(this);
}

void LavaSteam::startSteam() {
    setNerve(&NrvLavaSteam::HostTypeSteam::sInstance);
}

void LavaSteam::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::emitEffect(this, "Sign");
        mEffectSRTVec.setAll(1.0f);
    }

    if (MR::isGreaterStep(this, mSignTime-8) && MR::isLessEqualStep(this, mSignTime)) {
        int step = getNerveStep();
        f32 f = MR::getEaseInValue(((mSignTime)-step)*0.125f, 0.001f, 1.0f, 1.0f);
        mEffectSRTVec.setAll(f);
    }

    if (MR::isStep(this, mSignTime))
        MR::forceDeleteEffect(this, "Sign");

    if (MR::isStep(this, mSignTime+30))
        setNerve(&NrvLavaSteam::HostTypeSteam::sInstance);
}

void LavaSteam::exeSteam() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Steam");
        MR::validateHitSensors(this);
    }

    MR::startLevelSound(this, "SE_OJ_LV_LAVA_STEAM_OUT", -1, -1, -1);

    if (MR::isStep(this, mSteamTime) && !mSteamForeverMode) {
        MR::deleteEffect(this, "Steam");
        setNerve(&NrvLavaSteam::HostTypeSteamEnd::sInstance);
    }
}

void LavaSteam::exeSteamEnd() {
    if (MR::isStep(this, mInterval))
        setNerve(&NrvLavaSteam::HostTypeWait::sInstance);
}

void LavaSteam::exeOff() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
        MR::emitEffect(this, "Off");
    }
}

void LavaSteam::startSteamSign() {
    setNerve(&NrvLavaSteam::HostTypeWait::sInstance);
}

void LavaSteam::setOff() {
    setNerve(&NrvLavaSteam::HostTypeOff::sInstance);
}

LavaSteam::~LavaSteam() {

}

namespace NrvLavaSteam {
    void HostTypeWait::execute(Spine* pSpine) const {
        ((LavaSteam*)pSpine->mExecutor)->exeWait();
    }

    void HostTypeWaitForSwitchOn::execute(Spine* pSpine) const {
    }
    
    void HostTypeSteam::execute(Spine* pSpine) const {
        ((LavaSteam*)pSpine->mExecutor)->exeSteam();
    }
    
    void HostTypeSteamEnd::execute(Spine* pSpine) const {
        ((LavaSteam*)pSpine->mExecutor)->exeSteamEnd();
    }

    void HostTypeOff::execute(Spine* pSpine) const {
        ((LavaSteam*)pSpine->mExecutor)->exeOff();
    }

    HostTypeWait(HostTypeWait::sInstance);
    HostTypeWaitForSwitchOn(HostTypeWaitForSwitchOn::sInstance);
    HostTypeSteam(HostTypeSteam::sInstance);
    HostTypeSteamEnd(HostTypeSteamEnd::sInstance);
    HostTypeOff(HostTypeOff::sInstance);
}
