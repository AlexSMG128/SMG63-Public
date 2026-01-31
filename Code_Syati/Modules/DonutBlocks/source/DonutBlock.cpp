#include "DonutBlock.h"

namespace pt {
    extern void initRailToNearestAndRepositionWithGravity(LiveActor* pActor);
    extern void getTransRotateFromRailWithGravity(LiveActor* pActor);
}

DonutBlock::DonutBlock(const char *pName) : LiveActor(pName) {
    mFallDelay = 25;
    mFallDuration = 120;
    mRespawnDelay = 100;
    mFallSpeed = 5.0f;

    mRailSpeed = 5.0f;
    mRailDelay = 0;

    mOriginalPos = TVec3f();
    mRailDelayTimer = 0;
}

DonutBlock::~DonutBlock() { }

void DonutBlock::init(const JMapInfoIter &rIter) {
    MR::processInitFunction(this, rIter, false);
    MR::onCalcGravity(this);

    MR::getJMapInfoArg0NoInit(rIter, &mFallDelay);
    MR::getJMapInfoArg1NoInit(rIter, &mFallDuration);
    MR::getJMapInfoArg2NoInit(rIter, &mRespawnDelay);
    MR::getJMapInfoArg3NoInit(rIter, &mFallSpeed);
    MR::getJMapInfoArg4NoInit(rIter, &mRailSpeed);
    MR::getJMapInfoArg5NoInit(rIter, &mRailDelay);

    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
        mRailRider->setSpeed(mRailSpeed);

        pt::initRailToNearestAndRepositionWithGravity(this);
    }

    MR::useStageSwitchReadA(this, rIter);
    MR::useStageSwitchWriteB(this, rIter);
    
    if (MR::isValidSwitchA(this))
        MR::listenStageSwitchOnA(this, MR::Functor(this, &startFall));

    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Platform");
    MR::initCollisionParts(this, "DonutBlock", getSensor("Platform"), NULL);

    initNerve(&NrvDonutBlock::NrvWait::sInstance, 0);

    MR::startBtpAndSetFrameAndStop(this, "TexChange", 0);
    makeActorAppeared();
}

void DonutBlock::control() {
    if (mRailRider == NULL)
        return;

    if (isNerve(&NrvDonutBlock::NrvFallStart::sInstance)) {
        f32 decel = (mFallDelay - mSpine->mStep) / mFallDelay;
        mRailRider->setSpeed(mRailSpeed * decel);
    }
    else if (!isNerve(&NrvDonutBlock::NrvWait::sInstance)) {
        return;
    }

    if (mRailDelayTimer <= 0) {
        mRailRider->move();
        MR::moveTransToCurrentRailPos(this);
        pt::getTransRotateFromRailWithGravity(this);
    }

    if (mRailRider->isReachedGoal() || mRailRider->isReachedEdge()) {
        if (mRailDelayTimer == 0)
            mRailDelayTimer = mRailDelay;

        mRailDelayTimer--;

        if (mRailDelayTimer <= 0)
            mRailRider->setSpeed(-mRailRider->mSpeed);
    }
}

void DonutBlock::exeWait() {
    if (MR::isOnPlayer(this))
        startFall();
}

void DonutBlock::exeFallStart() {
    if (MR::isStep(this, mFallDelay)) {
        mOriginalPos.set(mTranslation);

        if (MR::isValidSwitchB(this))
            MR::onSwitchB(this);

        setNerve(&NrvDonutBlock::NrvFall::sInstance);
        return;
    }
}

void DonutBlock::exeFall() {
    if (MR::isFirstStep(this)) {
        if (mRailRider != NULL)
            mRailRider->setSpeed(0);
            
        TVec3f upVec;
		MR::calcUpVec(&upVec, this);

        mVelocity = -upVec * mFallSpeed;
    }
    else if (MR::isStep(this, mFallDuration)) {
        setNerve(&NrvDonutBlock::NrvRespawn::sInstance);
        return;
    }
}

void DonutBlock::exeRespawn() {
    if (MR::isFirstStep(this)) {
        emitEffectAndPlayAudio();

        MR::hideModel(this);
        MR::invalidateCollisionParts(this);

        mVelocity.setAll(0);
    }
    else if (MR::isGreaterEqualStep(this, mRespawnDelay)) {
        if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this))
            return;

        mTranslation.set(mOriginalPos);

        emitEffectAndPlayAudio();
        MR::startBtpAndSetFrameAndStop(this, "TexChange", 0);

        MR::showModel(this);
        MR::validateCollisionParts(this);

        if (mRailRider != NULL)
            mRailRider->setSpeed(mRailSpeed);

        if (MR::isValidSwitchB(this))
            MR::offSwitchB(this);

        setNerve(&NrvDonutBlock::NrvWait::sInstance);
    }
}

void DonutBlock::startFall() {
    if (!isNerve(&NrvDonutBlock::NrvWait::sInstance))
        return;

    MR::startBck(this, "FallStart", NULL);
    MR::startBtpAndSetFrameAndStop(this, "TexChange", 1);

    setNerve(&NrvDonutBlock::NrvFallStart::sInstance);
}

void DonutBlock::emitEffectAndPlayAudio() {
    MR::startActionSound(this, "OjBlockAppear", -1, -1, -1);
    MR::emitEffect(this, "Smoke");
}

namespace NrvDonutBlock {
    void NrvWait::execute(Spine *pSpine) const {
        ((DonutBlock *)pSpine->mExecutor)->exeWait();
    }

    void NrvFallStart::execute(Spine *pSpine) const {
        ((DonutBlock *)pSpine->mExecutor)->exeFallStart();
    }

    void NrvFall::execute(Spine *pSpine) const {
        ((DonutBlock *)pSpine->mExecutor)->exeFall();
    }

    void NrvRespawn::execute(Spine *pSpine) const {
        ((DonutBlock *)pSpine->mExecutor)->exeRespawn();
    }

    NrvWait(NrvWait::sInstance);
    NrvFallStart(NrvFallStart::sInstance);
    NrvFall(NrvFall::sInstance);
    NrvRespawn(NrvRespawn::sInstance);
}