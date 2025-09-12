#pragma once

#include "syati.h"

namespace pt {
    class CrystalSwitch : public LiveActor {
    public:
        CrystalSwitch(const char *pName);

        virtual void init(const JMapInfoIter &rIter);
        virtual void control();
        virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
        virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

        void exeOff();
        void exeSwitchDown();
        void exeOn();
        void exeSwitchUp();
        void calcRotSpeed();

        s32 mActiveTime;
        f32 mRotSpeed;
        u8 mStartSpin;
    };

    namespace NrvCrystalSwitch {
        NERVE(NrvOff);
        NERVE(NrvSwitchDown);
        NERVE(NrvOn);
        NERVE(NrvSwitchUp);
    };
};
