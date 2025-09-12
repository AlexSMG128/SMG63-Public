#pragma once

#include "ActorCreation.h"

{{IncludeList}}


const CreateActorEntry cNewCreateMapObjTable[] = {
	//DUMMY - This is never read
	{ "1234567890", NULL },
{{ProductMapObjList}}
};

const s32 cNewCreateMapObjTableCount = sizeof(cNewCreateMapObjTable) / sizeof(CreateActorEntry);