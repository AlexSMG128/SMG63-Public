#pragma once 

#include "syati.h"

namespace pt {
    class ScrewSwitchBase : public LiveActor {
    public:
        ScrewSwitchBase(const char* pName);

        virtual void init(const JMapInfoIter& rIter);
        virtual void initAfterPlacement();
        virtual void calcAndSetBaseMtx();
        virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
        virtual void initScrew(const JMapInfoIter& rIter) = 0;
        virtual void exeWait() = 0;
        virtual void exeAdjust() = 0;
        virtual void exeScrew() = 0;
        virtual void exeEnd();

        void updateBindActorMtx();

        LiveActor* mBindedActor;
        MapObjConnector* mMapObjConnector;
    };

    namespace NrvScrewSwitchBase {
        NERVE(NrvWait);
        NERVE(NrvAdjust);
        NERVE(NrvScrew);
        NERVE(NrvEnd);
    };

    class ScrewSwitch : public ScrewSwitchBase {
    public:
        ScrewSwitch(const char* pName);

        virtual void initScrew(const JMapInfoIter& rIter);
        virtual void control();
        virtual void exeWait();
        virtual void exeAdjust();
        virtual void exeScrew();

        CollisionParts* mScrewCollision;
    };

    class ScrewSwitchReverse : public ScrewSwitchBase {
    public:
        ScrewSwitchReverse(const char* pName);

        virtual void initScrew(const JMapInfoIter& rIter);
        virtual void exeWait();
        virtual void exeAdjust();
        virtual void exeScrew();

        void endBind();

        bool mForceJump;
    };

    class ValveSwitch : public ScrewSwitchBase {
    public:
        ValveSwitch(const char* pName);

        virtual void initScrew(const JMapInfoIter& rIter);
        virtual void control();
        virtual void exeWait();
        virtual void exeAdjust();
        virtual void exeScrew();

        TMtx34f mCollisionMtx;
        bool mIsReverse;
    };
};