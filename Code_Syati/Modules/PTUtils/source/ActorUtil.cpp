#include "ActorUtil.h"

/*
* Authors: Aurum
*/

namespace pt {
	void moveAndTurnToPlayer(LiveActor *pActor, const MR::ActorMoveParam &rParam) {
		MR::moveAndTurnToPlayer(pActor, rParam._0, rParam._4, rParam._8, rParam._C);
	}

	void moveAndTurnToDirection(LiveActor *pActor, const TVec3f &rDir, const MR::ActorMoveParam &rParam) {
		MR::moveAndTurnToDirection(pActor, rDir, rParam._0, rParam._4, rParam._8, rParam._C);
	}

	void turnToDirectionUpFront(LiveActor *pActor, TVec3f rUp, TVec3f rFront) {
		TMtx34f mtx;
		MR::makeMtxUpFront((TPos3f*)&mtx, rUp, rFront);
		((TRot3f*)&mtx)->getEulerXYZ(pActor->mRotation);
		pActor->mRotation *= 57.295776f;
	}

	void turnToDirectionGravityFront(LiveActor *pActor, TVec3f rFront) {
		turnToDirectionUpFront(pActor, -pActor->mGravity, rFront);
	}

	void initRailToNearestAndRepositionWithGravity(LiveActor *pActor) {
		RailRider *pRider = pActor->mRailRider;
		pRider->setCoord(pRider->calcNearestPos(pActor->mTranslation));
		getTransRotateFromRailWithGravity(pActor);
	}

	void getTransRotateFromRailWithGravity(LiveActor *pActor) {
		RailRider *pRider = pActor->mRailRider;
		pActor->mTranslation.set(pRider->mCurrentPos);
		turnToDirectionUpFront(pActor, -pActor->mGravity, pRider->mCurrentDir);
	}

	bool isPlayerFrontSide (const LiveActor *pActor) {
        MtxPtr v0 = pActor->getBaseMtx();
        TVec3f v1(v0[0][2], v0[1][2], v0[2][2]);
        TVec3f v2(*MR::getPlayerPos());
        JMathInlineVEC::PSVECSubtract(v2, pActor->mTranslation, v2);
        return v2.dot(v1) >= 0.0f;
    }

	AnimScaleController* createSamboAnimScaleController(AnimScaleParam *pAnimScaleParam) {
		pAnimScaleParam->_10 = 4.0f;
		pAnimScaleParam->_14 = 1.0f;
		pAnimScaleParam->_18 = 0.06f;
		pAnimScaleParam->_1C = 2.0f;
		pAnimScaleParam->_20 = 0.06f;
		pAnimScaleParam->_24 = 0.4f;
		pAnimScaleParam->_28 = 0.8f;

		return new AnimScaleController(pAnimScaleParam);
	}
	
	//Loads an arc and a selected file into memory.
	void* loadArcAndFile(const char *pArc, const char *pFile, JKRHeap *pHeap) {
		OSReport("Loading file %s from %s\n", pFile, pArc);
		if (!pHeap) 
			pHeap = MR::getStationedHeapGDDR3();
		JKRArchive* arc = MR::mountArchive(pArc, pHeap, true);
		void* file = arc->getResource(pFile);

		if (arc && file) {
			OSReport("(PTD Archive Loader) Archive %s and file %s both exist!\n", pArc, pFile);
			return file;
		}
		else
			OSReport("(PTD Archive Loader) %s %s isn't exist!\n", pArc, pFile);

		return file;
	}

	void initShadowVolumeBox(LiveActor* pActor, const TVec3f& rPos) {
		pActor->initShadowControllerList(1);
		MR::addShadowVolumeBox(pActor, "ボリューム影(ボックス)", rPos, (MtxPtr)pActor->getBaseMtx());
	}

	LiveActor* getSpecificActorFromGroup(LiveActor* pActor, const char* pName) {
		LiveActor* pObj = 0;
    	LiveActorGroup* group = MR::getGroupFromArray(pActor);
		
    	for (s32 i = 0; i < group->mNumObjs; i++) {
    	    if (MR::isEqualString(group->getActor(i)->mName, pName)) {
    	        pObj = group->getActor(i);
    	        break;
    	    }
    	}
		return pObj;
	}

	NameObj *createObjByName(const char *pName) {
		CreateActorFunc *pCreator;
		pCreator = (CreateActorFunc *)NameObjFactory::getCreator(pName);
		if (!pCreator) {
			OSReport("(PTUtils createObjByName) Creator for %s isn't exist!\n", pName);
			return 0;
		}
		return pCreator(MR::getJapaneseObjectName(pName));
	}

	NameObj *createObjByNameAndInit(const char *pName, const JMapInfoIter &rIter) {
		NameObj *pObj = createObjByName(pName);
		if (pObj) {
			pObj->init(rIter);
			return pObj;
		}
		OSReport("(PTUtils createObjByNameAndInit) init for %s not called!\n", pName);
		return 0;
	}

	NameObj *createObjByNameAndInitWithoutIter(const char *pName) {
		NameObj *pObj = createObjByName(pName);
		if (pObj) {
			pObj->initWithoutIter();
			return pObj;
		}
		OSReport("(PTUtils createObjByNameAndInitWithoutIter) initWithoutIter for %s not called!\n", pName);
		return 0;
	}
}
