#include "BossKameck.h"
#include "BossKameckStateBattle.h"

#include "syati.h"

extern "C"
{
	PowerStar* __kAutoMap_802E1D70(int);
}

// ObjArg0: Level Number 
//          Determines what Battle Data archive to load. (BossKameckVsX, where X is ObjArg0)
// ObjArg1: Power Star ID
//          The Power Star ID to register to. -1 to disable. (SW_DEAD works with and without a star)
// ObjArg7: Camera Related
//          Camera vector or something idk it's the same as SMG1

BossKameck::BossKameck(const char* pName) : LiveActor(pName) {
	mBattlePatterns = NULL;
	mBattlePatternCount = 0;
	mCurrentBattlePattern = 0;

	mKameckFollowerGroupList = NULL;
	mKameckFollowerGroupListCount = 0;

	mMoveRails = NULL;
	mMoveRailCount = 0;

	mBarrierList = NULL;
	mBarrierListCount = 0;

	mJointCtrl = NULL;
	mPowerStarId = -1;
	mCameraVectorId = -1;

	_90 = TQuat4f(0.0f, 0.0f, 0.0f, 1.0f);
	_A0 = TVec3f(0, 0, 1);
	_C8 = TVec3f(0);

	mSequencer = NULL;
	mBeamListener = NULL;

	mMasterBeamList = NULL;
	mMasterBeamListCount = NULL;

	mIsUseLv2Anim = false;
}

void BossKameck::init(const JMapInfoIter& rIter) {
	MR::processInitFunction(this, rIter, false);

	MR::makeQuatAndFrontFromRotate(&_90, &_A0, this);
	MR::calcGravity(this);
	MR::invalidateClipping(this);
	initBattle(rIter);
	initFollowers(rIter);
	initMoveRail(rIter);
	MR::getJMapInfoArg1NoInit(rIter, &mPowerStarId);
	if (mPowerStarId > 0)
		MR::declarePowerStar(this, mPowerStarId);
	initBeam();

	MR::listenStageSwitchOnA(this, MR::FunctorV0M<BossKameck*, void (BossKameck::*)(void)>(this, &BossKameck::startSequence));
	makeActorDead();

	mJointCtrl = new ActorJointCtrl(this);
	MR::addToAttributeGroupSearchTurtle(this);

	MR::getJMapInfoArg6NoInit(rIter, &mIsUseLv2Anim);
	MR::getJMapInfoArg7WithInit(rIter, &mCameraVectorId);
	if (mCameraVectorId != -1)
		MR::declareCameraRegisterVec(this, mCameraVectorId, &mTranslation);

	OSReport("BossKameck - Init Success\n");

	//mKameckFollowerGroupList[0]->mHolder->appearAll(); //I used this for testing
}
void BossKameck::initAfterPlacement() {
	initStar();
}

void BossKameck::initBattle(const JMapInfoIter& rIter) {
	s32 BattleArg = 0;
	MR::getJMapInfoArg0NoInit(rIter, &BattleArg);
	char BattleArchiveName[32];
	snprintf(BattleArchiveName, 32, "/ObjectData/BossKameckVs%d.arc", BattleArg);
	if (!MR::isFileExist(BattleArchiveName, false))
		JUTException::panic_f(__FILE__, __LINE__, "%s isn't exist.\n", BattleArchiveName);
	ResourceHolder* pResourceHolder = MR::createAndAddResourceHolder(BattleArchiveName);
	if (!MR::isExistFileInArc(pResourceHolder, "BattleData.bcsv"))
		JUTException::panic_f(__FILE__, __LINE__, "BattleData.bcsv isn't exist in %s.arc.\n", BattleArchiveName);

	const s32 SIZE = 8; // Why would anyone want more than like, 8 of these...
	mBarrierList = new BossKameckBarrierEntry*[SIZE];
	for (s32 i = 0; i < SIZE; i++) {
		mBarrierList[i] = new BossKameckBarrierEntry();
		mBarrierList[i]->mName = NULL;
		mBarrierList[i]->mActor = NULL;
	}

	const JMapInfo* pBattleData = MR::createCsvParser(pResourceHolder, "BattleData.bcsv");
	mBattlePatternCount = MR::getCsvDataElementNum(pBattleData);
	mBattlePatterns = new BossKameckBattlePattern*[mBattlePatternCount];
	for (s32 i = 0; i < mBattlePatternCount; i++)
	{
		if (!MR::isExistFileInArc(pResourceHolder, "BeamPatternLv%d.bcsv", i+1))
			JUTException::panic_f(__FILE__, __LINE__, "BeamPatternLv%d.bcsv isn't exist in %s.arc.\n", i+1, BattleArchiveName);

		const JMapInfo* pBeamData = MR::createCsvParser(pResourceHolder, "BeamPatternLv%d.bcsv", i + 1);

		// Ready to create the Battle Pattern!
		BossKameckBattlePattern* currentPattern = new BossKameckBattlePattern();
		MR::getCsvDataS32(&currentPattern->mFollowerGroupId, pBattleData, "FollowerGroupId", i);
		MR::getCsvDataBool(&currentPattern->mFollowerAllowRespawn, pBattleData, "AllowFollowerReappear", i);
		MR::getCsvDataStrOrNULL(&currentPattern->mPowerUpAnimName, pBattleData, "AnimName", i);
		MR::getCsvDataS32(&currentPattern->mMovementType, pBattleData, "MovementType", i);

		const char* pBarrierName;
		MR::getCsvDataStrOrNULL(&pBarrierName, pBattleData, "Barrier", i);
		currentPattern->mBarrier = initBarrier(rIter, pBarrierName);
		if (currentPattern->mBarrier != NULL)
			currentPattern->mBarrier->makeActorDead();

		currentPattern->mBeamInfoCurrentIdx = 0;
		currentPattern->mBeamInfoCount = MR::getCsvDataElementNum(pBeamData);
		currentPattern->mBeamInfo = new BossKameckBattleBeamEntry*[currentPattern->mBeamInfoCount];
		for (s32 j = 0; j < currentPattern->mBeamInfoCount; j++)
		{
			BossKameckBattleBeamEntry* currentBeam = new BossKameckBattleBeamEntry();
			MR::getCsvDataS32(&currentBeam->mBeamType, pBeamData, "Beam", j);
			MR::getCsvDataS32(&currentBeam->mCount, pBeamData, "Count", j);
			currentPattern->mBeamInfo[j] = currentBeam;
		}

		mBattlePatterns[i] = currentPattern;
	}

	MR::declareStarPiece(this, mBattlePatternCount * 8);

	mSequencer = new BossKameckSequencer("BossKameckSequencer");
	mSequencer->init(this, rIter);
}
void BossKameck::initFollowers(const JMapInfoIter& rIter) {
	s32 childNum = MR::getChildObjNum(rIter);
	s32 kameckChildNum = 0;
	const char* objName;

	for (s32 i = 0; i < childNum; i++) {
		MR::getChildObjName(&objName, rIter, i);

		if (MR::isEqualString(objName, "BossKameckFollowerGroup")) {
			kameckChildNum++;
		}
	}

	if (kameckChildNum <= 0)
		return;

	mKameckFollowerGroupListCount = kameckChildNum;
	mKameckFollowerGroupList = new BossKameckFollowerGroup*[mKameckFollowerGroupListCount];

	s32 id = 0;
	for (s32 i = 0; i < childNum; i++) {
		MR::getChildObjName(&objName, rIter, i);

		if (MR::isEqualString(objName, "BossKameckFollowerGroup")) {
			BossKameckFollowerGroup* currentHolder = new BossKameckFollowerGroup("BossKameckFollowerGroup");
			MR::initChildObj(currentHolder, rIter, i);

			mKameckFollowerGroupList[id] = currentHolder;
			id++;
		}
	}
}
void BossKameck::initMoveRail(const JMapInfoIter& rIter) {
	s32 childNum = MR::getChildObjNum(rIter);
	mMoveRailCount = 0;
	const char* objName;

	for (s32 i = 0; i < childNum; i++) {
		MR::getChildObjName(&objName, rIter, i);

		if (MR::isEqualString(objName, "BossKameckMoveRail")) {
			mMoveRailCount++;
		}
	}

	if (mMoveRailCount <= 0)
		return;
	
	mMoveRails = new BossKameckMoveRail*[mMoveRailCount];
	s32 curRails = 0;
	for (s32 j = 0; j < childNum; j++) {
		MR::getChildObjName(&objName, rIter, j);

		if (MR::isEqualString(objName, "BossKameckMoveRail")) {
			mMoveRails[curRails] = new BossKameckMoveRail("BossKameckMoveRail");
			MR::initChildObj(mMoveRails[curRails], rIter, j);
			curRails++;
		}
	}
}
LiveActor* BossKameck::initBarrier(const JMapInfoIter& rIter, const char* pName) {
	if (pName == NULL)
		return NULL;
	
	for (s32 i = 0; i < mBarrierListCount; i++)
	{
		if (MR::isEqualString(mBarrierList[i]->mName, pName))
			return mBarrierList[i]->mActor;
	}

	s32 childNum = MR::getChildObjNum(rIter);
	const char* objName;
	for (s32 i = 0; i < childNum; i++)
	{
		MR::getChildObjName(&objName, rIter, i);

		if (MR::isEqualString(objName, pName))
		{
			LiveActor* pActor = (LiveActor*)NameObjFactory::initChildObj(rIter, i); // Somewhat unsafe, but hopefully will be OK...
			mBarrierList[mBarrierListCount]->mName = pName;
			mBarrierList[mBarrierListCount]->mActor = pActor;
			mBarrierListCount++;
			return pActor;
		}
	}
	return NULL;
}
void BossKameck::initStar() {
	if (mPowerStarId > 0)
	{
		if (MR::isExistBtp(this, "PowerStarColor"))
		{
			PowerStar* pStar = __kAutoMap_802E1D70(mPowerStarId);
			MR::startBtpAndSetFrameAndStop(this, "PowerStarColor", static_cast<f32>(pStar->mColor));
		}

		s32 hasPowerStar = MR::hasPowerStarInCurrentStageWithDeclarer(mName, mPowerStarId);
		MR::startBrkAndSetFrameAndStop(this, "Star", static_cast<f32>(hasPowerStar));
	}
	else
		MR::startBrkAndSetFrameAndStop(this, "Star", 0.f);
}
void BossKameck::initBeam() {
	mBeamListener = new BossKameckBeamEventListener(this);

	// This is a tough one...
	// We need to not have so many duplicates

	// Okay first we must figure out how many unique Beam Holders we will be needing
	const s32 SIZE = 64; // I'll be shocked if this works
	BossKameckBattleBeamEntry* AllHolderListing[SIZE];
	s32 AllHolderListingEnd = 0;
	for (s32 i = 0; i < SIZE; i++)
		AllHolderListing[i] = NULL;

	for (s32 i = 0; i < mBattlePatternCount; i++)
	{
		BossKameckBattlePattern* pPattern = mBattlePatterns[i];
		for (s32 j = 0; j < pPattern->mBeamInfoCount; j++)
		{
			BossKameckBattleBeamEntry* pBeamEntry = pPattern->mBeamInfo[j];
			bool IsAlreadyContainBeamType = false;
			for (s32 x = 0; x < AllHolderListingEnd; x++)
				if (AllHolderListing[x]->mBeamType == pBeamEntry->mBeamType && AllHolderListing[x]->mCount == pBeamEntry->mCount) {
					IsAlreadyContainBeamType = true;
					break;
				}

			if (!IsAlreadyContainBeamType) {
				AllHolderListing[AllHolderListingEnd] = pBeamEntry;
				AllHolderListingEnd++;
			}
		}
	}

	mMasterBeamListCount = AllHolderListingEnd;
	OSReport("BossKameck - Master Beam List Count = %d\n", mMasterBeamListCount);
	mMasterBeamList = new KameckBeamHolder*[mMasterBeamListCount];
	for (s32 i = 0; i < mMasterBeamListCount; i++)
	{
		// The normal magikoopas init 3 beams instead of 2, but since Kamella is beam heavy I figured I may as well keep it low if possible...
		BossKameckBattleBeamEntry* Current = AllHolderListing[i];
		KameckBeamHolder* pHolder = new KameckBeamHolder(Current->mBeamType, 2, Current->mCount);
		mMasterBeamList[i] = pHolder;

		for (s32 j = 0; j < mBattlePatternCount; j++)
		{
			BossKameckBattlePattern* pPattern = mBattlePatterns[j];
			for (s32 k = 0; k < pPattern->mBeamInfoCount; k++)
			{
				BossKameckBattleBeamEntry* pBeamEntry = pPattern->mBeamInfo[k];
				if (Current->mBeamType == pBeamEntry->mBeamType && Current->mCount == pBeamEntry->mCount)
					pBeamEntry->mBeamHolderRef = pHolder;
			}
		}
	}
}

void BossKameck::makeActorDead() {
	LiveActor::makeActorDead();
	MR::invalidateShadowAll(this);
}

void BossKameck::control() {
	if (mSequencer != NULL) {
		mSequencer->update();
	}

	//mActorList->removeDeadActor();
	_C8.set(mVelocity);
	mJointCtrl->update();
}

void BossKameck::calcAndSetBaseMtx() {
	MR::setBaseTRMtx(this, _90);
	MR::setBaseScale(this, mScale);
	mJointCtrl->setCallBackFunction();
}

void BossKameck::attackSensor(HitSensor* a1, HitSensor* a2) {
	if (mSequencer != NULL)
		mSequencer->attackSensor(a1, a2);
}

bool BossKameck::receiveMsgPlayerAttack(u32 msg, HitSensor* a1, HitSensor* a2) {
	if (mSequencer != NULL)
		return mSequencer->receiveMsgPlayerAttack(msg, a1, a2);

	return false;
}


BossKameckFollowerGroup* BossKameck::getFollowerGroup(s32 no) const {
	for (s32 i = 0; i < mKameckFollowerGroupListCount; i++)
		if (mKameckFollowerGroupList[i]->mId == no)
			return mKameckFollowerGroupList[i];

	return NULL;
}
BossKameckMoveRail* BossKameck::getMoveRail(s32 no) {
	for (s32 i = 0; i < mMoveRailCount; i++)
		if (mMoveRails[i]->_8C == no)
			return mMoveRails[i];

	return NULL;
}
s32 BossKameck::getLivingKameckNum() const {
	BossKameckFollowerGroup* pCurrentGroup = tryGetCurrentFollowerGroup();
	if (pCurrentGroup == NULL)
	{
		s32 LivingNum = 0;
		for (s32 i = 0; i < mKameckFollowerGroupListCount; i++)
			LivingNum += mKameckFollowerGroupList[i]->getLivingActorNum();

		return LivingNum;
	}
	return pCurrentGroup->getLivingActorNum();
}
KameckBeamHolder* BossKameck::tryGetBeamHolder(BossKameckBattlePattern* pPattern) {
	if (pPattern->mBeamInfoCount <= 0)
		return NULL;
	s32 id = pPattern->mBeamInfoCurrentIdx;
	BossKameckBattleBeamEntry* pBeamInfo = pPattern->mBeamInfo[id++];
	if (id >= pPattern->mBeamInfoCount)
		id = 0;
	pPattern->mBeamInfoCurrentIdx = id;
	return pBeamInfo->mBeamHolderRef;
}

void BossKameck::startSequence() {
	mCurrentBattlePattern = 0;
	mSequencer->start();
	appear();
	MR::hideModel(this);
}
bool BossKameck::tryGoNextPattern() {
	OSReport("BossKameck - Pattern %d -> %d / %d\n", mCurrentBattlePattern, mCurrentBattlePattern+1, mBattlePatternCount);
	mCurrentBattlePattern++;
	if (mCurrentBattlePattern >= mBattlePatternCount)
		return false;
	return true;
}
const char* BossKameck::tryGetCurrentAnimName() const {
	if (mCurrentBattlePattern >= mBattlePatternCount)
		return NULL;
	return mBattlePatterns[mCurrentBattlePattern]->mPowerUpAnimName;
}
bool BossKameck::isNextNeedPowerUpDemo() const {
	return tryGetCurrentAnimName() != NULL;
}
BossKameckFollowerGroup* BossKameck::tryGetCurrentFollowerGroup() const {
	BossKameckBattlePattern* pPattern = mSequencer->mBattleState->mBattlePattarn;
	if (pPattern == NULL)
		return NULL;
	if (pPattern->mFollowerGroupId < 0)
		return NULL;
	return getFollowerGroup(pPattern->mFollowerGroupId);
}
KameckBeamHolder* BossKameck::tryGetCurrentBeamHolder() {
	if (mCurrentBattlePattern >= mBattlePatternCount)
		return NULL;
	return tryGetBeamHolder(mBattlePatterns[mCurrentBattlePattern]);
}

void BossKameck::startDemoAppearKameck() {
	s32 next = mCurrentBattlePattern;
	OSReport("BossKameck -- startDemoAppearKameck (%d/%d)\n", next, mBattlePatternCount);
	if (next >= mBattlePatternCount)
		return;
	if (mBattlePatterns[next]->mFollowerGroupId < 0)
		return;
	BossKameckFollowerGroup* mFollowerGroup = getFollowerGroup(mBattlePatterns[next]->mFollowerGroupId);
	OSReport("BossKameck -- startDemoAppearKameck %s\n", (mFollowerGroup == NULL ? "false" : "true"));
	if (mFollowerGroup != NULL)
		mFollowerGroup->startDemoAppear();
}
void BossKameck::endDemoAppearKameck() {
	s32 next = mCurrentBattlePattern;
	if (next >= mBattlePatternCount)
		return;
	if (mBattlePatterns[next]->mFollowerGroupId < 0)
		return;
	BossKameckFollowerGroup* mFollowerGroup = getFollowerGroup(mBattlePatterns[next]->mFollowerGroupId);
	if (mFollowerGroup != NULL)
		mFollowerGroup->endDemoAppear();
}
void BossKameck::appearKameck(s32 id) {
	BossKameckFollowerGroup* mFollowerGroup = getFollowerGroup(id);
	if (mFollowerGroup != NULL && mFollowerGroup->mHolder != NULL)
		mFollowerGroup->reviveAll();
}
void BossKameck::deadKameck() {
	for (s32 i = 0; i < mKameckFollowerGroupListCount; i++)
		mKameckFollowerGroupList[i]->deadForceAll();
}

void BossKameck::killAllBeam() {
	for (s32 i = 0; i < mMasterBeamListCount; i++)
	{
		KameckBeamHolder* CurrentBeamList = mMasterBeamList[i];
		CurrentBeamList->makeAllDead();
	}
}

void BossKameck::startDemo() {
	_C8.zero();
	MR::zeroVelocity(this);
	MR::calcGravity(this);
	mJointCtrl->endDynamicCtrl("Suso1", 0);
	mJointCtrl->endDynamicCtrl("Cap1", 0);
}
void BossKameck::endDemo() {
	_C8.zero();
	MR::zeroVelocity(this);
	MR::calcGravity(this);
	mJointCtrl->startDynamicCtrl("Suso1", -1);
	mJointCtrl->startDynamicCtrl("Cap1", -1);
}


void BossKameck::setPose(MtxPtr mtx) {
	TPos3f pos;
	pos.set(mtx);
	pos.getQuat(_90);

	f32 z = pos.mMtx[2][3];
	f32 y = pos.mMtx[1][3];
	f32 x = pos.mMtx[0][3];
	mTranslation.set(x, y, z);

	f32 v1 = (2.0f * (_90.x * _90.z)) + (2.0f * (_90.w * _90.y));
	f32 v2 = (2.0f * (_90.y * _90.z)) - (2.0f * (_90.w * _90.x));
	f32 v3 = (1.0f - (2.0f * (_90.x * _90.x))) - (2.0f * (_90.y * _90.y));
	_A0.set(v1, v2, v3);
}
void BossKameck::updatePose() {
	TVec3f negGravity(-mGravity);
	TVec3f var21(negGravity);
	TVec3f velocity(mVelocity);
	velocity.sub(_C8); //Presumably getting a velocity difference
	f32 dotresult = mGravity.dot(velocity);
	TVec3f ScaleAddResult;
	JMAVECScaleAdd((const Vec*)&mGravity, (const Vec*)&velocity, (Vec*)&ScaleAddResult, -dotresult);
	f32 magnitude = PSVECMag((const Vec*)ScaleAddResult);

	if (!MR::isNearZero(magnitude, 0.001f)) {
		f32 normalize = MR::normalize(magnitude, 0.0f, 3.0f);
		TVec3f v13(ScaleAddResult);
		TVec3f* ref = &var21; //r31

		v13.scale((4.0f * normalize) / magnitude); //TODO: This is maybe wrong in Petari. Experiment and see...
		ref->add(v13);

		if (!MR::isNearZero(*ref, 0.001f)) {
			MR::normalize(ref);
		}
		else {
			ref->set(-mGravity);
		}
	}

	MR::blendQuatUpFront(&_90, var21, _A0, 0.039999999f, 0.2f);
}


void BossKameck::hitBeam(s32 beamType) {
	// SMG1 had a switch case that was completely useless, so I just removed it...
	if (!MR::isDead(this))
		MR::startSound(this, "SE_BV_KAMECK_ATK_SUCCESS", -1, -1);
}


void BossKameck::appearStarPieceToUp(s32 num) {
	TVec3f neg = -mGravity;
	MR::appearStarPieceToDirection(this, mTranslation, neg, num, 30.0f, 40.0f, false);
}
void BossKameck::appearStarPieceToPlayer(s32 num) {
	TVec3f dir;
	MR::calcVecToPlayerH(&dir, this, NULL);
	dir.sub(mGravity);
	MR::normalizeOrZero(&dir);
	MR::appearStarPieceToDirection(this, mTranslation, dir, num, 30.0f, 65.0f, false);
}