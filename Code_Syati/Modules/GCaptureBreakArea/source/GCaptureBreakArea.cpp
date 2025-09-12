#include "Game/MapObj/GCapture.h"
#include "GCaptureBreakArea.h"
GCapture *currentGC;

void storeCurrentGCapture (GCapture *pGC, HitSensor *pCaptureTarget) {
    currentGC = pGC;
    pGC->requestBind(pCaptureTarget);
}
kmCall(0x802A5B68, storeCurrentGCapture); // requestBind__8GCaptureFP9HitSensor

GCaptureBreakArea::GCaptureBreakArea (const char *pName) : AreaObj(pName) {}
GCaptureBreakArea::~GCaptureBreakArea () {}
void GCaptureBreakArea::init (const JMapInfoIter &rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}
void GCaptureBreakArea::movement () {
    if (MR::isPlayerInAreaObj("GCaptureBreakArea") && currentGC) 
        currentGC->exeBreak();
}
