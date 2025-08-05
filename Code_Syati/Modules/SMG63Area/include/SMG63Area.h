#pragma once

#include "syati.h"
#include "Game/Screen/GameSceneLayoutHolder.h"

class SMG63Area : public AreaObj {
public:
    SMG63Area(const char* pName);
    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual const char* getManagerName() const;

    bool mIsLeftArea;
};