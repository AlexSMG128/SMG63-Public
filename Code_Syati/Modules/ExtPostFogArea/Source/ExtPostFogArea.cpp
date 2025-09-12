#include "syati.h"
#include "Game/AreaObj/PostFogArea.h"

/*
 * Author: Gabbo
 * Patch: ExtPostFogArea
 * Description:
 * A patch that adds a new functionality to the PostFogArea.
 * If Obj_arg7 is given a number, the fog changes opacity depending on how many stars the player has collected. 
 * The fog starts at 127 alpha with 0 stars, and it gradually decreases for each star collected, until it reaches the value in Obj_arg7 with zero alpha.
 * This module is intended for HubWorlds.
 * 
 * - Obj_arg7: the max number of stars (Obj_arg2 gets ignored if this arg is used).
*/

const s32 ExtPostFogArea(PostFogArea* FogArea) {
    if (FogArea->mObjArg7 > -1) {
        f32 starNum = MR::getPowerStarNum();
        if (starNum > FogArea->mObjArg7)
            starNum = FogArea->mObjArg7;
        return (127.0f - ((starNum / FogArea->mObjArg7) * 127.0f));               
    }
    if (FogArea->mObjArg2 <= -1) {
        return 255;
    } 
    return FogArea->mObjArg2;    
}

kmBranch(0x80075750, ExtPostFogArea);
