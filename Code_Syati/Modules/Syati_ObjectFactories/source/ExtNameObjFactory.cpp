#include "revolution.h"
#include "kamek/hooks.h"
#include "Game/Util/StringUtil.h"
#include "ModuleData_NameObjFactory.h"

namespace {
    void* getNameObjCreator(const char* pName) {
        void* creator = NameObjFactory::getCreator(pName);

        if (!creator) {
            for (s32 i = 1; i < cModuleCreateNameObjTableCount; i++) {
                const CreateActorEntry entry = cModuleCreateNameObjTable[i];

                if (MR::isEqualString(entry.pActorName, pName)) {
                    creator = entry.mCreationFunc;
                    break;
                }
            }
        }

        return creator;
    }

    kmCall(0x803394C0, getNameObjCreator); // redirection hook
    kmCall(0x803430C8, getNameObjCreator);
    kmCall(0x804564D4, getNameObjCreator);
}
