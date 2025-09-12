#include "PaintingEntranceObj.h"
#include "GalaxyLevelEngine.h"
PaintingEntranceObj::PaintingEntranceObj(const char* pName) : LiveActor(pName)
{
	mCameraInfo = NULL;
	mTargetDest = 0;
	mZoneID = 0;
}

void PaintingEntranceObj::init(const JMapInfoIter& rIter)
{
	MR::processInitFunction(this, rIter, false);

	f32 ColFrame = 0.0f, TexFrame = 0.0f;

	MR::getJMapInfoArg0NoInit(rIter, &ColFrame);
	if (MR::isExistBrk(this, "ColorChange"))
		MR::startBrkAndSetFrameAndStop(this, "ColorChange", MR::max(0.0, TexFrame));

	MR::getJMapInfoArg1NoInit(rIter, &TexFrame);
	if (MR::isExistBtp(this, "TexChange"))
		MR::startBtpAndSetFrameAndStop(this, "TexChange", MR::max(0.0, TexFrame));

	if (!MR::isExistStageSwitchAwake(rIter))
		makeActorAppeared();
	
	MR::getJMapInfoArg2NoInit(rIter, &mTargetDest);

	mZoneID = MR::getPlacedZoneId(rIter);

	MR::initActorCamera(this, rIter, &mCameraInfo);
	
	if (MR::isValidSwitchA(this))
		MR::onSwitchA(this);

	initNerve(&NrvPaintingEntranceObj::NrvWait::sInstance, 0);
}

void PaintingEntranceObj::initAfterPlacement() {
	if (MR::isOnSwitchB(this))
		setNerve(&NrvPaintingEntranceObj::NrvExit::sInstance);
}

void PaintingEntranceObj::exeWait() {
	MR::startBtkAndSetFrameAndStop(this, "WarpRing", MR::getBtkFrameMax(this));

	if (MR::isOnSwitchB(this)) {
		setNerve(&NrvPaintingEntranceObj::NrvEnter::sInstance);
	}
}

void PaintingEntranceObj::exeEnter() {
	MR::offDrawEffect(MarioAccess::getPlayerActor());
	if (MR::isFirstStep(this)) {
		MR::hidePlayer();
		DemoStartRequestUtil::startDemoSystem(this, "Enter", 0, DemoStartInfo::DEMOTYPE_0, DemoStartInfo::CINEMAFRAMETYPE_0, DemoStartInfo::STARPOINTERTYPE_0, DemoStartInfo::DELETEEFFECTYPE_0, 0);
		MR::offPlayerControl();
		MR::disableStarPointerShootStarPiece();
		MR::startBtk(this, "WarpRing");
		MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
		MR::deactivateDefaultGameLayout();
		MR::startActionSound(this, "OnEnter", -1, -1, -1);

		if (MR::isValidSwitchA(this))
			MR::offSwitchA(this);
	}

	if (MR::isStep(this, 75))
		MR::closeSystemWipeWhiteFade(60);

	if (MR::isStep(this, 145)) {
		JMapInfo* pInfo = GLE::getChangeSceneListInfoFromZone(mZoneID);
		GLE::requestMoveStageFromJMapInfo(pInfo, mTargetDest);
	}
}

void PaintingEntranceObj::exeExit() {
	if (MR::isLessStep(this, 60))
		return;

	if (MR::isStep(this, 60)) {
		MR::startBtk(this, "WarpRing");
		MR::startActionSound(this, "OnExit", -1, -1, -1);
	}

	if (MR::isBtkOneTimeAndStopped(this) && !MR::isDemoActive())
		setNerve(&NrvPaintingEntranceObj::NrvWait::sInstance); //Quirky direct setNerve
}

namespace NrvPaintingEntranceObj {
	void NrvWait::execute(Spine* pSpine) const {
		((PaintingEntranceObj*)pSpine->mExecutor)->exeWait();

	}
	NrvWait(NrvWait::sInstance);

	void NrvEnter::execute(Spine* pSpine) const {
		// Play animation & Camera & Warp
			((PaintingEntranceObj*)pSpine->mExecutor)->exeEnter();
	}
	NrvEnter(NrvEnter::sInstance);

	void NrvExit::execute(Spine* pSpine) const {
		// Just play the animation, and return to wait when the cutscene ends
		((PaintingEntranceObj*)pSpine->mExecutor)->exeExit();
	}
	NrvExit(NrvExit::sInstance);
}