#include "Game/MapObj/CollapsePlaneExt.h"

CollapsePlaneExt::CollapsePlaneExt(const char* pName) : CollapsePlane(pName) {};

namespace pt {

    CollapsePlaneExt* createCollapsePlaneExt(const char* pName) {
        return new CollapsePlaneExt(pName);
    }

    kmCall(0x8033EF04, createCollapsePlaneExt);
    kmWrite32(0x8033EF08, 0x48000014);


	void CollapsePlaneInit(CollapsePlaneExt* pActor, const JMapInfoIter& rIter, MapObjActorInitInfo& rInfo) {
		pActor->mRespawnTime = -1;
		MR::getJMapInfoArg1NoInit(rIter, &pActor->mRespawnTime);
		MR::useStageSwitchReadA(pActor, rIter);

        pActor->initialize(rIter, rInfo);
	}

	kmCall(0x8029016C, CollapsePlaneInit);

    void CollapsePlaneDecideNerve(CollapsePlaneExt* pActor) {
        if (pActor->mRespawnTime > -1)
            pActor->setNerve(&NewNrvCollapsePlane::NrvReturn::sInstance);
        else
            pActor->setNerve(&NrvCollapsePlane::CollapsePlaneNrvEnd::sInstance);
    }

    kmCall(0x802903D8, CollapsePlaneDecideNerve);
}

void CollapsePlaneExt::exeReturn() {
	if (MR::isStep(this, mRespawnTime)) {
        MR::showMaterial(this, "PlaneMat_v");
        MR::validateCollisionParts(this);
        setNerve(&NrvCollapsePlane::CollapsePlaneNrvWait::sInstance);
    }
};

namespace NewNrvCollapsePlane {
	void NrvReturn::execute(Spine* pSpine) const {
        ((CollapsePlaneExt*)pSpine->mExecutor)->exeReturn();
    };

	NrvReturn(NrvReturn::sInstance);
}