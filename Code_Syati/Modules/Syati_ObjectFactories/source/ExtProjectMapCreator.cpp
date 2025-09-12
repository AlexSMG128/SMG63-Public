#include "revolution.h"
#include "kamek/hooks.h"
#include "Game/Util/StringUtil.h"
#include "Game/Map/ProductMapCreator.h"
#include "ModuleData_ProjectMapCreator.h"

namespace {
	void* getMapObjCreator(ProductMapCreator* pFactory, const char* pName) {
		CreateActorFunc* creator = pFactory->getProductMapCreator(pName);

		if (!creator) {
			for (s32 i = 1; i < cNewCreateMapObjTableCount; i++) {
				const CreateActorEntry entry = cNewCreateMapObjTable[i];

				if (MR::isEqualString(entry.pActorName, pName)) {
					creator = entry.mCreationFunc;
					break;
				}
			}
		}

		return creator;
	}

	kmCall(0x8026305C, getMapObjCreator); // redirection hook
}