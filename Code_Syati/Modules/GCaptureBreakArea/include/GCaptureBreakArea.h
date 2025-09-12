#pragma once

#include "syati.h"

class GCaptureBreakArea : public AreaObj
{
public:
    GCaptureBreakArea(const char *pName);
    virtual ~GCaptureBreakArea();
    virtual void init(const JMapInfoIter &rIter);
    virtual void movement();
};