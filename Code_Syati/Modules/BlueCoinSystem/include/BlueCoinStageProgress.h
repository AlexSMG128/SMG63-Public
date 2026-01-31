#pragma once

#include "syati.h"
#include "BlueCoinUtil.h"

class BlueCoinStageProgress : public LayoutActor {
public:
    BlueCoinStageProgress(const char* pName);
    virtual ~BlueCoinStageProgress();
    virtual void init(const JMapInfoIter& rIter); 
    virtual void appear();
    virtual void kill();
    virtual void control();
    void exeAppear();
    void exeWait();
    void exeUpdate();
    void exeDisappear(); 

    void disappear();
    void updateTextBox();
    void updateAnimBox();
    void updateNumBox();
    void setTargetIDAndAppear(s32 id);
    void calcSlidePaneFrame();
    void moveCounterForScoreAttack();

    TVec2f mFollowPos;
    CounterLayoutAppearer* mAppearer;
    CountUpPaneRumbler* mPaneRumbler;
    s32 mTargetID;
    f32 mSlideFrame;
    bool mIsTargetIDGot;
};

namespace NrvBlueCoinStageProgress {
    NERVE_DECL_EXE(NrvAppear, BlueCoinStageProgress, Appear);
    NERVE_DECL_EXE(NrvWait, BlueCoinStageProgress, Wait);
    NERVE_DECL_EXE(NrvUpdate, BlueCoinStageProgress, Update);
    NERVE_DECL_EXE(NrvDisappear, BlueCoinStageProgress, Disappear);
    NERVE(NrvHide);
};