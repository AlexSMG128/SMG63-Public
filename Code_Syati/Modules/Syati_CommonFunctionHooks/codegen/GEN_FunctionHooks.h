#pragma once

{{IncludeListOnGameSceneStart}}
{{IncludeListAnySceneDestroyed}}
{{IncludeListGameSystemFrameLoop}}
{{IncludeListGameSceneUpdate}}

typedef void (*HookFuncDef)();

const HookFuncDef cModuleGameSceneStartFuncTable[] = {
    NULL,
{{GameSceneStartFuncList}}
};
const s32 cModuleGameSceneStartFuncTableCount = sizeof(cModuleGameSceneStartFuncTable) / sizeof(HookFuncDef);


const HookFuncDef cModuleAnySceneDestroyedFuncTable[] = {
    NULL,
{{AnySceneDestroyedFuncList}}
};
const s32 cModuleAnySceneDestroyedFuncTableCount = sizeof(cModuleAnySceneDestroyedFuncTable) / sizeof(HookFuncDef);


const HookFuncDef cModuleGameSystemFrameLoopTable[] = {
    NULL,
{{GameSystemFrameLoopList}}
};
const s32 cModuleGameSystemFrameLoopTableCount = sizeof(cModuleGameSystemFrameLoopTable) / sizeof(HookFuncDef);

const HookFuncDef cModuleGameSceneUpdateTable[] = {
    NULL,
{{GameSceneUpdateList}}
};
const s32 cModuleGameSceneUpdateTableCount = sizeof(cModuleGameSceneUpdateTable) / sizeof(HookFuncDef);