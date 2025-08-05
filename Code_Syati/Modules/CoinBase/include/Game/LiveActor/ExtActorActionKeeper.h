#include "Game/LiveActor.h"

class RedCoin;
class BlueCoin;

class ExtActorActionKeeper : public ActorActionKeeper {
public:
    ExtActorActionKeeper(LiveActor *pActor, ActorAnimKeeper *pAnimKeeper, ActionFlagCtrl *pFlagCtrl, ActorPadAndCameraCtrl *pPadAndCameraCtrl,
        ActorHitReactionCtrl *pHitReactionCtrl, ActorItemGenerator *pItemGenerator, ActorParam *pActorParam, ActionScreenBlurCtrl *pScreenBlurCtrl,
        ActionFootPrintCtrl *pFootPrintCtrl, ActionBgmCtrl *pBgmCtrl);

    LiveActor* mNewActor;
    s32 mType;
};