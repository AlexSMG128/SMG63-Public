#pragma once

#include "ActorCreation.h"

{{IncludeList}}


const CreateActorEntry cModuleCreateNameObjTable[] = {
	//DUMMY - This is never read
	{ "1234567890", NULL },
{{NameObjFactoryList}}
};

const s32 cModuleCreateNameObjTableCount = sizeof(cModuleCreateNameObjTable) / sizeof(CreateActorEntry);