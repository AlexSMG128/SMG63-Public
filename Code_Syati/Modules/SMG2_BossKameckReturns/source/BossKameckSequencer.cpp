#include "BossKameckSequencer.h"
#include "BossKameckBattleDemo.h"
#include "BossKameckStateBattle.h"
#include "BossKameck.h"

#include "Game/Util/ActorStateUtil.h"
#include "Game/Util/NerveUtil.h"

BossKameckSequencer::BossKameckSequencer(const char* pName) : NerveExecutor(pName) {
	mBossKameck = NULL;
	mBattleDemo = NULL;
	mBattleState = NULL;
}

void BossKameckSequencer::init(BossKameck* pBoss, const JMapInfoIter& rIter) {
	mBossKameck = pBoss;

	mBattleDemo = new BossKameckBattleDemo(pBoss, rIter);
	mBattleDemo->init();
	mBattleState = new BossKameckStateBattle(pBoss);
	mBattleState->init();
	initNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvOpeningDemo::sInstance, 0);
}

void BossKameckSequencer::start() {
	setNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvOpeningDemo::sInstance);
}

void BossKameckSequencer::update() {
	updateNerve();
}

bool BossKameckSequencer::isBattle() const {
	return isNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvBattleFirst::sInstance) || isNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvBattleLoop::sInstance);
}


void BossKameckSequencer::attackSensor(HitSensor* a1, HitSensor* a2) {
	if (isBattle()) {
		mBattleState->attackSensor(a1, a2);
	}
}

bool BossKameckSequencer::receiveMsgPlayerAttack(u32 msg, HitSensor* a2, HitSensor* a3) {
	if (isBattle()) {
		return mBattleState->receiveMsgPlayerAttack(msg, a2, a3);
	}

	return false;
}


void BossKameckSequencer::exeOpeningDemo() {
	if (MR::isFirstStep(this)) {
		mBattleDemo->startDemoAppear();
	}

	if (MR::updateActorState(this, mBattleDemo)) {
		setNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvBattleFirst::sInstance);
	}
}

void BossKameckSequencer::exeBattle(bool isSpawnStarPiece) {
	if (MR::isFirstStep(this)) {
		if (isSpawnStarPiece)
			mBossKameck->appearStarPieceToPlayer(8);
		mBattleState->setMoveRail(mBossKameck->getMoveRail(mBossKameck->mCurrentBattlePattern));
		mBattleState->setBattlePattarn(mBossKameck->mBattlePatterns[mBossKameck->mCurrentBattlePattern]);
		mBattleState->_3C = !mBossKameck->tryGoNextPattern();
		mBattleState->_30 = mBattleState->_3C ? 4 : 3;
	}

	const Nerve* nextNerve = &NrvBossKameckSequencer::BossKameckSequencerNrvBattleLoop::sInstance;
	if (mBossKameck->isNextNeedPowerUpDemo())
		nextNerve = &NrvBossKameckSequencer::BossKameckSequencerNrvPowerUpDemo::sInstance;
	else if (mBattleState->_3C)
		nextNerve = &NrvBossKameckSequencer::BossKameckSequencerNrvEndDemo::sInstance;

	MR::updateActorStateAndNextNerve(this, mBattleState, nextNerve);
}

void BossKameckSequencer::exePowerUpDemo() {
	if (MR::isFirstStep(this)) {
		mBattleDemo->startDemoPowerUp();
	}

	if (MR::updateActorState(this, mBattleDemo)) {
		setNerve(&NrvBossKameckSequencer::BossKameckSequencerNrvBattleFirst::sInstance);
	}
}

void BossKameckSequencer::exeEndDemo() {
	if (MR::isFirstStep(this)) {
		mBattleDemo->startDemoDown();
	}

	MR::updateActorState(this, mBattleDemo);
}


namespace NrvBossKameckSequencer {
	void BossKameckSequencerNrvOpeningDemo::execute(Spine* pSpine) const {
		((BossKameckSequencer*)pSpine->mExecutor)->exeOpeningDemo();
	}
	BossKameckSequencerNrvOpeningDemo(BossKameckSequencerNrvOpeningDemo::sInstance);


	void BossKameckSequencerNrvBattleFirst::execute(Spine* pSpine) const {
		((BossKameckSequencer*)pSpine->mExecutor)->exeBattle(false);
	}
	BossKameckSequencerNrvBattleFirst(BossKameckSequencerNrvBattleFirst::sInstance);


	void BossKameckSequencerNrvBattleLoop::execute(Spine* pSpine) const {
		((BossKameckSequencer*)pSpine->mExecutor)->exeBattle(true);
	}
	BossKameckSequencerNrvBattleLoop(BossKameckSequencerNrvBattleLoop::sInstance);


	void BossKameckSequencerNrvPowerUpDemo::execute(Spine* pSpine) const {
		((BossKameckSequencer*)pSpine->mExecutor)->exePowerUpDemo();
	}
	BossKameckSequencerNrvPowerUpDemo(BossKameckSequencerNrvPowerUpDemo::sInstance);


	void BossKameckSequencerNrvEndDemo::execute(Spine* pSpine) const {
		((BossKameckSequencer*)pSpine->mExecutor)->exeEndDemo();
	}
	BossKameckSequencerNrvEndDemo(BossKameckSequencerNrvEndDemo::sInstance);
};