#include "MathUtil.h"

namespace pt {
	void clampS32(s32 min, s32 max, s32* val) {
		if (*val < min)
			*val = min;
		else if (*val > max)
			*val = max;
	}
};
