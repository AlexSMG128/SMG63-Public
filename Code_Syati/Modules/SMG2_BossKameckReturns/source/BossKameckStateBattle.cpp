#include "BossKameckStateBattle.h"
#include "BossKameckBattlePattern.h"
#include "BossKameckMoveRail.h"
#include "BossKameck.h"
#include "Game/Enemy/KameckBeam.h"
#include "Game/Enemy/KameckBeamHolder.h"
#include "Game/Util/JointUtil.h"

#include "syati.h"

BossKameckStateBattle::BossKameckStateBattle(BossKameck* pBoss) : mBossKameck(pBoss), ActorStateBase<BossKameck>("ボスカメック戦闘状態") {
    mMoveRail = NULL;
    mBattlePattarn = NULL;
    mBeam = NULL;
    mBeamListener = mBossKameck->mBeamListener;
    _20.zero();
    _2C = -1;
    _30 = 1;
    _34 = 0;
    _38 = 0;
    _3C = 0;
    _3D = 0;
}

void BossKameckStateBattle::init() {
    initNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvWait::sInstance, 0);
}

void BossKameckStateBattle::appear() {
    mIsDead = false;
    startMove();
    MR::onCalcGravity(mBossKameck);
}

void BossKameckStateBattle::setMoveRail(BossKameckMoveRail* pMoveRail) {
    mMoveRail = pMoveRail;
}

void BossKameckStateBattle::setBattlePattarn(BossKameckBattlePattern* pPattarn) {
    if (mBattlePattarn == NULL)
    {
        if (pPattarn != NULL && pPattarn->mBarrier != NULL)
            pPattarn->mBarrier->appear();
    }
    else if (pPattarn == NULL)
    {
        if (mBattlePattarn->mBarrier != NULL) // No need to check for mBattlePattarn being null, as it's impossible to get here if it is.
            mBattlePattarn->mBarrier->kill();
    }
    else if (mBattlePattarn->mBarrier != pPattarn->mBarrier)
    {
        if (mBattlePattarn->mBarrier != NULL)
            mBattlePattarn->mBarrier->kill();
        if (pPattarn->mBarrier != NULL)
            pPattarn->mBarrier->appear();
    }

    mBattlePattarn = pPattarn;
}

void BossKameckStateBattle::attackSensor(HitSensor* a1, HitSensor* a2) {
    if (isEnableGuard() && MR::isSensorPlayer(a2)) {
        if (isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvGuard::sInstance)) {
            if (MR::sendMsgEnemyAttackFlipRot(a2, a1)) {
                return;
            }
        }
        else if (MR::isSensorEnemyAttack(a1)) {
            if (!MR::sendMsgEnemyAttack(a2, a1)) {
                MR::sendMsgPush(a2, a1);
            }
        }
    }
}

bool BossKameckStateBattle::receiveMsgPlayerAttack(u32 msg, HitSensor* a1, HitSensor* a2) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        return requestDamage(a1, a2);
    }

    if (MR::isMsgPlayerSpinAttack(msg)) {
        return requestGuard(a1, a2);
    }

    return false;
}

bool BossKameckStateBattle::requestDamage(HitSensor* a1, HitSensor* a2) {
    if (isEnableDamage()) {
        if (mBeam != NULL) {
            mBeam->kill();
            mBeam = NULL;
        }

        mBossKameck->killAllBeam();
        MR::emitEffectHitBetweenSensors(mBossKameck, a1, a2, 0.0f, "HitMarkNormal");
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvDamage::sInstance);
        return true;
    }

    return false;
}

bool BossKameckStateBattle::requestGuard(HitSensor* a1, HitSensor* a2) {
    if (isEnableGuard()) {
        if (mBeam != NULL) {
            mBeam->kill();
            mBeam = NULL;
        }

        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvGuard::sInstance);
        return true;
    }

    return false;
}

void BossKameckStateBattle::control() {
    mBossKameck->updatePose();
}

bool BossKameckStateBattle::tryAttackWait() {
    if (MR::isGreaterStep(this, 120) && MR::isNear(mBossKameck, _20, 100.0f)) {
        if (tryChargeBram()) {
            setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttackWait::sInstance);
        }
        else {
            startMove();
        }

        return true;
    }

    return false;
}

bool BossKameckStateBattle::trySummonKameck() {
    if (mBattlePattarn->mFollowerGroupId < 0 || !mBattlePattarn->mFollowerAllowRespawn)
        return false;

    if (_38 <= 3) {
        return false;
    }

    if (mBossKameck->getLivingKameckNum() == 0) {
        _38 = 0;
        return true;
    }

    return false;
}

bool BossKameckStateBattle::tryChargeBram() {
    /*if (MR::getRandom((s32)0, 101) < 25) {
        return false;
    }*/

    KameckBeamHolder* pHolder = mBossKameck->tryGetBeamHolder(mBattlePattarn);
    if (pHolder == NULL)
        return false;

    TVec3f v6;
    v6.zero();
    MtxPtr jointMtx = MR::getJointMtx(mBossKameck, "PowerStarC");
    mBeam = pHolder->startFollowKameckBeam(jointMtx, 1.0f, v6, mBeamListener);// MR::startFollowKameckBeam(mBattlePattarn->goNextPattarn(), jointMtx, 1.0f, v6, mBeamListener);

    if (mBeam == NULL)
        return false;

    mBeam->setEventListener(mBossKameck->mBeamListener);
    if (mBossKameck->getLivingKameckNum() == 0) {
        _38++;
    }
    else {
        _38 = 0;
    }

    return true;
}

void BossKameckStateBattle::startMove() {
    if (mBattlePattarn->mMovementType) { // lol treat as boolean
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveStart::sInstance);
    }
    else {
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvMove::sInstance);
    }
}

void BossKameckStateBattle::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Wait");
    }

    MR::attenuateVelocity(mBossKameck, 0.95999998f);

    if (MR::isGreaterStep(this, 60)) {
        startMove();
    }
}

void BossKameckStateBattle::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Wait");
        selectPosition();
    }

    f32 mag = (100.0f * PSVECMag((const Vec*)&mBossKameck->mVelocity));
    MR::startLevelSound(mBossKameck, "SE_BM_LV_KAMECK_FLOAT", mag, -1, -1);

    s32 v2 = (_3C) ? 40 : 90;

    if (MR::isLessEqualStep(this, v2)) {
        if (MR::isNear(mBossKameck, _20, 100.0f) || MR::isNearPlayer(_20, 400.0f)) {
            selectPosition();
        }

        BossKameck* boss = mBossKameck;
        //MR::addVelocityAwayFromTarget(boss, , 1.5f, 0.0f, 0.0f, 500.0f);
        {
            TVec3f* playerpos = MR::getPlayerPos();
            TVec3f bosspos(boss->mTranslation);
            TVec3f result;
            PSVECSubtract((const Vec*)&bosspos, (const Vec*)playerpos, (Vec*)&bosspos);
            MR::calcVelocityMoveToDirection(&result, boss, bosspos, 1.5f, 0.0f, 0.0f, 500.0f);
            boss->mVelocity.add(result);
        }
    }

    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);
    MR::addVelocityMoveToTarget(mBossKameck, _20, 0.089999996f, 0.89999998f, 0.0f, 400.0f);
    MR::addVelocityKeepHeight(mBossKameck, _20, 0.0f, 0.5f, 50.0f);
    MR::attenuateVelocity(mBossKameck, 0.95999998f);
    tryAttackWait();
}

void BossKameckStateBattle::exeHideMoveStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Hide");
        MR::startSound(mBossKameck, "SE_BM_KAMECK_HIDE_SPIN", -1, -1);
    }

    MR::zeroVelocity(mBossKameck);
    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);

    if (MR::isActionEnd(mBossKameck)) {
        MR::startSound(mBossKameck, "SE_BM_KAMECK_HIDE_SMOKE", -1, -1);
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMove::sInstance);
    }
}

void BossKameckStateBattle::exeHideMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Move");
        MR::hideModelAndOnCalcAnim(mBossKameck);
        selectPosition();
        _34 = 0;
    }

    MR::startLevelSound(mBossKameck, "SE_BM_LV_KAMECK_HIDE_MOVE", -1, -1, -1);

    f32 v2;

    if (_3C) {
        v2 = 20.0f;
    }
    else {
        v2 = 15.0f;
    }

    MR::addVelocityMoveToTarget(mBossKameck, _20, (0.1f * v2), v2, 0.0f, 400.0f);
    MR::addVelocityKeepHeight(mBossKameck, _20, 0.0f, 0.5f, 50.0f);
    MR::attenuateVelocity(mBossKameck, 0.89999998f);
    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);

    if (MR::isNear(mBossKameck, _20, 100.0f)) {
        if (_34 >= _30) {
            setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveEnd::sInstance);
        }
        else {
            _34++;

            if (_30 == _34) {
                selectStoppablePosition();
            }
            else {
                selectPosition();
            }
        }
    }
}

void BossKameckStateBattle::exeHideMoveEnd() {
    if (MR::isFirstStep(this)) {
        MR::showModel(mBossKameck);
        MR::startAction(mBossKameck, "Appear");
        MR::startSound(mBossKameck, "SE_BM_KAMECK_APPEAR_SMOKE", -1, -1);
        MR::startSound(mBossKameck, "SE_BM_KAMECK_APPEAR_SPIN", -1, -1);
    }

    MR::zeroVelocity(mBossKameck);
    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);

    if (MR::isActionEnd(mBossKameck)) {
        if (trySummonKameck()) {
            setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameckWait::sInstance);
        }
        else if (tryChargeBram()) {
            setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttackWait::sInstance);
        }
        else {
            startMove();
        }
    }
}

void BossKameckStateBattle::exeSummonKameckWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "SummonWait");
    }

    MR::zeroVelocity(mBossKameck);
    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);

    if (MR::isActionEnd(mBossKameck)) {
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameck::sInstance);
    }
}

void BossKameckStateBattle::exeSummonKameck() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Summon");
        mBossKameck->appearKameck(mBattlePattarn->mFollowerGroupId);
    }

    if (MR::isActionEnd(mBossKameck)) {
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvWait::sInstance);
    }
}

void BossKameckStateBattle::exeAttackWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "AttackWait");
    }

    MR::startLevelSound(mBossKameck, "SE_BM_LV_KAMECK_STAFF_TURN", -1, -1, -1);
    MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);
    MR::addVelocityMoveToTarget(mBossKameck, _20, 0.089999996f, 0.89999998f, 0.0f, 400.0f);
    MR::addVelocityKeepHeight(mBossKameck, _20, 0.0f, 0.2f, 50.0f);
    MR::attenuateVelocity(mBossKameck, 0.89999998f);

    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttack::sInstance);
    }
}

void BossKameckStateBattle::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Attack");
    }

    if (MR::isStep(this, 12)) {
        MR::startSound(mBossKameck, "SE_BM_KAMECK_STAFF_SWING", -1, -1);
        MR::startSound(mBossKameck, "SE_BV_KAMECK_STAFF_SWING", -1, -1);
    }

    if (MR::isStep(this, 23)) {
        mBeam->requestShootToPlayerCenter(20.0f);
        //mBossKameck->mActorList->addActor(mBeam);
        mBeam = NULL;
        MR::startSound(mBossKameck, "SE_BM_KAMECK_STAFF_EFFECT", -1, -1);
    }

    MR::attenuateVelocity(mBossKameck, 0.95999998f);

    if (MR::isGreaterStep(this, 30)) {
        setNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvWait::sInstance);
    }
}

void BossKameckStateBattle::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mBossKameck);

        if (_3C) {
            MR::stopStageBGM(30);
            MR::startAction(mBossKameck, "Down");
            MR::startSound(mBossKameck, "SE_BV_KAMECK_LAST_DAMAGE", -1, -1);
            MR::startSound(mBossKameck, "SE_BM_KAMECK_LAST_DAMAGE", -1, -1);
        }
        else {
            MR::startAction(mBossKameck, "Damage");
            MR::startSound(mBossKameck, "SE_BV_KAMECK_DAMAGE", -1, -1);
            MR::startSound(mBossKameck, "SE_BM_KAMECK_DAMAGE", -1, -1);
        }
    }

    if (MR::isActionEnd(mBossKameck)) {
        kill();
    }
}

void BossKameckStateBattle::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Recover");
        MR::startSound(mBossKameck, "SE_BM_KAMECK_RECOVER", -1, -1);
    }

    if (MR::isGreaterStep(this, 60)) {
        MR::turnDirectionToPlayerDegree(mBossKameck, &mBossKameck->_A0, 6.0f);
        MR::addVelocityMoveToTarget(mBossKameck, _20, 0.089999996f, 0.89999998f, 0.0f, 400.0f);
        MR::addVelocityKeepHeight(mBossKameck, _20, 0.0f, 0.5f, 50.0f);
        MR::attenuateVelocity(mBossKameck, 0.95999998f);
    }

    if (MR::isGreaterStep(this, 60)) {
        if (MR::isNear(mBossKameck, _20, 100.0f)) {
            MR::validateHitSensors(mBossKameck);
            startMove();
        }
    }
}

void BossKameckStateBattle::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mBossKameck, "Guard");
        MR::turnDirectionToTarget(mBossKameck, &mBossKameck->_A0, *MR::getPlayerPos(), 0.30000001f);
    }

    MR::attenuateVelocity(mBossKameck, 0.95999998f);

    if (MR::isActionEnd(mBossKameck)) {
        startMove();
    }
}

void BossKameckStateBattle::selectStoppablePosition() {
    _2C = mMoveRail->calcRandomStoppableIndex(_2C);
    mMoveRail->calcMovePosition(&_20, _2C);
}

void BossKameckStateBattle::selectPosition() {
    _2C = mMoveRail->calcRandomIndex(_2C);
    mMoveRail->calcMovePosition(&_20, _2C);
}

bool BossKameckStateBattle::isEnableDamage() const {
    bool ret = false;

    if (isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvMove::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveStart::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveEnd::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameckWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameck::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttackWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttack::sInstance)) {
        return true;
    }

    return ret;
}

bool BossKameckStateBattle::isEnableGuard() const {
    bool ret = false;

    if (isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvMove::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveStart::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvHideMoveEnd::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameckWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvSummonKameck::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttackWait::sInstance)
        || isNerve(&NrvBossKameckStateBattle::BossKameckStateBattleNrvAttack::sInstance)) {
        return true;
    }

    return ret;
}

BossKameckStateBattle::~BossKameckStateBattle() {

}

#pragma warning off (10548)
// For some reason the compiler doesn't like this. It works perfectly fine in game, and without it, it will not link, soooooo...
ActorStateBase<BossKameck>::~ActorStateBase() {

}
#pragma warning on (10548)

namespace NrvBossKameckStateBattle {
    void BossKameckStateBattleNrvWait::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeWait();
    }
    BossKameckStateBattleNrvWait(BossKameckStateBattleNrvWait::sInstance);

    void BossKameckStateBattleNrvMove::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeMove();
    }
    BossKameckStateBattleNrvMove(BossKameckStateBattleNrvMove::sInstance);

    void BossKameckStateBattleNrvHideMoveStart::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeHideMoveStart();
    }
    BossKameckStateBattleNrvHideMoveStart(BossKameckStateBattleNrvHideMoveStart::sInstance);

    void BossKameckStateBattleNrvHideMove::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeHideMove();
    }
    BossKameckStateBattleNrvHideMove(BossKameckStateBattleNrvHideMove::sInstance);

    void BossKameckStateBattleNrvHideMoveEnd::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeHideMoveEnd();
    }
    BossKameckStateBattleNrvHideMoveEnd(BossKameckStateBattleNrvHideMoveEnd::sInstance);

    void BossKameckStateBattleNrvSummonKameckWait::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeSummonKameckWait();
    }
    BossKameckStateBattleNrvSummonKameckWait(BossKameckStateBattleNrvSummonKameckWait::sInstance);

    void BossKameckStateBattleNrvSummonKameck::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeSummonKameck();
    }
    BossKameckStateBattleNrvSummonKameck(BossKameckStateBattleNrvSummonKameck::sInstance);

    void BossKameckStateBattleNrvAttackWait::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeAttackWait();
    }
    BossKameckStateBattleNrvAttackWait(BossKameckStateBattleNrvAttackWait::sInstance);

    void BossKameckStateBattleNrvAttack::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeAttack();
    }
    BossKameckStateBattleNrvAttack(BossKameckStateBattleNrvAttack::sInstance);

    void BossKameckStateBattleNrvDamage::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeDamage();
    }
    BossKameckStateBattleNrvDamage(BossKameckStateBattleNrvDamage::sInstance);

    void BossKameckStateBattleNrvRecover::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeRecover();
    }
    BossKameckStateBattleNrvRecover(BossKameckStateBattleNrvRecover::sInstance);

    void BossKameckStateBattleNrvGuard::execute(Spine* pSpine) const {
        ((BossKameckStateBattle*)pSpine->mExecutor)->exeGuard();
    }
    BossKameckStateBattleNrvGuard(BossKameckStateBattleNrvGuard::sInstance);
};