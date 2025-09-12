#include "ExtGameDataUtil.h"
#include "ModuleData_ExtGameData.h"

namespace ExtGameDataUtil {
    ExtGameDataHolder* getCurrentGameDataHolder() {
        return (ExtGameDataHolder *)GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder;
    }
}