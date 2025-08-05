#include "syati.h"
#include "Game/LiveActor/ExtActorActionKeeper.h"

ExtActorActionKeeper::ExtActorActionKeeper(LiveActor *pActor, ActorAnimKeeper *pAnimKeeper, ActionFlagCtrl *pFlagCtrl, ActorPadAndCameraCtrl *pPadAndCameraCtrl,
    ActorHitReactionCtrl *pHitReactionCtrl, ActorItemGenerator *pItemGenerator, ActorParam *pActorParam, ActionScreenBlurCtrl *pScreenBlurCtrl,
    ActionFootPrintCtrl *pFootPrintCtrl, ActionBgmCtrl *pBgmCtrl) : 
    ActorActionKeeper(pActor, pAnimKeeper, pFlagCtrl, pPadAndCameraCtrl, pHitReactionCtrl, pItemGenerator, pActorParam, pScreenBlurCtrl, pFootPrintCtrl, pBgmCtrl) {
    
    mNewActor = 0;
    mType = 0;
}

ExtActorActionKeeper* createExtActorActionKeeper(s32 l, LiveActor *pActor, ActorAnimKeeper *pAnimKeeper, ActionFlagCtrl *pFlagCtrl, ActorPadAndCameraCtrl *pPadAndCameraCtrl,
    ActorHitReactionCtrl *pHitReactionCtrl, ActorItemGenerator *pItemGenerator, ActorParam *pActorParam, ActionScreenBlurCtrl *pScreenBlurCtrl,
    ActionFootPrintCtrl *pFootPrintCtrl, ActionBgmCtrl *pBgmCtrl) {

    return new ExtActorActionKeeper(pActor, pAnimKeeper, pFlagCtrl, pPadAndCameraCtrl, pHitReactionCtrl, pItemGenerator, pActorParam, pScreenBlurCtrl, pFootPrintCtrl, pBgmCtrl);
}

kmWrite32(0x8022A9AC, 0x48000010); // b 0x10
kmCall(0x8022A9E4, createExtActorActionKeeper); // bl createExtActorActionKeeper