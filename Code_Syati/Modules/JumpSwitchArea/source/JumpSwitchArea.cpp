#include "JumpSwitchArea.h"

/*
* Authors: Someone, Aurum
* Objects: JumpSwitchArea
* Parameters:
*  - SW_A, needed: Activated when jumping
*  - SW_AWAKE
* 
* An area that activates its SW_A event whenever the player jumps.
*/
namespace pt {
    JumpSwitchArea::JumpSwitchArea(const char *pName) : AreaObj(pName) {
        mActivated = false;
    }

    void JumpSwitchArea::init(const JMapInfoIter &rIter) {
        AreaObj::init(rIter);
        MR::connectToSceneAreaObj(this);
    }

    void JumpSwitchArea::movement() {
        if (!mActivated && isInVolume(*MR::getPlayerPos()) && MR::isPlayerJumpRising() && !MR::isPlayerSwingAction()) {
            if (isOnSwitchA()) {
                offSwitchA();
            }
            else {
                onSwitchA();
            }

            mActivated = true;
        }

        if (MR::isOnGroundPlayer()) {
            mActivated = false;
        }
    }

    const char* JumpSwitchArea::getManagerName() const {
        return "JumpSwitchArea";
    }
}
