#include "BossKameckFollowerGroup.h"

#include "Game/Enemy/KameckBeamHolder.h"
#include "Game/Util/ActorSensorUtil.h"
#include "Game/Util/ActorShadowUtil.h"
#include "Game/Util/ChildObjUtil.h"
#include "Game/Util/JMapUtil.h"
#include "Game/Util/LiveActorUtil.h"
#include "Game/Util/ObjUtil.h"
#include "Game/Util/SceneUtil.h"
#include "Game/Util/StringUtil.h"

BossKameckFollowerGroup::BossKameckFollowerGroup(const char* pName) : NameObj(pName)
{
	mId = -1;
}

void BossKameckFollowerGroup::init(const JMapInfoIter& rIter)
{
	MR::getJMapInfoArg0WithInit(rIter, &mId);
	mHolder = (DeriveActorGroup<Kameck>*)MR::initChildObjGroup(this, "BossKameckFollowerGroup", rIter);
	mHolder->makeAllDead();
}

s32 BossKameckFollowerGroup::getLivingActorNum() const {
	if (mHolder != NULL)
		return mHolder->getLivingActorNum();
	return 0;
}



void BossKameckFollowerGroup::startDemoAppear()
{
	if (mHolder == NULL)
		return;

	for (s32 i = 0; i < mHolder->mNumObjs; i++)
	{
		Kameck* mKameck = reinterpret_cast<Kameck*>(mHolder->getActor(i));
		if (MR::isDead(mKameck))
		{
			OSReport("Kameck Demo Appear\n");
			mKameck->startDemoAppear(); // Game freaks out when this is called for some reason... I have no idea why, so the stuff below is to fix it... hopefully
			MR::requestMovementOn(mKameck);
			MR::onCalcAnim(mKameck);
			MR::showModel(mKameck);
			MR::onCalcShadow(mKameck, NULL);
			MR::onCalcGravity(mKameck);
		}
	}
}
void BossKameckFollowerGroup::endDemoAppear()
{
	if (mHolder == NULL)
		return;

	for (s32 i = 0; i < mHolder->mNumObjs; i++)
	{
		Kameck* mKameck = reinterpret_cast<Kameck*>(mHolder->getActor(i));
		// Kameck::endDemoAppear
		{
			OSReport("Kameck Demo Appear End ");
			if (mKameck->isNerve(&NrvKameck::KameckNrvDemoAppear::sInstance))
			{
				mKameck->setNerve(&NrvKameck::KameckNrvWait::sInstance);
				OSReport("Success\n");
			}
			else
				OSReport("Failure\n");
		}
	}
}
void BossKameckFollowerGroup::deadForceAll()
{
	if (mHolder == NULL)
		return;

	for (s32 i = 0; i < mHolder->mNumObjs; i++)
	{
		Kameck* mKameck = reinterpret_cast<Kameck*>(mHolder->getActor(i));
		// Kameck::makeActorDeadForce
		{
			mKameck->resetBeam();
			mKameck->mBeamHolder->killAll();
			mKameck->makeActorDead();
		}
	}
}
void BossKameckFollowerGroup::reviveAll() {
	if (mHolder == NULL)
		return;

	for (s32 i = 0; i < mHolder->mNumObjs; i++)
	{
		Kameck* mKameck = reinterpret_cast<Kameck*>(mHolder->getActor(i));
		if (MR::isDead(mKameck))
			mKameck->appear();
	}
}