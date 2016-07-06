//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stub accelerometer hardware abstration layer
//
//*****************************************************************************

#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/Math/VuVector3.h"


class VuStubAccel : public VuAccel
{
public:
	virtual bool	getAccel(VuVector3 &accel) { accel = VuVector3(0,0,0); return true; }
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAccel, VuStubAccel);
