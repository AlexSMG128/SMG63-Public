#include "SMG63Area.h"

SMG63Area::SMG63Area(const char* pName) : AreaObj(pName) {
    mIsLeftArea = false;
}

void SMG63Area::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
	MR::connectToSceneAreaObj(this);
}

void SMG63Area::movement() {
    if (!mIsLeftArea || mObjArg0 > 0) {
        if (isInVolume(*MR::getPlayerPos())) {
            if (mObjArg0 < 1) {
                MR::deactivateDefaultGameLayout();

                if (mObjArg0 == 0)
                    MR::tryScreenToFrameCinemaFrame();
            }
            else if (mObjArg0 == 1) {
                MR::disableStarPointerShootStarPiece();
            }
            else if (mObjArg0 == 2) {
                MarioAccess::getPlayerActor()->_F9C = false;
                MarioAccess::getPlayerActor()->_F9A = 1;
            }

            mIsLeftArea = true;
        }
    }

    if (mIsLeftArea) {
        if (!isInVolume(*MR::getPlayerPos())) {
            if (mObjArg0 != 1) {
                MR::activateDefaultGameLayout();

                if (mObjArg0 == 0)
                    MR::tryFrameToScreenCinemaFrame();
            }
            else
                MR::enableStarPointerShootStarPiece();

            mIsLeftArea = false;
        }
    }
}

const char* SMG63Area::getManagerName() const {
    return "SMG63Area";
}