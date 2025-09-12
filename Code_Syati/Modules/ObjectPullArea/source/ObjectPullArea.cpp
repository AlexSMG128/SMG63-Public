#include "ObjectPullArea.h"

ObjectPullArea::ObjectPullArea (const char *pName) : AreaObj(pName) {
	mNoSpinning = false;
	for (s32 i = 0; i < 7; i++) {
		mCheckTypes[i] = 0;
	}
	mCheckTypes[0] = ATYPE_COIN;
}

ObjectPullArea::~ObjectPullArea () {

}

void ObjectPullArea::init (const JMapInfoIter &rIter) {
	AreaObj::init(rIter);
	MR::connectToSceneAreaObj(this);
	MR::getJMapInfoArg0NoInit(rIter, &mNoSpinning);
	MR::getJMapInfoArg1NoInit(rIter, &mCheckTypes[0]);
	MR::getJMapInfoArg2NoInit(rIter, &mCheckTypes[1]);
	MR::getJMapInfoArg3NoInit(rIter, &mCheckTypes[2]);
	MR::getJMapInfoArg4NoInit(rIter, &mCheckTypes[3]);
	MR::getJMapInfoArg5NoInit(rIter, &mCheckTypes[4]);
	MR::getJMapInfoArg6NoInit(rIter, &mCheckTypes[5]);
	MR::getJMapInfoArg7NoInit(rIter, &mCheckTypes[6]);
}

void ObjectPullArea::movement () {
	if (isInVolume(*MR::getPlayerPos())) {
		ObjectPullConfig::gIsEnabled = true;
		ObjectPullConfig::gSpinRequired = !mNoSpinning;
		for (s32 i = 0; i < 7; i++) 
			ObjectPullConfig::gCheckTypes[i] = mCheckTypes[i];
	} else 
		ObjectPullConfig::gIsEnabled = false;
}

const char *ObjectPullArea::getManagerName() const {
    return "ObjectPullArea";
}

// ------------------

bool ObjectPullConfig::isPullItem (MarioActor *pMarioActor, u32 sensorType) {
	if (!gIsEnabled)
		return false;
	if (!((MarioActor *)pMarioActor->_1080)->isActionOk("カメ持ち")) 
		return false;
	for (s32 i = 0; i < 7; i++) {
		if (gCheckTypes[i] == sensorType) 
			return true;
	}
	return false;
}

void ObjectPullConfig::clear () {
	gIsEnabled = false;
	gSpinRequired = true;
	for (s32 i = 0; i < 7; i++) 
		gCheckTypes[i] = 0;
}

LiveActor *newCoinPull (HitSensor *pSensor) {
	MarioActor *pMarioActor = MarioAccess::getPlayerActor();
	u32 sensorType = pSensor->mSensorType;
	if (ObjectPullConfig::isPullItem(pMarioActor, sensorType)) {
		TVec3f v0 = pSensor->mPosition - pMarioActor->mTranslation;
		// If the radius is too small, it will cause the duplicate coin bug
		if (PSVECMag(v0) < 120.0f) {
			//((MarioActor *)pMarioActor->_1080)->tryGetItem(pSensor);
			for (s32 i = 0; i < 128; i++) {
				if (pMarioActor->_7C0[i] == pSensor) {
					pMarioActor->_7C0[i] = 0;
					break;
				}
			}
		} else {
			// For coins
			MR::onCalcShadowAll(pSensor->mActor);
			MR::tryEmitEffect(pSensor->mActor, "CoinSpinBlur");

			MR::offBind(pSensor->mActor);
			MR::invalidateClipping(pSensor->mActor);

			pMarioActor->tryCoinPullOne(pSensor);
			for (s32 i = 0; i < 128; i++) {
				if (!pMarioActor->_7C0[i]) {
					pMarioActor->_7C0[i] = pSensor;
					break;
				}
			}
		}
	}
	return MR::getSensorHost(pSensor);
}

kmCall(0x803BA270, newCoinPull);

bool checkForSpin () {
	MarioActor *pMarioActor = MarioAccess::getPlayerActor();
	if (!ObjectPullConfig::gSpinRequired) 
		return true;
	return pMarioActor->_704 && pMarioActor->_E3D < 14;
}

kmCall(0x803CBEE4, checkForSpin);
kmWrite32(0x803CBEE8, 0x2C030000); // cmpwi r3, 0
kmWrite32(0x803CBEF0, 0x60000000); // nop
kmWrite32(0x803CBEF4, 0x60000000); // nop
kmWrite32(0x803CBEF8, 0x60000000); // nop

/*
extern "C" {
	bool sub_803B2880(Mario *, const TVec3f &, const TVec3f &);
}

bool isActionOk(MarioActor *, const char *);

void tryTornadoPullO(MarioActor *pMarioActor, HitSensor *pSensor)
{
	if (isActionOk((MarioActor *)pMarioActor->_1080, "コイン引っ張り"))
	{
		u32 sensorType = pSensor->mSensorType;
		if (!strcmp(MR::getSensorHost(pSensor)->mName, "カメックビーム用カメ")) 
			sensorType = 5555;
		if (sensorType - 18 > 1) {
			if (sensorType != ATYPE_STAR_PIECE) {
				newCoinPull(pMarioActor, sensorType, pSensor);
				if (sensorType != 5555) 
					return;
				if (isActionOk((MarioActor *)pMarioActor->_1080, "カメ持ち") && pSensor->receiveMessage(ACTMES_IS_PULL_ENABLE, pMarioActor->getSensor("Body"))) {
					LiveActor *pSensorHost = MR::getSensorHost(pSensor);
					pMarioActor->tryPullTrans(&pSensorHost->mVelocity, *MR::getSensorPos(pSensor));
					pSensor->receiveMessage(ACTMES_ITEM_PULL, pMarioActor->getSensor("Body"));
				}
			} else {
				if (pMarioActor->mPlayerMode != PLAYER_MODE_FOO && !pMarioActor->mMario->isSwimming()) 
					return;
				TVec3f v0(pMarioActor->_5F4);
				TVec3f v1 = pMarioActor->mMario->_488;
				if (pMarioActor->mMario->isSwimming()) {
					TVec3f v2(pMarioActor->_7A8);
					v2.scale(600.0f);
					TVec3f v3(v0);
					v3 -= v2;
					v1 = v3;
				}
				TVec3f v4(pMarioActor->mMario->_1F0);
				v4.scale(200.0f);
				v0.add(v4);
				TVec3f v5(*MR::getSensorPos(pSensor));
				TVec3f v6;
				f32 footPoint = MR::getFootPoint(v0, v1, v5, &v6);
				if (footPoint < -0.1f || footPoint > 2.0f || PSVECDistance(v6, v5) > 600.0f) 
					return;
				Mario *pMario = pMarioActor->mMario;
				TVec3f v7(*MR::getSensorPos(pSensor));
				v7 -= pMarioActor->_5F4;
				if (sub_803B2880(pMario, pMarioActor->_5F4, v7)) 
					return;
				for (s32 i = 0; i < 128; i++) {
					if (pMarioActor->_7C0[i] == pSensor) 
						return;
				}
				pSensor->receiveMessage(ACTMES_ITEM_PULL, pMarioActor->getSensor("Body"));
				if (!pSensor->receiveMessage(ACTMES_ITEM_PULL, pMarioActor->getSensor("Body"))) 
					return;
				for (s32 i = 0; i < 128; i++) {
					if (!pMarioActor->_7C0[i]) {
						pMarioActor->_7C0[i] = pSensor;
						return;
					}
				}
			}
		} else if (isActionOk((MarioActor *)pMarioActor->_1080, "カメ持ち") && pSensor->receiveMessage(ACTMES_IS_PULL_ENABLE, pMarioActor->getSensor("Body"))) {
			if (PSVECDistance(pMarioActor->mTranslation, *MR::getSensorPos(pSensor)) < 120.0f) 
				tryGetItem((MarioActor *)pMarioActor->_1088, pSensor);
			else {
				LiveActor *pSensorHost = MR::getSensorHost(pSensor);
				pMarioActor->tryPullTrans(&pSensorHost->mVelocity, *MR::getSensorPos(pSensor));
				pSensor->receiveMessage(ACTMES_ITEM_PULL, pMarioActor->getSensor("Body"));
				pMarioActor->_70C = pSensor;
			}
		}
	}
}

kmBranch(0x803BA230, tryTornadoPullO);*/