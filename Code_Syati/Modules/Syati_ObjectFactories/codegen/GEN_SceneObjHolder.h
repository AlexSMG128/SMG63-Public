#pragma once

#include "ActorCreation.h"

{{IncludeList}}


const CreateSceneObjEntry cNewCreateSceneObjTable[] = {
	//DUMMY - This is never read
	{ 1234567890, NULL },
{{SceneObjList}}
};

const s32 cNewCreateSceneObjTableCount = sizeof(cNewCreateSceneObjTable) / sizeof(CreateSceneObjEntry);