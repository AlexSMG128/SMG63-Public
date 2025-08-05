#include "RedCoin.h"
#include "RedCoinUtil.h"
#include "Game/MapObj/CoinHolder.h"
#include "RedCoinController.h"

RedCoin::RedCoin(const char* pName) : CoinBase(pName) {
    mLaunchVelocity = 25.0f;
    mHasRewardedCoins = false;
    mRedCoinCounterPlayerPos = false;

    MR::addToCoinHolder(this, this);
}

void RedCoin::init(const JMapInfoIter& rIter) {
    mCoinInfo.mExistInDarkComet = true;
    mCoinInfo.mCoinAddNo = 0;
    strcpy(mCoinInfo.mMirrorActorName, "RedCoin");
    mCoinInfo.mPlaySound = false;
    mCoinInfo.mUseReadSwitchB = false;
    CoinBase::init(rIter);
    MR::joinToGroupArray(this, rIter, "RedCoinGroup", 32); 

    MR::getJMapInfoArg0NoInit(rIter, &mLaunchVelocity);
    MR::getJMapInfoArg1NoInit(rIter, &mRedCoinCounterPlayerPos);

    if (MR::isValidSwitchB(this))
        requestHide();
}

void RedCoin::control() {
    if (MR::isOnSwitchB(this)) {
        if (isNerve(&NrvCoin::CoinNrvFixHide::sInstance))
            appearAndMove(mLaunchVelocity, "SE_SY_RED_COIN_APPEAR");

        mLifeTime = 0x7FFFFFFF;
    }

    if (isNerve(&NrvCoin::CoinNrvGot::sInstance))
        MR::zeroVelocity(this);
}

bool RedCoin::vRequestGetCoin() {
    MR::emitEffect(this, "RedCoinGet");
    mFlashingCtrl->end();
    setNerve(&NrvCoin::CoinNrvGot::sInstance);
    makeActorDead();
    
    RedCoinController* pController = (RedCoinController*)RedCoinUtil::getSpecificActorFromGroup(this, "RedCoinController");
    
    if (pController) {
        pController->startCountUp(this);
        MR::startSystemSE(pController->mHasAllRedCoins ? "SE_SY_RED_COIN_COMPLETE" : "SE_SY_RED_COIN", -1, -1);
    }
    
    if (!mHasRewardedCoins && !MR::isGalaxyDarkCometAppearInCurrentStage() && pController) {
        MR::incPlayerLife(1);
        GameSequenceFunction::getPlayResultInStageHolder()->addCoinNum(!pController->mRewardCoins ? 2 : 0);
        mHasRewardedCoins = true;
    }
    
    noticeGetCoin();
    return 1;
}