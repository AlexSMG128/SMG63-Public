#pragma once

#include "syati.h"

namespace pt {
    class WatchTowerRotateStep : public LiveActor {
    public:
        WatchTowerRotateStep(const char *pName);

        virtual void init(const JMapInfoIter &rIter);
        virtual void calcAndSetBaseMtx();

        void initLift(const JMapInfoIter &rIter);
        void attachLift();

        void exeMove();
        void exeMoveStart();

        TVec3f mRotateDeg;
        PartsModel* mParts[4];
        f32 mRotSpeed;
    };

    namespace NrvWatchTowerRotateStep {
        NERVE(NrvMove);
        NERVE(NrvMoveStart);
        NERVE(NrvWait);
    };
};
