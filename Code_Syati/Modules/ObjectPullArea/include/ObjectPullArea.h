#pragma once

#include "syati.h"

class ObjectPullArea : public AreaObj {
public:
    ObjectPullArea(const char *pName);
    ~ObjectPullArea();
    virtual void init(const JMapInfoIter &rIter);
    virtual void movement();
    virtual const char *getManagerName() const;

    bool mNoSpinning;
    s32 mCheckTypes[7];
};

// obj_arg0: Require spinning?
// obj_arg1: Hit Sensor Type to check
// obj_arg2: Hit Sensor Type to check
// obj_arg3: Hit Sensor Type to check
// obj_arg4: Hit Sensor Type to check
// obj_arg5: Hit Sensor Type to check
// obj_arg6: Hit Sensor Type to check
// obj_arg7: Hit Sensor Type to check

namespace ObjectPullConfig {
    static bool isPullItem(MarioActor *pMarioActor, u32 sensorType);
    void clear();
    static bool gIsEnabled;
	static bool gSpinRequired;
    static s32 gCheckTypes[7];
};