#include "Game/Screen/GameSceneLayoutHolder.h"
#include "GlobalLayouts.h"
#include "ModuleData_GlobalLayouts.h"

namespace {
	GlobalLayoutHolder::GlobalLayoutHolder()
	{
		mLayouts = new LayoutActor * [cModuleCreateLayoutTableCount]; //yes I know we're wasting a perfectly good 4 bytes. Shoot me
		
		for (s32 i = 1; i < cModuleCreateLayoutTableCount; i++) {
			const CreateLayoutEntry entry = cModuleCreateLayoutTable[i];

			if (!MR::isFileExist(entry.pLayoutPath, true))
			{
				mLayouts[i] = NULL;
				continue;
			}

			mLayouts[i] = entry.mCreationFunc(entry.pActorName);
			mLayouts[i]->initWithoutIter();
		}
	}

	void GlobalLayoutHolder::movement()
	{
		for (s32 i = 1; i < cModuleCreateLayoutTableCount; i++) {
			LayoutActor* pActorCurrent = mLayouts[i];
			if (pActorCurrent == NULL)
				continue;
			pActorCurrent->movement();
		}
	}

	void GlobalLayoutHolder::draw()
	{
		for (s32 i = 1; i < cModuleCreateLayoutTableCount; i++) {
			LayoutActor* pActorCurrent = mLayouts[i];
			if (pActorCurrent == NULL)
				continue;
			pActorCurrent->draw();
		}
	}

	void GlobalLayoutHolder::calcAnim()
	{
		for (s32 i = 1; i < cModuleCreateLayoutTableCount; i++) {
			LayoutActor* pActorCurrent = mLayouts[i];
			if (pActorCurrent == NULL)
				continue;
			pActorCurrent->calcAnim();
		}
	}

	LayoutActor* GlobalLayoutHolder::getLayout(const char* pName)
	{
		if (pName != NULL)
			for (s32 i = 1; i < cModuleCreateLayoutTableCount; i++) {
				LayoutActor* pActorCurrent = mLayouts[i];
				if (pActorCurrent == NULL)
					continue;
				if (MR::isEqualString(pActorCurrent->mName, pName))
					return pActorCurrent;
			}
		return NULL;
	}

	//-----------------------------------------------------------------

	//Edit CounterLayoutController
	kmWrite32(0x80471780, 0x38600050); //li r3, 0x4C -> li r3, 0x50

	void createGlobalLayouts(register CounterLayoutController* pLayout)
	{
		MR::connectToSceneLayout(pLayout);

		register GlobalLayoutHolder* glh = new GlobalLayoutHolder();
		__asm
		{
			stw glh, 0x4C(pLayout)
		}
	}
	kmCall(0x804657A0, createGlobalLayouts); //Add branch to create code

	CounterLayoutController* getCounterLayoutController()
	{
		register GameSceneLayoutHolder* holder = MR::getGameSceneLayoutHolder();
		if (holder == NULL)
			return NULL;
		register CounterLayoutController* controller = NULL;
		__asm
		{
			lwz controller, 0x34(holder)
		}
		return controller;
	}

	GlobalLayoutHolder* tryGetGlobalLayoutHolder()
	{
		register CounterLayoutController* pCounter = getCounterLayoutController();
		if (pCounter == NULL)
			return NULL;
		register GlobalLayoutHolder* pGlobal;
		__asm
		{
			lwz pGlobal, 0x4C(pCounter)
		}
		return pGlobal;
	}

	void doMovementGlobalLayouts(NerveExecutor* nrv)
	{
		nrv->updateNerve();

		GlobalLayoutHolder* pGlobal = tryGetGlobalLayoutHolder();
		if (pGlobal == NULL)
			return;
		pGlobal->movement();
	}

	void doDrawGlobalLayouts()
	{
		GlobalLayoutHolder* pGlobal = tryGetGlobalLayoutHolder();
		if (pGlobal == NULL)
			return;
		pGlobal->draw();
	}

	void doCalcAnimGlobalLayouts()
	{
		GlobalLayoutHolder* pGlobal = tryGetGlobalLayoutHolder();
		if (pGlobal == NULL)
			return;
		pGlobal->calcAnim();

		MR::isStageWorldMap();
	}

	kmCall(0x804518A8, doMovementGlobalLayouts);
	kmBranch(0x804589B8, doDrawGlobalLayouts);
	kmCall(0x80451948, doCalcAnimGlobalLayouts);
}