#include "SimpleNPC.h"

#ifdef GALAXY_LEVEL_ENGINE
#include "GalaxyLevelEngine.h"
#endif

SimpleNPC::SimpleNPC(const char* pName) : NPCActor(pName)
{
	mTakeOutStar = NULL;
}

void SimpleNPC::init(const JMapInfoIter& rIter)
{
	initNPCData(rIter);
	initBehaviourData(rIter);

#ifdef GALAXY_LEVEL_ENGINE
	GLE::registerAllGlobalFuncs(this);
#endif
}

extern "C"
{
	// I don't know how to represent MR::ActorTalkParam in the headers, so I'm doing this for now...
	void __kAutoMap_8034B7D0(void*, const char*); // MR::ActorTalkParam::setNoTurnAction((char const *))
	void __kAutoMap_8034CBF0(void*, const char*); // MR::ActorTalkParam::setSingleAction((char const *))
}

void SimpleNPC::initNPCData(const JMapInfoIter& rIter)
{
	const char* mObjectName;
	MR::getObjectName(&mObjectName, rIter);

	NPCActorCaps caps = NPCActorCaps(mObjectName);
	caps.mInitAppear = true;
	caps._D = true;
	caps._28 = true;
	caps.mInitScene = true;
	caps.mInitSearchTurtle = true;
	caps.mInitLodCtrlNPC = true;
	caps.mInitSpine = true;
	caps.mInitHitSensor = true;
	caps.mInitBcsvShadow = true;
	caps.mInitRailRider = true;
	caps.mUseSwitchWriteDead = true;
	caps.mUseStageSwitchSyncAppear = true;
	caps.mInitStarPointerTarget = true;
	caps.mInitTalkCtrl = true;
	caps.mInitYoshiLockOnTarget = false;

	JMapInfo* NpcInitData = MR::createInitActorCsvParser(mObjectName, NULL);
	if (NpcInitData != NULL)
	{
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionWait"))
			MR::getCsvDataStrByElement(&this->mActionWait, NpcInitData, "InitFunction", "NpcActionWait", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionWaitTurn"))
			MR::getCsvDataStrByElement(&this->mActionWaitTurn, NpcInitData, "InitFunction", "NpcActionWaitTurn", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionTalk"))
			MR::getCsvDataStrByElement(&this->mActionTalk, NpcInitData, "InitFunction", "NpcActionTalk", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionTalkTurn"))
			MR::getCsvDataStrByElement(&this->mActionTalkTurn, NpcInitData, "InitFunction", "NpcActionTalkTurn", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionSpin"))
			MR::getCsvDataStrByElement(&this->mActionSpinName, NpcInitData, "InitFunction", "NpcActionSpin", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionTrample"))
			MR::getCsvDataStrByElement(&this->mActionTrampledName, NpcInitData, "InitFunction", "NpcActionTrample", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionPointing"))
			MR::getCsvDataStrByElement(&this->mActionPointingName, NpcInitData, "InitFunction", "NpcActionPointing", "Data");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcActionReaction"))
			MR::getCsvDataStrByElement(&this->mActionReactionName, NpcInitData, "InitFunction", "NpcActionReaction", "Data");

		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcRotate"))
			this->_EC = MR::isInitFunctionBoolSet(NpcInitData, "NpcRotate");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcRotateTalk"))
			this->_ED = MR::isInitFunctionBoolSet(NpcInitData, "NpcRotateTalk");
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcYoshiTarget"))
			caps.mInitYoshiLockOnTarget = MR::isInitFunctionBoolSet(NpcInitData, "NpcYoshiTarget");
	}

	NPCActor::initialize(rIter, caps, mObjectName, NULL, false);

	if (NpcInitData != NULL)
	{
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcTalkDist"))
		{
			const char* pTemp;
			JMapInfoIter tempIter(MR::getCsvDataStrByElement(&pTemp, NpcInitData, "InitFunction", "NpcTalkDist", "Data"));
			if (tempIter.isValid())
				MR::getCsvDataF32(&this->mTalkCtrl->_40, NpcInitData, "Param00F32", tempIter.mIndex); // This doubles as a call to MR::setDistanceToTalk() !
		}
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcGoods"))
		{
			s32 arg = -1;
			MR::getJMapInfoArg3NoInit(rIter, &arg);
			NPCActorItem item(mObjectName, "", "", "", "");
			MR::getNPCItemData(&item, arg);
			equipment(item, false);
		}
		if (MR::hasCsvDataItem(NpcInitData, "InitFunction", "NpcAnimScale"))
		{
			const char* pJointName;
			MR::getCsvDataStrByElement(&pJointName, NpcInitData, "InitFunction", "NpcAnimScale", "Data");

			AnimScaleParam* newParam = new AnimScaleParam();

			const JMapInfo* AnimScaleData = MR::tryCreateCsvParser(mObjectName, "AnimScaleParam.bcsv");
			if (AnimScaleData != NULL)
			{
				MR::getParamDataF32(&newParam->_0, AnimScaleData, 0, 0);
				MR::getParamDataF32(&newParam->_4, AnimScaleData, 1, 0);
				MR::getParamDataF32(&newParam->_8, AnimScaleData, 2, 0);
				MR::getParamDataF32(&newParam->_C, AnimScaleData, 3, 0);
				MR::getParamDataF32(&newParam->_10, AnimScaleData, 4, 0);
				MR::getParamDataF32(&newParam->_14, AnimScaleData, 5, 0);
				MR::getParamDataF32(&newParam->_18, AnimScaleData, 6, 0);
				MR::getParamDataF32(&newParam->_1C, AnimScaleData, 7, 0);
				MR::getParamDataF32(&newParam->_20, AnimScaleData, 8, 0);
				MR::getParamDataF32(&newParam->_24, AnimScaleData, 9, 0);
				MR::getParamDataF32(&newParam->_28, AnimScaleData, 10, 0);
				MR::getParamDataS32(&newParam->_2C, AnimScaleData, 11, 0);
				MR::getParamDataF32(&newParam->_30, AnimScaleData, 12, 0);
			}
			mAnimScaleCtrl = new AnimScaleController(newParam);
			JointControlDelegator<SimpleNPC>* jointCtrl = new JointControlDelegator<SimpleNPC>();
			jointCtrl->mObjPtr = this;
			jointCtrl->mCalcJointMtxFunc = &NPCActor::calcJointScale;
			jointCtrl->mCalcJointMtxAfterChildFunc = NULL;

			MR::setJointControllerParam(jointCtrl, this, pJointName);
			mJointCtrlParam = jointCtrl;
		}
	}

	if (MR::isExistBrk(this, "ColorChange"))
	{
		f32 arg = 0.0f;
		MR::getJMapInfoArg0NoInit(rIter, &arg);
		MR::startBrkAndSetFrameAndStop(this, "ColorChange", arg);
	}
	if (MR::isExistBtp(this, "TexChange"))
	{
		f32 arg = 0.0f;
		MR::getJMapInfoArg1NoInit(rIter, &arg);
		MR::startBtpAndSetFrameAndStop(this, "TexChange", arg);
	}

	s32 InstanceParameters = 0;
	MR::getJMapInfoArg4NoInit(rIter, &InstanceParameters);
	if (InstanceParameters & 0x00000001)
	{
		MR::declarePowerStar(this);
		mTakeOutStar = new TakeOutStar(this, "TakeOutStar", "TakeOutStar", &NrvSimpleNPC::NrvNull::sInstance, 0);
	}
}


void SimpleNPC::initBehaviourData(const JMapInfoIter& rIter) {
	const char* mObjectName;
	MR::getObjectName(&mObjectName, rIter);

	const JMapInfo* NpcBehaviourData = MR::tryCreateCsvParser(mObjectName, "NpcBehavior.bcsv");
	if (NpcBehaviourData != NULL)
	{
		s32 arg = -1;
		MR::getJMapInfoArg2NoInit(rIter, &arg);
		if (arg <= -1 || arg >= MR::getCsvDataElementNum(NpcBehaviourData))
			goto SkipBehaviour; //TODO: Once this function is done, refractor out the use of Goto...

		const char* behaviourType;
		MR::getCsvDataStrOrNULL(&behaviourType, NpcBehaviourData, "Type", arg);
		if (behaviourType == NULL)
			goto SkipBehaviour;

		const char* Param00Str;
		const char* Param01Str;
		const char* Param02Str;
		MR::getCsvDataStrOrNULL(&Param00Str, NpcBehaviourData, "Param00Str", arg);
		MR::getCsvDataStrOrNULL(&Param01Str, NpcBehaviourData, "Param01Str", arg);
		MR::getCsvDataStrOrNULL(&Param02Str, NpcBehaviourData, "Param02Str", arg);

		// I would've made this a switch statement if it weren't for the fact that const char* doesn't work as a "Case" condition (because of course it doesn't...)
		if (MR::isEqualString(behaviourType, "NoTurn"))
		{
			if (Param00Str != NULL)
				__kAutoMap_8034B7D0(&this->_EC, Param00Str);
		}
		else if (MR::isEqualString(behaviourType, "Single"))
		{
			if (Param00Str != NULL)
				__kAutoMap_8034CBF0(&this->_EC, Param00Str);
		}
		else if (MR::isEqualString(behaviourType, "WaitTurnWalk"))
		{
			if (Param00Str != NULL)
			{
				mActionWait = Param00Str;
				mActionTalk = Param00Str;
			}
			if (Param01Str != NULL)
			{
				mActionWaitTurn = Param01Str;
				mActionTalkTurn = Param01Str;
			}
			if (Param02Str != NULL)
			{
				_120 = Param02Str;
				_124 = Param02Str;
			}
		}


		// IT'S NOT SPAGHETTI I SWEAR
	SkipBehaviour:;
	}
}

namespace NrvSimpleNPC
{
	void NrvNull::execute(Spine* pSpine) const {
		//do nothing for now
	}
	NrvNull(NrvNull::sInstance);

	void NrvTalk::execute(Spine* pSpine) const {
		((SimpleNPC*)pSpine->mExecutor)->exeTalk();
	}
	NrvTalk(NrvTalk::sInstance);

	void NrvWait::execute(Spine* pSpine) const {
		((SimpleNPC*)pSpine->mExecutor)->exeWait();
	}
	NrvWait(NrvWait::sInstance);

	void NrvReaction::execute(Spine* pSpine) const {
		MR::tryStartReactionAndPopNerve(((NPCActor*)pSpine->mExecutor));
	}
	NrvReaction(NrvReaction::sInstance);
}

namespace MR {
	void getParamDataS32(s32* pDest, const JMapInfo* pCsvData, s32 ParamID, s32 Row) {
		char Buffer[0xC];
		snprintf(Buffer, 0xC, "Param%02dInt", ParamID);
		MR::getCsvDataS32(pDest, pCsvData, Buffer, Row);
	}
	void getParamDataF32(f32* pDest, const JMapInfo* pCsvData, s32 ParamID, s32 Row) {
		char Buffer[0xC];
		snprintf(Buffer, 0xC, "Param%02dF32", ParamID);
		MR::getCsvDataF32(pDest, pCsvData, Buffer, Row);
	}
}