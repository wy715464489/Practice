//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stub interface class for GamePad.
//
//*****************************************************************************

#include "VuEngine/HAL/GamePad/VuGamePad.h"


class VuStubGamePad : public VuGamePad
{
public:
	virtual VuController	&getController(int index) { return mController; }

private:
	class VuStubController : public VuController
	{
		virtual void	playVibrationEffect(int effect) {};
	};

	VuStubController		mController;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuStubGamePad);
