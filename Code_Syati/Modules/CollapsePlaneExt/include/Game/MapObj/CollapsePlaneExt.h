#include "Game/MapObj/CollapsePlane.h"

class CollapsePlaneExt : public CollapsePlane {
public:
    CollapsePlaneExt(const char* pName);
    void exeReturn();
    s32 mRespawnTime; // 0xE4 NEW
};

namespace NewNrvCollapsePlane {
    NERVE(NrvReturn);
};