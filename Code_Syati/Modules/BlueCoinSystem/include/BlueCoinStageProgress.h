#pragma once

#include "syati.h"
#include "BlueCoinUtil.h"

class MultiEmitter {
    public:
    void setOffset(const TVec3f& rVec);
};

class BlueCoinStageProgress : public LayoutActor {
public:
    BlueCoinStageProgress(const char* pName);
    virtual ~BlueCoinStageProgress();
    virtual void init(const JMapInfoIter& rIter); 
    virtual void appear();
    virtual void control();
    void exeAppear();
    void exeWait();
    void exeUpdate();
    void exeDisappear(); 

    void disappear();
    void updateTextBox();
    void updateAnimBox();
    void setTargetIDAndAppear(s32 id);

    s32 mTargetID;
    CounterLayoutAppearer* mAppearer;
    CountUpPaneRumbler* mPaneRumbler;
};

namespace NrvBlueCoinStageProgress {
    NERVE(NrvAppear);
    NERVE(NrvWait);
    NERVE(NrvUpdate);
    NERVE(NrvDisappear);
    NERVE(NrvHide);
};