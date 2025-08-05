#include "PaintingEntranceObj.h"

PaintingEntranceObj::PaintingEntranceObj(const char* pName) : LiveActor(pName)
{
	mEntranceCollision = NULL;
	mCameraInfo = NULL;
	mCooldown = 0;
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

	mEntranceCollision = MR::createCollisionPartsFromLiveActor(this, "BackWall", getSensor("Main"), MR::CollisionScaleType_AutoEqualScale);

	MR::invalidateCollisionParts(mCollisionParts);
	MR::invalidateCollisionParts(mEntranceCollision);

	MR::initActorCamera(this, rIter, &mCameraInfo);

	initNerve(&NrvPaintingEntranceObj::NrvWait::sInstance, 0);
}

bool PaintingEntranceObj::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (msg == ACTMES_END_DEMO)
		mCooldown = 60;

	if (!isNerve(&NrvPaintingEntranceObj::NrvWait::sInstance))
		return false;

	if (pSender != MR::getPlayerBodySensor() || MR::isMsgSpinStormRange(msg))
		return false;

	if (pReceiver != getSensor("Main"))
		return false;

	if (!MR::isDemoActive() && mCooldown <= 0)
		setNerve(&NrvPaintingEntranceObj::NrvEnter::sInstance);

	return false;
}

namespace NrvPaintingEntranceObj {
	void NrvWait::execute(Spine* pSpine) const {
		PaintingEntranceObj* pActor = (PaintingEntranceObj*)pSpine->mExecutor;

		MR::startBtkAndSetFrameAndStop(pActor, "WarpRing", MR::getBtkFrameMax(pActor));

		if (pActor->mCooldown > 0)
		{
			pActor->mCooldown--;
			return;
		}

		if (MR::isDemoActive())
		{
			if (MR::isNearPlayer(pActor, 500.f))
			{
				//OSReport("Cutscene Begin\n");
				pSpine->setNerve(&NrvPaintingEntranceObj::NrvExit::sInstance);
			}
			MR::invalidateCollisionParts(pActor->mCollisionParts);
			MR::invalidateCollisionParts(pActor->mEntranceCollision);
		}
		else if (MR::isGreaterStep(pActor, 30))
		{
			MR::validateCollisionParts(pActor->mCollisionParts);
			MR::validateCollisionParts(pActor->mEntranceCollision);
		}
	}
	NrvWait(NrvWait::sInstance);

	void NrvEnter::execute(Spine* pSpine) const {
		// Play animation & Camera & Warp
		PaintingEntranceObj* pActor = (PaintingEntranceObj*)pSpine->mExecutor;
		if (MR::isFirstStep(pActor))
		{
			MR::offPlayerControl();
			MR::startBtk(pActor, "WarpRing");
			MR::startActorCameraTargetSelf(pActor, pActor->mCameraInfo, -1);
			MR::deactivateDefaultGameLayout();
			MR::startActionSound(pActor, "OnEnter", -1, -1, -1);
		}
		if (MR::isGreaterStep(pActor, 30))
			MR::hidePlayer();
		if (MR::isBtkOneTimeAndStopped(pActor) && MR::isValidSwitchA(pActor))
			MR::onSwitchA(pActor);
	}
	NrvEnter(NrvEnter::sInstance);

	void NrvExit::execute(Spine* pSpine) const {
		// Just play the animation, and return to wait when the cutscene ends
		PaintingEntranceObj* pActor = (PaintingEntranceObj*)pSpine->mExecutor;

		if (MR::isLessStep(pActor, 60))
			return;

		if (MR::isStep(pActor, 60))
		{
			MR::startBtk(pActor, "WarpRing");
			MR::startActionSound(pActor, "OnExit", -1, -1, -1);
		}

		if (MR::isBtkOneTimeAndStopped(pActor) && !MR::isDemoActive())
			pSpine->setNerve(&NrvPaintingEntranceObj::NrvWait::sInstance); //Quirky direct setNerve
	}
	NrvExit(NrvExit::sInstance);
}