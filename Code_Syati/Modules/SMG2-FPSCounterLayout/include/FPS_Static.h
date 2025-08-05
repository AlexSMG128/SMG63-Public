#pragma once

#include "revolution/types.h"
#include "Game/Screen/LayoutActor.h"

namespace MR
{
    s32 lastTick;
    s32 currentTick;
    f32 CurrentFPS;
    //0 = Default
    //1 = Precise
    //2 = Rounded
    u8 RoundMode;

    void CalcFPS();

    //this should be in Syati...
    void setAnimFrameAndStop(LayoutActor*, f32, u32);
}