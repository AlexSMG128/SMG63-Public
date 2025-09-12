#include "kamek/hooks.h"

#include "ModuleData_{{ClassName}}_Ext.h"

#if defined(TWN) || defined(KOR)
kmWrite16({{HookAddrMinor}} + 0x02, sizeof({{ClassName}}_Ext)); // New Size Allocation
#else
kmWrite16({{HookAddrMajor}} + 0x02, sizeof({{ClassName}}_Ext)); // New Size Allocation
#endif