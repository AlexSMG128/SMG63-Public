#pragma once

#include "Game/MapObj/InvisiblePolygonObj.h"

// 0 to 13 correspond to player modes
#define TRANSPARENT_WALL_FLAG_EXTRA_START 14
#define TRANSPARENT_WALL_CHECK_PLAYER_SKATING (TRANSPARENT_WALL_FLAG_EXTRA_START + 0)
#define TRANSPARENT_WALL_CHECK_PLAYER_TERESA_DISAPPEAR (TRANSPARENT_WALL_FLAG_EXTRA_START + 1)

namespace pt {
    class TransparentWall : public InvisiblePolygonObj {
    public:
        TransparentWall(const char *pName);

        virtual void init(const JMapInfoIter &rIter);
        virtual void control();

        inline bool checkAllowed(s32 index) const {
            return mAllowFlags & (1 << index);
        }

        s32 mAllowFlags;
        bool mValidated;
    };
};
