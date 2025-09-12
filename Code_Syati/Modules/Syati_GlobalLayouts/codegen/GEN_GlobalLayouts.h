#pragma once
#include "revolution/types.h"
#include "GlobalLayouts.h"

{{IncludeList}}

const CreateLayoutEntry cModuleCreateLayoutTable[] = {
	//DUMMY - This is never read
	{ "", "", NULL},
{{GlobalLayoutList}}
};

const s32 cModuleCreateLayoutTableCount = sizeof(cModuleCreateLayoutTable) / sizeof(CreateLayoutEntry);