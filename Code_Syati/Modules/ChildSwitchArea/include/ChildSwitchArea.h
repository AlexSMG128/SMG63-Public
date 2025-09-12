#pragma once

#include "syati.h"
#include "Game/AreaObj/AreaObj.h"
#include "Game/LiveActor/LiveActorGroup.h"
#include "revolution/os.h"

namespace pt
{
    class ChildSwitchArea : public AreaObj
    {
    public:
        ChildSwitchArea(const char *);

        virtual void init(const JMapInfoIter &rIter);
        virtual void movement();
        virtual const char *getManagerName() const;

        bool mActive;
        LiveActorGroup *pGroup;
        int mChildAmount;
    };
};
