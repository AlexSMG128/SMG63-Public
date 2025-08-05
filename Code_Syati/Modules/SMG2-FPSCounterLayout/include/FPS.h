#pragma once

#include "syati.h"
#include "revolution/vi.h"

#define VI_50FPS (VI_PAL || VI_MPAL)

class FPSCounter : public LayoutActor
{
public:
    FPSCounter(const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();


    virtual void appear();
    virtual void kill();
};