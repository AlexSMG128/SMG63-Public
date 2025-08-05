#include "syati.h"

class BlueCoinCounter : public LayoutActor {
public:
        BlueCoinCounter(const char* pName);
        
        virtual ~BlueCoinCounter();
        virtual void init(const JMapInfoIter& rIter);
        virtual void appear();
        virtual void control();
        void forceAppear();
        void updateCounter();
        void disappear();
        void setCounter();
        void exeHide();
        void exeAppear();
        void exeWait();
        void exeDisappear();
        void exeShowTextBox();

        CounterLayoutAppearer* mAppearer;
        CountUpPaneRumbler* mPaneRumbler;
        SysInfoWindow* mSysInfoWindow;
        s32 mBlueCoinCount;
        s32 mBlueCoinDisplayNum;
        s32 _3C;
        bool mIsAppear;
};

namespace NrvBlueCoinCounter {
	NERVE(NrvHide);
        NERVE(NrvAppear);
        NERVE(NrvWait);
        NERVE(NrvDisappear);
        NERVE(NrvShowTextBox);
};
