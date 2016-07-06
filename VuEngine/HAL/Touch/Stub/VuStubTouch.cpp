//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stub touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuEngine/HAL/Touch/VuTouch.h"


class VuStubTouch : public VuTouch
{
public:
	virtual int		getTouchCount(VUUINT32 priority) { return 0; }
	virtual void	getTouchRaw(int index, VuVector2 &touch) { }
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuStubTouch);
