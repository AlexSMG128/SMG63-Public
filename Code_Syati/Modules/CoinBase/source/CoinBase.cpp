#include "CoinBase.h"

void CoinInfo::setDefault() {
    mExistInDarkComet = false;
    mPlaySound = true;
    mUseSyncAppear = true;
    mUseReadSwitchB = true;
    mIgnoreSensorScaling = false;
    mInitFunction = true;
    mCoinAddNo = 1;
    strcpy(mMirrorActorName, "Coin");
}

CoinBase::CoinBase(const char* pName) : Coin(pName) {
    mCoinInfo.setDefault();
    MR::createCoinHolder();
}

void CoinBase::init(const JMapInfoIter& rIter) {
    MR::createSceneObj(SCENE_OBJ_PLAYER_FOLLOW_PASS);

    if (mCoinInfo.mInitFunction)
        MR::processInitFunction(this, rIter, false);

    if (MR::isValidInfo(rIter)) {
        setShadowAndPoseModeFromJMapIter(rIter);
        initShadow(rIter);
    }
    
    if (mIgnoreGravity) {
        TVec3f actorAxis;
        MR::calcActorAxisY(&actorAxis, this);
        mGravity.set(-actorAxis);
    }


    if (mIsInBubble) {
        mAirBubble = MR::createPartsModelNoSilhouettedMapObj(this, "アワ", "AirBubble", 0);
        mAirBubble->initFixedPosition(TVec3f(0.0f, 70.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), 0);
        mAirBubble->makeActorDead();
        MR::registerDemoSimpleCastAll(mAirBubble);
        MR::startAction(mAirBubble, "Move");
    }
    
    mFlashingCtrl = new FlashingCtrl(this, 1);

    MR::tryCreateMirrorActor(this, mCoinInfo.mMirrorActorName);
    
    initNerve(&NrvCoin::CoinNrvFix::sInstance, 0);

    if (MR::isValidInfo(rIter)) {
        if (!MR::useStageSwitchSyncAppear(this, rIter) && mCoinInfo.mUseSyncAppear) {
            appearFixInit();
        }

        if (mCoinInfo.mUseReadSwitchB) {
            if (!MR::initUseStageSwitchReadB(this, rIter)) {
                MR::listenStageSwitchOnB(this, MR::Functor<Coin>(this, &Coin::makeActorDead));
            }
        }

        MR::useStageSwitchAwake(this, rIter);
    }
    else
        makeActorDead();

}

void CoinBase::makeActorAppeared() {
    if (!isNerve(&NrvCoin::CoinNrvGot::sInstance)) {
        MR::offCalcGravity(this);
        if (!mIgnoreGravity)
            MR::calcGravityOrZero(this);

        if (!MR::isGalaxyDarkCometAppearInCurrentStage() && mCoinInfo.mExistInDarkComet) {
            MR::showModel(this);
            LiveActor::makeActorAppeared();
        }

        if (mIsInBubble)
            mAirBubble->appear();

        if (!mCoinInfo.mIgnoreSensorScaling || mIsInBubble) {
            if (mIsInBubble) {
                MR::setSensorRadius(this, "coin", 150.0f);
            }
            else {
                MR::setSensorRadius(this, "coin", 55.0f);
            }
        }

        mFlashingCtrl->end();
        setCalcShadowMode();
        MR::validateShadow(this, 0);

        mInWater = MR::isInWater(mTranslation);
    }
}

bool CoinBase::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgItemGet(msg))
        vRequestGetCoin();

    if (MR::isMsgItemShow(msg))
        requestShow();

    if (MR::isMsgItemHide(msg))
        requestHide();

    if (MR::isMsgItemNigeroTake(msg))
        canStartMove();

    if (MR::isMsgItemStartMove(msg))
        requestStartControl();

    if (MR::isMsgItemEndMove(msg))
        requestEndControl();

    if (MR::isMsgItemNigeroDrop(msg))
        requestMove();

    if (MR::isMsgItemSupportTicoDrop(msg))
        requestThrow();

    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }
    
    return false;
}

bool CoinBase::vRequestGetCoin() {
    noticeGetCoin();
    MR::emitEffect(this, "CoinGet");
    mFlashingCtrl->end();
    setNerve(&NrvCoin::CoinNrvGot::sInstance);
    makeActorDead();
    return true;
}

void CoinBase::noticeGetCoin() {
    if (mIsInBubble) {
        MR::emitEffect(mAirBubble, "RecoveryBubbleBreak");
        MR::incPlayerOxygen(8);
        mAirBubble->kill();
        mIsInBubble = false;
    }

    if (!mCoinInfo.mExistInDarkComet && mCoinInfo.mCoinAddNo != 0)
        MR::incCoin(mCoinInfo.mCoinAddNo, this);
    
    if (mCoinInfo.mPlaySound)
        MR::startActionSound(this, "SyCoin", -1, -1, -1);

    if (MR::isValidSwitchA(this))
        MR::onSwitchA(this);

    mCoinHostInfo->mCollectedCount++;
}

void CoinBase::appearAndMove(f32 vel, const char* soundName) {
    TVec3f gravityVec;
    MR::calcGravityVector(this, mTranslation, &gravityVec, 0, 0);
    gravityVec.scale(-vel);
    appearMove(mTranslation, gravityVec, 0x7FFFFFFF, -1);

    MR::startSystemSE(soundName, -1, -1);
}


CoinBase::~CoinBase() {
}