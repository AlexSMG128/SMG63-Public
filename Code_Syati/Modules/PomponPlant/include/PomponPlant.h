#pragma once 

#include "syati.h"

namespace pt {
    class PomponPlant : public LiveActor {
    public:
        PomponPlant(const char *pName);

        virtual void init(const JMapInfoIter &rIter);
        virtual void control();
        virtual void attackSensor(HitSensor *pSender, HitSensor *pReceiver);
        virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor *pSender, HitSensor *pReceiver);

        bool tryGenItem();
        void exeSwingDpd();
        void exeSwingPlayer();

        s32 mCollectible;
        s32 mAmount;
        bool mItemGenerated;
    };

    namespace NrvPomponPlant {
        NERVE(NrvWait);
        NERVE(NrvSwingDpd);
        NERVE(NrvSwingPlayer);
    };
};
