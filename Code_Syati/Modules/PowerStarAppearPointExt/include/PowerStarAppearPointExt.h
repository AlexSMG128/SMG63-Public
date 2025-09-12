#pragma once

#include "syati.h"

class ActorCameraInfo;

namespace pt {
class PowerStarAppearPointExt : public LiveActor {
    public:
        PowerStarAppearPointExt(const char* pName);
        virtual void init(const JMapInfoIter& rIter);
        virtual void movement();
    
        ActorCameraInfo* mCamInfo;
        bool mSpawnAtPlayer;
        f32 mYOffset;
    };
};