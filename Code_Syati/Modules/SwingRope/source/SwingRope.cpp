#include "Game/Ride/SwingRope.h"

/*
* Author: Aurum
* Objects: SwingRope
* Parameters: <none>
* 
* This is the swinging vine from Beach Bowl Galaxy. This actor was left unused in SMG2's code, though. Its constructor
* is lost, but it can be easily restored. Other than that, all of its functionality is fully intact.
*/

SwingRope::SwingRope(const char *pName) : LiveActor(pName) {
	_90.zero();
	mClippingRadius = 0.0f;
	_A0 = false;
	_A4 = 0;
	_A8 = 0;
	_AC = 0;
	_B0 = NULL;
	_B4 = NULL;
	PSMTXIdentity(mRideMtx);
	_E8 = 0.0f;
	_EC = 0.0f;
	_F0 = 0.0f;
	_F4 = NULL;
	mActorCamera = NULL;
	_FC = 0.0f;
	_100 = 0.0f;
	_104 = 0.0f;
	_108.zero();
	mShadowDropPos.zero();
	_120 = 0.0f;
	_124 = 0;
}
