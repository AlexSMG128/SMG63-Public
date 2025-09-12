#include "CrystalBox.h"

CrystalBox::CrystalBox(const char* pName) : LiveActor (pName) {
    mBreakModel = 0;
    mDisplayModel = 0;
    mPurpleCoin = 0;
    mDisplayModelID = -1;
    mDisplayModelOffset = 0.0f;
}

void CrystalBox::init(const JMapInfoIter& rIter) {
    MR::processInitFunction(this, rIter, false);
    initNerve(&NrvCrystalBox::NrvWait::sInstance, 0);
    MR::calcGravity(this);
    mBreakModel = new ModelObj("クリスタル破壊", "CrystalBoxBreak", (MtxPtr)getBaseMtx(), 0x16, -2, -2, 0);
    mBreakModel->initWithoutIter();
    MR::hideModel(mBreakModel);
    MR::getJMapInfoArg7NoInit(rIter, &mDisplayModelID);
    if (mDisplayModelID > -1) {
        mDisplayModel = MR::createDummyDisplayModelCrystalItem(this, rIter, mTranslation, TVec3f(0.0f, 0.0f, 0.0f));
        mDisplayModel->initFixedPosition((MtxPtr)mDisplayModel->getBaseMtx(), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));

        if (mDisplayModelID == 0)
            MR::declareCoin(this, 1);

        if (mDisplayModelID == 4)
            MR::declareStarPiece(this, 5);
        
        if (mDisplayModelID == 7)
            MR::declarePowerStar(this);

        if (mDisplayModelID == 8 || mDisplayModelID == 14)
            mDisplayModelOffset = -30.0f;

        if (mDisplayModelID == 12)
            mDisplayModelOffset = 45.0f;

        if (mDisplayModelID == 27) {
            mPurpleCoin = MR::createPurpleCoin(this, "PurpleCoinCrystalBox");
            MR::addToCoinHolder(this, mPurpleCoin);
            mPurpleCoin->initWithoutIter();
        }

        if (mDisplayModelID == 28)
            mDisplayModelOffset = 52.0f;
    }

    MR::startBck(this, "CrystalBox", 0);

    makeActorAppeared();
}

void CrystalBox::calcAndSetBaseMtx() {
    if (mDisplayModel) {
        PSMTXCopy(MR::getJointMtx(this, "CrystalBox"), (MtxPtr)&mNewBaseMtx);
        TVec3f yDir;
        MR::extractMtxYDir((MtxPtr)&mNewBaseMtx, &yDir);
        yDir.scale(mDisplayModelOffset);
        MR::addTransMtx((MtxPtr)&mNewBaseMtx, yDir);

        MR::setBaseTRMtx(mDisplayModel, (MtxPtr)&mNewBaseMtx);
    }

    LiveActor::calcAndSetBaseMtx();
}

void CrystalBox::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isDead(this) && isNerve(&NrvCrystalBox::NrvWait::sInstance))
        MR::sendMsgPush(pReceiver, pSender);
}

bool CrystalBox::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if ((MR::isMsgPlayerHitAll(msg) || MR::isMsgPlayerHipDrop(msg)) && isNerve(&NrvCrystalBox::NrvWait::sInstance)) {
        setNerve(&NrvCrystalBox::NrvBreak::sInstance);
        return true;
    }
    return false;
}

void CrystalBox::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startBck(mBreakModel, "Break", 0);
        MR::startSound(this, "SE_OJ_CRYSTAL_CAGE_S_BREAK", -1, -1);
        MR::hideModel(this);
        MR::showModel(mBreakModel);

        if (mDisplayModelID == 0) {
            MR::appearCoinPop(this, mDisplayModel->mTranslation, 1);
        }
        else if (mDisplayModelID == 4) {
            MR::appearStarPiece(this, mDisplayModel->mTranslation, 5, 10.0f, 40.0f, false);
        }
        else if (mDisplayModelID == 7) {
            MR::requestAppearPowerStar(this, mDisplayModel->mTranslation);
            MR::tryDeleteEffectAll(mDisplayModel);
        }
        else if (mDisplayModelID == 27) {
            TVec3f coinVelocity = TVec3f(0.0f, 25.0f, 0.0f);
            coinVelocity.scale(coinVelocity.y, -mGravity);
            mPurpleCoin->appearMove(mDisplayModel->mTranslation, coinVelocity, 0x7FFFFFFF, 60);
            MR::startSystemSE("SE_SY_PURPLE_COIN_APPEAR", -1, -1);
        }

        if (MR::isValidSwitchDead(this))
            MR::onSwitchDead(this);
    }
}

namespace NrvCrystalBox {
    void NrvWait::execute(Spine* pSpine) const {

    }

    void NrvBreak::execute(Spine* pSpine) const {
        ((CrystalBox*)pSpine->mExecutor)->exeBreak();
    }

    NrvWait(NrvWait::sInstance);
    NrvBreak(NrvBreak::sInstance);
}