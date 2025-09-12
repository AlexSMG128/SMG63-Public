#pragma once

#include "syati.h"
#include "Game/NameObj/NameObj.h"

namespace pt {
	template<s32 COLOR>
    NameObj* createSuperSpinDriverCustomColor(const char *pName) {
        return new SuperSpinDriver(pName, COLOR);
    }
};
