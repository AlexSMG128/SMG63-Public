#include "GhostPlayer.h"

#include "Game/Camera/CameraTargetArg.h"
#include "Game/Camera/CameraTargetMtx.h"
#include "Game/Player/J3DModelX.h"
#include "Game/Player/MarioAccess.h"
#include "Game/Player/MarioHolder.h"
#include "Game/Player/RushEndInfo.h"
#include "Game/NameObj/NameObjCategories.h"
#include "Game/NPC/EventDirector.h"
#include "Game/Util/ActionSoundUtil.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActorInitUtil.h"
#include "Game/Util/ActorMovementUtil.h"
#include "Game/Util/ActorSensorUtil.h"
#include "Game/Util/ActorShadowUtil.h"
#include "Game/Util/ActorSwitchUtil.h"
#include "Game/Util/BgmUtil.h"
#include "Game/Util/DemoUtil.h"
#include "Game/Util/EffectUtil.h"
#include "Game/Util/EventUtil.h"
#include "Game/Util/FileUtil.h"
#include "Game/Util/GamePadUtil.h"
#include "Game/Util/JMapUtil.h"
#include "Game/Util/LiveActorUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/ModelUtil.h"
#include "Game/Util/MtxUtil.h"
#include "Game/Util/ObjUtil.h"
#include "Game/Util/PlayerUtil.h"
#include "Game/Util/SceneUtil.h"
#include "Game/Util/SoundUtil.h"
#include "Game/Util/StringUtil.h"
#include "Game/System/AllData/GameSequenceFunction.h"
#include "Game/System/GameSequenceInGame.h"
#include "Game/System/PlayResultInStageHolder.h"

#define GHOSTNAME_MARIO "GhostMario"
#define GHOSTNAME_LUIGI "GhostLuigi"
#define GENERALPOS_RACE_END_POSITION "負け時マリオ位置"
//Change this to change the availibility of the starting boost. Longer means easier to do
#define DASHNOTICE_AMOUNT 5
#define GHOSTDATA_FILENAME "GhostPlayerData"



	GhostPlayer::GhostPlayer(const char* pName) :LiveActor(pName)
	{
		_8C = 0;
		_90 = 0;
		mCameraInfo = NULL;
		_98 = 0;

		_F8.zero();

		_113 = false;
		_118 = false;
		_115 = false;

		mGstFileData = NULL;

		mRollingRock = NULL;
	}

	void GhostPlayer::init(const JMapInfoIter& rIter)
	{
		mRaceManagerLayout = new StartCountdownLayout();
		mRaceManagerLayout->init(rIter);

		char GhostFileName[256];

		s32 ObjArg0 = 0;
		MR::getJMapInfoArg0NoInit(rIter, &ObjArg0);

		if (MR::isPlayerLuigi())
		{
			//Fetch Luigi Ghost Data
			sprintf(GhostFileName, "%s%02dLuigi.gst", GHOSTDATA_FILENAME, ObjArg0);
		}
		else
		{
			//Fetch Mario Ghost Data
			sprintf(GhostFileName, "%s%02d.gst", GHOSTDATA_FILENAME, ObjArg0);
		}
		//Updating this from SMG1 to use the Ghost archive

		if (MR::isPlayerLuigi())
			initModelManagerWithAnm(GHOSTNAME_LUIGI, NULL, NULL, false);
		else
			initModelManagerWithAnm(GHOSTNAME_MARIO, NULL, NULL, false);

		mGstFileData = new RaceDataReader(this, GhostFileName, MR::getPlayerXanimeResource());
		if (mGstFileData == NULL) //is this even possible in SMG2? Lol
		{
			OSReport("Failed to init RaceDataReader\n");
			makeActorDead();
			return;
		}

		initEffectKeeper(5, "GhostMario", false);
		MR::connectToScene(this, MR::MovementType_Player, MR::CalcAnimType_Player, MR::CategoryList_Null, MR::DrawType_Player);
		MR::initDefaultPos(this, rIter);
		mHomePosition = mTranslation;
		mVelocity.zero();
		initAnimation();
		initHitSensor(1);
		MR::addHitSensorRide(this, "Body", 4, 40.0f, TVec3f(0.0f, 80.0f, 0.0f));
		MR::initShadowVolumeSphere(this, 50.0f);
		MR::validateShadow(this, NULL);
		MR::calcGravity(this);
		MR::onCalcShadow(this, NULL);
		MR::onCalcShadowDropGravity(this, NULL);
		initSound(6, "GhostPlayer", NULL, TVec3f(0.0f));
		MR::invalidateClipping(this);
	
		if (MR::isValidInfo(rIter))
		{
			MR::initMultiActorCamera(this, rIter, &mCameraInfo, "レース開始1");
			MR::initMultiActorCamera(this, rIter, &mCameraInfo, "レース開始2");
			MR::initMultiActorCamera(this, rIter, &mCameraInfo, "レース開始3");
			MR::initMultiActorCamera(this, rIter, &mCameraInfo, "レース終了");
		}

		mCameraTargetMtx = new CameraTargetMtx("カメラターゲットダミー");

		initNerve(NrvGhostPlayerWait, 0);

		MR::needStageSwitchReadAppear(this, rIter);
		MR::syncStageSwitchAppear(this);
		makeActorDead();
		_111 = true;
		_112 = true;
		MR::getRotatedAxisZ(&_104, mRotation);
		PSMTXCopy((MtxPtr)getBaseMtx(), (MtxPtr)&mGstFileData->_8);

		mJetTurtleShadow = new JetTurtleShadow("カメシャドウモデル");
		mJetTurtleShadow->initWithoutIter();

		mOriginFixedPos = new FixedPosition(this, "All_Root", TVec3f(0.f, 0.f, 0.f), TVec3f(0.f, 0.f, 0.f));
		mHandRFixedPos = new FixedPosition(this, "HandR", TVec3f(15.59f, 42.5f, 42.93f), TVec3f(-17.05f, -0.7f, 113.55f));

		MR::declareStarPiece(this, 50);
		MR::declareCoin(this, 50);

		_124 = 0;
		_126 = 0;

		const char* pRollingRockName = MR::isPlayerLuigi() ? "GhostRockLuigiRollingRock" : "GhostRockMarioRollingRock";
		mRollingRock = new PartsModel(this, "GhostPlayerRollingRock", pRollingRockName, NULL, MR::DrawBufferType_MapObj, true);
		MR::invalidateClipping(mRollingRock);
		MR::initShadowFromCSVWithoutInitShadowVolumeSphere(mRollingRock, "Shadow");
	}

	void GhostPlayer::initAnimation()
	{
		//MUCH simpler in SMG2...
		setAnimation("基本");
		raceDataReaderSetSound(mGstFileData, true);
	}

	//Didn't feel like writing a GX header
	extern "C"
	{
		void GXInvalidateVtxCache();
	}

	void GhostPlayer::draw() const {
		if (MR::isDead(this) || _110)
			return;

		J3DModel* pModel = MR::getJ3DModel(this);
		pModel->viewCalc(); //Might need to reimplement viewCalc2...
		GXInvalidateVtxCache();
		((J3DModelX*)pModel)->directDraw(NULL); //This looks horrible...

		pModel = MR::getJ3DModel(mRollingRock);
		pModel->viewCalc();
		mOriginFixedPos->calc();
	}

	void GhostPlayer::appear() {
		LiveActor::appear();

		setNerve(NrvGhostPlayerPreStartDemo0);

		_110 = true;
		MR::invalidateShadow(this, NULL);

		if (MR::isPlayerLuigi())
		{
			MR::startBtk(this, GHOSTNAME_LUIGI);
			MR::startBrk(this, GHOSTNAME_LUIGI);
		}
		else
		{
			MR::startBtk(this, GHOSTNAME_MARIO);
			MR::startBrk(this, GHOSTNAME_MARIO);
		}

		MR::emitEffect(this, "Shadow");
	}

	void GhostPlayer::control() {
		_115 = false; //Disable Koopa Shells

		if (_111)
			return;

		if (MR::isDead(this))
			return;

		const char* pCurrentBckName = MR::getPlayingBckName(this);
		//OSReport("GhostPlayer: %s\n", pCurrentBckName);
		mEnableRollingRock = MR::isEqualString(pCurrentBckName, "RollingRockStart");

		MR::startActionSound(this, "BmLvGhostMarioAmbient", -1, -1, -1);

		if (isNerve(NrvGhostPlayerLostDemo) || isNerve(NrvGhostPlayerWinDemo))
		{
			mRollingRock->makeActorDead();
			MR::invalidateShadow(this, NULL);
			return;
		}

		if (isPlayerInPowerStarGet() && isNerve(NrvGhostPlayerRun))
		{
			setNerve(NrvGhostPlayerLostDemo);
			_113 = true;
			return;
		}
		//Is it even possible to be NULL at this point?
		if (mGstFileData == NULL || _113)
			return;

		if (MR::isNormalTalking() || MR::isSystemTalking())
			return; // not having this could actually crash the game


		if (mEnableRollingRock)
		{
			mRollingRock->makeActorAppeared();
			MR::invalidateShadow(this, NULL);
		}
		else
		{
			mRollingRock->makeActorDead();
			MR::invalidateShadow(this, NULL);
		}
		mOriginFixedPos->calc();

		if (receiveGhostPacket() != 0)
		{
			//poggers
		}
	}

	void GhostPlayer::calcAndSetBaseMtx() {
		if (MR::isDead(this))
			return;

		J3DModel* pModel = MR::getJ3DModel(this);
	
		MR::blendMtxRotate((MtxPtr)getBaseMtx(), (MtxPtr)&mGstFileData->_8, 0.2f, (MtxPtr)&pModel->_24);
		MR::setMtxTrans((MtxPtr)&pModel->_24, mTranslation.x, mTranslation.y, mTranslation.z);

		pModel->mBaseScale.x = mScale.x;
		pModel->mBaseScale.y = mScale.y;
		pModel->mBaseScale.z = mScale.z;

		if (_115)
		{
			mHandRFixedPos->calc();
			mJetTurtleShadow->calcType0((MtxPtr)&mHandRFixedPos->_1C);
		}
		if (mEnableRollingRock)
		{
			mOriginFixedPos->calc();
			mOriginFixedPos->copyTrans(&mRollingRock->mTranslation);
			mOriginFixedPos->copyRotate(&mRollingRock->mRotation);
		}
	}

	bool GhostPlayer::isPlayerInPowerStarGet()
	{
		return MR::isEqualString(MR::getPlayerCurrentBckName(), "PowerStarGet");
	}

	void GhostPlayer::attackSensor(HitSensor* pSender, HitSensor* pReceiver)
	{
		if (pReceiver->isType(ATYPE_POWER_STAR_BIND))
		{
			if (MR::getCurrentRushSensor() == NULL || !MR::getCurrentRushSensor()->isType(ATYPE_POWER_STAR_BIND))
			{
				if (MR::isPlayerConfrontDeath() || _113)
					return;

				if (isPlayerInPowerStarGet())
					return;

				setNerve(NrvGhostPlayerWinDemo);
				_113 = true;
				//MR::preventPlayerRush
				{
					MR::getMarioHolder()->getMarioActor()->setNerve((Nerve*)&sInstance__Q213NrvMarioActor19MarioActorNrvNoRush);
				}
			}
		}
		else
		{
			if (isNerve(NrvGhostPlayerRun) && MR::sendMsgPush(pReceiver, pSender) && _124 == 0)
			{
				//Unlike SMG1, the game will automatically play the correct
				//sound effect. This means we can skip checking to see
				//if we're spawning starbits underwater!
				MR::appearStarPiece(this, mTranslation, 1, 10.0f, 40.0f, false);
				_124 = 5;
			}

			if (isNerve(NrvGhostPlayerWait))
			{
				//what
			}
		}
	}

	bool GhostPlayer::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
		if (MR::isMsgJetTurtleAttack(msg))
			return true;

		if (!MR::isMsgPlayerTrample(msg))
			return false;

		if (_126 != 0)
			return false;

		MR::appearStarPiece(this, mTranslation, 5, 10.0f, 40.0f, false);
		_126 = 10;
		return true;
	}

	bool GhostPlayer::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
		return true;
	}


	bool GhostPlayer::isRequestSkipDemo() const {
		return (GameSequenceFunction::getGameSequenceInGame()->getPlayResultInStageHolder()->mMissNum > 0) && MR::testCorePadTriggerA(0);
	}
	bool GhostPlayer::receiveGhostPacket() {
		bool IsActive = mGstFileData->receivePacket();
	
		if (_124 != 0)
			_124--;
		if (_126 != 0)
			_126--;

		return IsActive;
	}
	void GhostPlayer::setAnimation(const char* pName) {
		raceDataReaderTryPlayActionSoundName(mGstFileData, pName);
	}
	void GhostPlayer::setAnimationWeight(const f32 weight) {
		//Routing this through to the RaceDataReader as well
		//but it's a bit different this time

		mGstFileData->setTrackWeight(0, weight);
		mGstFileData->setTrackWeight(1, weight);
		mGstFileData->setTrackWeight(2, weight);
		mGstFileData->setTrackWeight(3, weight);
	}
	void GhostPlayer::warpPosition(const char* pName) {
		MR::findNamePos(pName, getBaseMtx());
		PSMTXCopy(getBaseMtx(), (MtxPtr)&mGstFileData->_8);
		MR::extractMtxTrans(getBaseMtx(), &mTranslation);
	}

	void GhostPlayer::exeWait() {
		//Because of how the layout works in SMG2, do we even need to do anything?
	}
	void GhostPlayer::exePreStartDemo0() {
		if (MR::isFirstStep(this))
		{
			MR::startSound(this, "SE_BM_GHOST_MARIO_APPEAR", -1, -1);
			MR::startSound(this, "SE_BV_GHOST_MARIO_APPEAR", -1, -1);
			MR::offPlayerControl();
			mCameraTargetMtx->setMtx(getBaseMtx());
			CameraTargetArg CTA = CameraTargetArg(mCameraTargetMtx);
			MR::startMultiActorCameraTargetOther(this, mCameraInfo, "レース開始1", CTA, -1);
			warpPosition("ゴーストデモゴースト位置");
		}

		if (getNerveStep() == 1)
		{
			Mtx NamePos;
			MR::findNamePos("ゴーストデモマリオ位置", (MtxPtr)&NamePos);
			TVec3f NamePosTrans;
			MR::extractMtxTrans((MtxPtr)&NamePos, &NamePosTrans);
			MR::setPlayerPosAndWait(NamePosTrans, false);
			MR::setPlayerBaseMtx(NamePos);
			MR::startBckPlayerJ("レース見る");
			MR::resetPlayerEffect();
			MR::tryStartDemo(this, "レース準備");
			MR::requestMovementOn(this);
			MR::requestMovementOn(MR::getMarioHolder()->getMarioActor());
		}

		if (getNerveStep() == 0x3C)
		{
			_110 = false;
			setAnimation("ゴースト出現");
			MR::validateShadow(this, NULL);
		}

		if (getNerveStep() == 0x6B)
		{
			MR::startSound(this, "SE_BV_GHOST_MARIO_LAND", -1, -1);
		}

		if (getNerveStep() == 0x6F)
		{
			MR::startSound(this, "SE_BM_GHOST_MARIO_LAND", -1, -1);
		}

		if (getNerveStep() == 0x92)
		{
			MR::startSound(this, "SE_BV_GHOST_MARIO_PROVOKE", -1, -1);
		}

		if (getNerveStep() == 0xF0)
		{
			MR::endMultiActorCamera(this, mCameraInfo, "レース開始1", false, -1);
			mCameraTargetMtx->setMtx((MtxPtr)getBaseMtx());
			CameraTargetArg CTA2 = CameraTargetArg(mCameraTargetMtx);
			MR::startMultiActorCameraTargetOther(this, mCameraInfo, "レース開始2", CTA2, -1);
			setNerve(NrvGhostPlayerPreStartDemo1);
			setAnimation("レース見る");
			return;
		}

		if (!isRequestSkipDemo())
			return;

		MR::endMultiActorCamera(this, mCameraInfo, "レース開始1", false, -1);
		MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "レース開始3", -1);
		setNerve(NrvGhostPlayerPreStartDemo2);
		if (!MR::isGreaterEqualStep(this, 0x3C))
		{
			_110 = false;
			setAnimation("ゴースト出現");
			MR::validateShadow(this, NULL);
		}
	}
	void GhostPlayer::exePreStartDemo1() {
		if (getNerveStep() == 0xF0)
		{
			MR::endMultiActorCamera(this, mCameraInfo, "レース開始2", false, -1);
			MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "レース開始3", -1);
			setNerve(NrvGhostPlayerPreStartDemo2);
			return;
		}

		if (!isRequestSkipDemo())
			return;

		MR::endMultiActorCamera(this, mCameraInfo, "レース開始2", false, -1);
		MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "レース開始3", -1);
		setNerve(NrvGhostPlayerPreStartDemo2);
	}
	void GhostPlayer::exePreStartDemo2() {
		if (MR::isFirstStep(this))
		{
			mRaceManagerLayout->appear();
			setAnimation("ゴーストレース開始");
			MR::startBckPlayerJ("レース開始");
			Mtx PlayerPos;
			MR::findNamePos("レース開始時マリオ位置", (MtxPtr)&PlayerPos);
			MR::setPlayerBaseMtx((MtxPtr)&PlayerPos);
			mTranslation = mHomePosition;
			MR::endDemo(this, "レース準備");
		}

		if (MR::getPlayerTriggerZ())
		{
			MR::startBckPlayerJ("レースクラウチング開始");
			MR::startSoundPlayer("SE_PV_SQUAT", -1, -1);
		}
		else if (MR::testSubPadReleaseZ(0))
		{
			MR::startBckPlayerJ("レース開始");
		}

		//Don't need to do the countdown sfx here since the layout does it for us

		TVec3f VecNormalize = _104;
		bool WasZero = MR::normalizeOrZero(&VecNormalize);
		MtxPtr pBaseMtx = (MtxPtr)getBaseMtx();
		TVec3f VecUnk;
		VecUnk.set(pBaseMtx[0][1], pBaseMtx[1][1], pBaseMtx[2][1]);
		if (!WasZero)
		{
			TVec3f FrontVec;
			MR::calcFrontVec(&FrontVec, this);
			if (!MR::vecBlendSphere(&FrontVec, VecNormalize, FrontVec, 0.1f))
				FrontVec = VecNormalize;

			MR::makeMtxUpFront((TPos3f*)getBaseMtx(), VecUnk, FrontVec);
			//Don't update these as the gst data will reset the position instead
			//PSMTXCopy((MtxPtr)getBaseMtx(), (MtxPtr)&mGstFileData->_8);
			//MR::extractMtxTrans((MtxPtr)getBaseMtx(), &mTranslation);
		}

		//Just a bit different than SMG1
		if (mRaceManagerLayout->isStart())
		{
			_111 = false;
			MR::startSound(this, "SE_BV_GHOST_MARIO_RUN_START", -1, -1);
			setNerve(NrvGhostPlayerRun);
			MR::onPlayerControl(true);
			//MR::NoticePlayerDashChance
			{
				Mario* pMario = MR::getMarioHolder()->getMarioActor()->mMario;
				pMario->mDashNotice = DASHNOTICE_AMOUNT;
			}
			MR::startBckPlayerJ("基本");
			//Don't need to play the race start sfx, the layout does it for us
			MR::endMultiActorCamera(this, mCameraInfo, "レース開始3", false, -1);
		}
	}
	void GhostPlayer::exeLostDemo() {
		if (MR::isFirstStep(this))
		{
			warpPosition(GENERALPOS_RACE_END_POSITION);
			setAnimation("DieSwimEvent");
			mVelocity.zero();
			//setAnimation("レース負け");
		}
	}
	void GhostPlayer::exeWinDemo() {
		if (MR::isFirstStep(this))
		{
			MR::offPlayerControl();
			MR::tryPlayerKillTakingActor();
			//Reimplement MR::readyPlayerDemo
			{
				MarioActor* pMarioActor = MR::getMarioHolder()->getMarioActor();
				pMarioActor->setPlayerMode(0, true, true);
				if (pMarioActor->mIsInRush)
				{
					if (!pMarioActor->getSensor("Body")->
						receiveMessage(ACTMES_RUSH_CANCEL, pMarioActor->_E14))
					{
						pMarioActor->getSensor("Body")->
							receiveMessage(ACTMES_RUSH_CANCEL, pMarioActor->_E14);
					}

					if (pMarioActor->mIsInRush) //If you're STILL somehow in a rush
					{
						RushEndInfo REI = RushEndInfo(NULL, 4, TVec3f(0.0f), false, 0);
						MarioAccess::endRush(&REI);
					}
				}

				if (pMarioActor->mMario->isStatusActive(0x12))
					pMarioActor->mMario->closeStatus(NULL);
			}

			Mtx MarioLostMtx;
			MR::findNamePos(GENERALPOS_RACE_END_POSITION, (MtxPtr)&MarioLostMtx);
			TVec3f MarioLostPos;
			MR::extractMtxTrans((MtxPtr)&MarioLostMtx, &MarioLostPos);
			MR::setPlayerPosAndWait(MarioLostPos, false);
			MR::setPlayerBaseMtx((MtxPtr)&MarioLostMtx);
			MR::startBckPlayerJ("レース見る");
			MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "レース終了", -1);
			_114 = true;
			HitSensor* pBodySensor = getSensor("Body");
			for (s32 i = 0; i < pBodySensor->mNumSensors; i++)
			{
				if (pBodySensor->mSensors[i]->mSensorType != ATYPE_POWER_STAR_BIND) //Wrong type?
					continue;

				_118 = pBodySensor->mSensors[i]->mActor;
				mTranslation = (pBodySensor->mSensors[i]->mActor->mTranslation);
			}

			MR::startSubBGM("BGM_RACE_LOSE", false);
			//Some Race Manager Layout stuff...
			setAnimation("ゴースト勝利");
			MR::startSound(this, "SE_BV_GHOST_MARIO_WIN", -1, -1);
			MR::tryStartDemo(this, "レース終了");
			MR::requestMovementOn(this);
			//MR::requestMovementOnPlayer
			{
				MR::requestMovementOn(MR::getMarioHolder()->getMarioActor());
			}
			if (_118 != NULL)
				MR::requestMovementOn(_118);
		}

		if (_118 != NULL)
		{
			//Mathematique
			PSMTXCopy((MtxPtr)_118->getBaseMtx(), (MtxPtr)getBaseMtx());
			PSMTXCopy((MtxPtr)getBaseMtx(), (MtxPtr)&mGstFileData->_8);
			MR::extractMtxTrans((MtxPtr)getBaseMtx(), &mTranslation);
		}

		if (MR::isStep(this, 0xB4))
		{
			if (_114)
			{
				MR::endMultiActorCamera(this, mCameraInfo, "レース終了", true, -1);
				_114 = false;
			}
			MR::endDemo(this, "レース終了");
			//MR::forceKillPlayerByGhostRace
			{
				MarioAccess::forceKill(2, 0);
			}
		}
	}

	namespace NrvGhostPlayer
	{
		void HostTypeNrvWait::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exeWait();
		}
		HostTypeNrvWait(HostTypeNrvWait::sInstance);

		void HostTypeNrvWinDemo::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exeWinDemo();
		}
		HostTypeNrvWinDemo(HostTypeNrvWinDemo::sInstance);

		void HostTypeNrvLostDemo::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exeLostDemo();
		}
		HostTypeNrvLostDemo(HostTypeNrvLostDemo::sInstance);

		void HostTypeNrvPreStartDemo0::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exePreStartDemo0();
		}
		HostTypeNrvPreStartDemo0(HostTypeNrvPreStartDemo0::sInstance);

		void HostTypeNrvPreStartDemo1::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exePreStartDemo1();
		}
		HostTypeNrvPreStartDemo1(HostTypeNrvPreStartDemo1::sInstance);

		void HostTypeNrvPreStartDemo2::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exePreStartDemo2();
		}
		HostTypeNrvPreStartDemo2(HostTypeNrvPreStartDemo2::sInstance);

		void HostTypeNrvStartDemo::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exeWait();
		}
		HostTypeNrvStartDemo(HostTypeNrvStartDemo::sInstance);

		void HostTypeNrvRun::execute(Spine* pSpine) const {
			((GhostPlayer*)pSpine->mExecutor)->exeWait();
		}
		HostTypeNrvRun(HostTypeNrvRun::sInstance);
	}



	RaceDataReader* raceDataReaderSetUseSoundOff(RaceDataReader* pReader)
	{
		pReader->mUseXanimePlayer = false;
		raceDataReaderSetSound(pReader, false);
		return pReader;
	}

	kmCall(0x80376194, raceDataReaderSetUseSoundOff);


	void raceDataReaderTryPlayActionSoundName(RaceDataReader* pReader, const char* pName)
	{
		pReader->startActionName(pName);
		if (raceDataReaderIsSound(pReader))
		{
			GhostSoundData* pData = GhostPlayer::playSound(pName);
			if (pData == NULL)
				return;

			if (pData->pSfxName != NULL)
				MR::startSound(pReader->pActor, pData->pSfxName, -1, -1);

			if (pData->pVoiceName != NULL)
				MR::startSound(pReader->pActor, pData->pVoiceName, -1, -1);
		}
	}

	kmCall(0x803762F8, raceDataReaderTryPlayActionSoundName);

	void raceDataReaderTryPlayActionSoundHash(RaceDataReader* pReader, u32 hash)
	{
		pReader->startActionHash(hash);
		if (raceDataReaderIsSound(pReader))
		{
			GhostSoundData* pData = GhostPlayer::playSound(pReader->_38->getCurrentAnimationName());
			if (pData == NULL)
				return;

			if (pData->pSfxName != NULL)
				MR::startSound(pReader->pActor, pData->pSfxName, -1, -1);

			if (pData->pVoiceName != NULL)
				MR::startSound(pReader->pActor, pData->pVoiceName, -1, -1);
		}
	}
	kmCall(0x80376314, raceDataReaderTryPlayActionSoundHash);


	asm bool raceDataReaderIsSound(RaceDataReader* pReader)
	{
		lbz r3, 0x41(r3)
		blr
	}
	asm void raceDataReaderSetSound(RaceDataReader* pReader, bool toggle)
	{
		stb r4, 0x41(r3)
		blr
	}

#include "Game/System/WPadHolder.h"

namespace MR
{
	bool testSubPadReleaseZ(s32 p)
	{
		WPad* pPad = MR::getWPad(p);
		register WPadButton* pButtons = pPad->mButtons;
		register bool result;
		__asm
		{
			lwz       r0, 0xC(pButtons)
			extrwi    result, r0, 1, 18
		}
		return result;
	}


	bool isGalaxyGhostCometAppearInCurrentStage()
	{
		return EventFunction::isStartCometEvent("Ghost");
	}
}

#include "Game/LiveActor/ClippingJudge.h"
#include "Game/MapObj/PowerStar.h"

void PowerStar_initForGhostComet(PowerStar* pStar, const JMapInfoIter& rIter)
{
	if (MR::isGalaxyGhostCometAppearInCurrentStage())
		MR::invalidateClipping(pStar);

	pStar->initShadow(rIter);
}
kmCall(0x802DF688, PowerStar_initForGhostComet); //Replaces the call to PowerStar::initShadow

bool PowerStar_ProcessWaitForGhostComet()
{
	register PowerStar* pStar;
	__asm
	{
		mr pStar, r31
	}
	if (MR::isGalaxyGhostCometAppearInCurrentStage())
	{
		if (MR::getClippingJudge()->isJudgedToClipFrustum(pStar->mTranslation, 5000.f, 6))
		{
			MR::hideModelAndOnCalcAnimIfShown(pStar);
			MR::forceDeleteEffect(pStar, "Light");
		}
		else
		{
			MR::showModelIfHidden(pStar);
			MR::emitEffect(pStar, "Light");
		}
	}

	return MR::isPowerStarGetDemoActive();
}

kmCall(0x802E0E6C, PowerStar_ProcessWaitForGhostComet);


void PowerStar_exeWaitForGhostComet(PowerStar* pStar)
{
	if (!MR::isGalaxyGhostCometAppearInCurrentStage())
		MR::validateClipping(pStar);
}

kmCall(0x802E14F4, PowerStar_exeWaitForGhostComet);