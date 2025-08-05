#include "Game/Screen/CounterLayoutController.h"

class BlueCoinCounter;
class BlueCoinStageProgress;
class CounterLayoutControllerExt : public CounterLayoutController {
public:
    CounterLayoutControllerExt();
    s32* mGlobalLayoutHolder;
    BlueCoinCounter* mBlueCoinCounter;
    BlueCoinStageProgress* mBlueCoinStageProgress;
};