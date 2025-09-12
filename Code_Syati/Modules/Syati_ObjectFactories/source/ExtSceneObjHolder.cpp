#include "revolution.h"
#include "kamek/hooks.h"
#include "Game/NameObj/NameObj.h"
#include "Game/Scene/SceneObjHolder.h"
#include "ModuleData_SceneObjHolder.h"

namespace {
	NameObj* createSceneObj(SceneObjHolder* pHolder, s32 type) {
		NameObj* sceneObj = pHolder->newEachObj(type);

		if (!sceneObj) {
			for (s32 i = 0; i < cNewCreateSceneObjTableCount; i++) {
				const CreateSceneObjEntry e = cNewCreateSceneObjTable[i];

				if (e.mSlotId == type) {
					sceneObj = e.mCreationFunc();
					break;
				}
			}
		}

		return sceneObj;
	}

	kmCall(0x804599D0, createSceneObj); // redirection hook
}