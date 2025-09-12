#include "kamek/hooks.h"

#include "ModuleData_{{ClassName}}_Ext.h"

kmWrite16({{HookAddr}} + 0x02, sizeof({{ClassName}}_Ext)); // New Size Allocation