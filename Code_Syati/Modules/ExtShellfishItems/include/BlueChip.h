#pragma once

#include "syati.h"

#define SCENE_OBJ_BLUE_CHIP_HOLDER 110

namespace pt {
	class BlueChip : public ChipBase {
	public:
		BlueChip(const char *pName);
	};

	class BlueChipGroup : public ChipGroup {
	public:
		BlueChipGroup(const char *pName);
	};

	NameObj* createBlueChipHolder();
	ChipHolder* getBlueChipHolder();
};
