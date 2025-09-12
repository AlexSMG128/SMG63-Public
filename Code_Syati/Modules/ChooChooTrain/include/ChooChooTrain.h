#pragma once

#include "syati.h"

namespace pt {
    class ChooChooTrainParts;

    class ChooChooTrain : public LiveActor {
    public:
        ChooChooTrain(const char *pName);

        virtual void init(const JMapInfoIter &rIter);
        virtual void startClipped();
        virtual void endClipped();
        virtual void control();

        void accel();
        void deccel();
        void setWheelRunRate(f32 rate);
        void playMoveLevelSound(f32 rate);

        void exeAccel();
        void exeMove();
        void exeDecel();

        MR::Vector<MR::AssignableArray<ChooChooTrainParts*> > mParts;
        TVec3f mRailClipping;
        f32 mSpeed;
        f32 mMaxSpeed;
        s32 mWhistleTimer;
        s32 mAccelTime;
    };

    class ChooChooTrainParts : public ModelObj {
    public:
        ChooChooTrainParts(ChooChooTrain *pHost, const char *pModelName);

        ChooChooTrain* mHost;
    };

    namespace NrvChooChooTrain {
        NERVE(NrvWait);
        NERVE(NrvAccel);
        NERVE(NrvMove);
        NERVE(NrvDecel);
    };
};
