#pragma once
#include "syati.h"
#include "Game/AreaObj/AreaObjContainer.h"

{{IncludeList}}

static ManagerEntry cModuleCreateAreaObjMgrTable[] = {
    {NULL, NULL, NULL},
{{ManagerEntryList}}
};

const s32 cModuleCreateAreaObjMgrTableCount = sizeof(cModuleCreateAreaObjMgrTable) / sizeof(ManagerEntry);
