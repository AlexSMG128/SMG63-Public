#include "Game/Screen/PauseMenu.h"

class BlueCoinList;

class PauseMenuExt : public PauseMenu {
public: 
    PauseMenuExt();
    virtual ~PauseMenuExt();

    #ifdef BLUECOINSYSTEM
    void exeBlueCoinList();
    #endif
    
    // BCS
    BlueCoinList* mBlueCoinList;
    s32 mDisplayMode;
    bool mIsInvalidBack;
    bool mIsExistBlueCoins;
    
    // PMNB
    ButtonPaneController* mButtonNew;
    TVec2f mButtonNewFollowPos;
    bool mIsUsedNewButton;

};

namespace NrvPauseMenuExt {
    NERVE(NrvPauseMenuExtBlueCoinList);
};