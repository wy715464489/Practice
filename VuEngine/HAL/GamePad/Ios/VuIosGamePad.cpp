//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Ios Game Pad
//
//*****************************************************************************

#include "VuIosGamePadObjC.h"
#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/Managers/VuTickManager.h"


class VuIosGamePad : public VuGamePad
{
public:
	VuIosGamePad();
	~VuIosGamePad();

	virtual bool init();
	virtual void release();

	// cross-platform functionality
	virtual  VuController	&getController(int index) { return mpControllers[index]; }
	
	// platform-specific functionality
	static VuIosGamePad *IF() { return static_cast<VuIosGamePad *>(VuGamePad::IF());}
	
	class VuIosController;
	VuIosController	&getIosController(int index) { return mpControllers[index]; }

	void			tick(float fdt);

	class VuIosController : public VuController
	{
	public:
		VuIosController() : mPausedHandlerFired(false) {}
		virtual void	playVibrationEffect(int effect) {}
		bool			mPausedHandlerFired;
	};

	VuIosController			*mpControllers;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuIosGamePad);


//*****************************************************************************
VuIosGamePad::VuIosGamePad()
{
	addAxis("LEFT_STICK_X", -1.0f, 1.0f);
	addAxis("LEFT_STICK_Y", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_X", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_Y", -1.0f, 1.0f);
	addAxis("LEFT_TRIGGER", 0.0f, 1.0f);
	addAxis("RIGHT_TRIGGER", 0.0f, 1.0f);

	VUASSERT(mAxisDefs.size() == VuIosGamePadObjC::AXIS_COUNT, "VuIosGamePad::VuIosGamePad() axis count mismatch");

	addButton("A");
	addButton("B");
	addButton("X");
	addButton("Y");
	addButton("LEFT_SHOULDER");
	addButton("RIGHT_SHOULDER");
	addButton("DPAD_UP");
	addButton("DPAD_DOWN");
	addButton("DPAD_LEFT");
	addButton("DPAD_RIGHT");
	addButton("PAUSE");

	addButton("SIMPLE_A");
	addButton("SIMPLE_B");
	addButton("SIMPLE_X");
	addButton("SIMPLE_Y");
	addButton("SIMPLE_LEFT_SHOULDER");
	addButton("SIMPLE_RIGHT_SHOULDER");
	addButton("SIMPLE_DPAD_UP");
	addButton("SIMPLE_DPAD_DOWN");
	addButton("SIMPLE_DPAD_LEFT");
	addButton("SIMPLE_DPAD_RIGHT");

	VUASSERT(mButtonDefs.size() == VuIosGamePadObjC::BUTTON_COUNT, "VuIosGamePad::VuIosGamePad() button count mismatch");

	mpControllers = new VuIosController[MAX_NUM_PADS];
}

//*****************************************************************************
VuIosGamePad::~VuIosGamePad()
{
	delete[] mpControllers;
}

//*****************************************************************************
bool VuIosGamePad::init()
{
	if ( !VuGamePad::init() )
		return false;

	VuTickManager::IF()->registerHandler(this, &VuIosGamePad::tick, "Input");

	return true;
}

//*****************************************************************************
void VuIosGamePad::release()
{
	VuTickManager::IF()->unregisterHandlers(this);

	VuGamePad::release();
}

//*****************************************************************************
void VuIosGamePad::tick(float fdt)
{
	VuIosGamePadObjC::update(VuGamePad::MAX_NUM_PADS);
	
	// handle pause button
	for ( int i = 0; i < VuGamePad::MAX_NUM_PADS; i++ )
	{
		VuIosController &controller = VuIosGamePad::IF()->getIosController(i);
		
		if ( controller.mPausedHandlerFired )
			controller.mButtons |= 1<<VuIosGamePadObjC::BUTTON_PAUSE;
		else
			controller.mButtons &= ~(1<<VuIosGamePadObjC::BUTTON_PAUSE);
		
		controller.mPausedHandlerFired = false;
	}
}

//*****************************************************************************
bool VuIosGamePadObjC::getAvailableIndex(int &index)
{
	for ( int i = 0; i < VuGamePad::MAX_NUM_PADS; i++ )
	{
		if ( !VuGamePad::IF()->getController(i).mIsConnected )
		{
			index = i;
			return true;
		}
	}
	
	return false;		
}

//*****************************************************************************
void VuIosGamePadObjC::setConnectedState(int index, bool isConnected)
{
	VuGamePad::IF()->getController(index).mIsConnected = isConnected;
}

//*****************************************************************************
void VuIosGamePadObjC::setExtendedState(int index, float lsx, float lsy, float rsx, float rsy, float lt, float rt, unsigned int buttons)
{
	VuGamePad::VuController &controller = VuGamePad::IF()->getController(index);
	
	controller.mDeviceType = VuGamePad::DEVICE_GAMEPAD;
	
	controller.mAxes[AXIS_LEFT_STICK_X] = lsx;
	controller.mAxes[AXIS_LEFT_STICK_Y] = lsy;
	controller.mAxes[AXIS_RIGHT_STICK_X] = rsx;
	controller.mAxes[AXIS_RIGHT_STICK_Y] = rsy;
	controller.mAxes[AXIS_LEFT_TRIGGER] = lt;
	controller.mAxes[AXIS_RIGHT_TRIGGER] = rt;
	
	controller.mButtons = buttons;
}

//*****************************************************************************
void VuIosGamePadObjC::setSimpleState(int index, unsigned int buttons)
{
	VuGamePad::VuController &controller = VuGamePad::IF()->getController(index);
	
	controller.mDeviceType = VuGamePad::DEVICE_SIMPLE_GAMEPAD;
	
	controller.mButtons = buttons;
}

//*****************************************************************************
void VuIosGamePadObjC::onPauseHandlerFired(int index)
{
	VuIosGamePad::VuIosController &controller = VuIosGamePad::IF()->getIosController(index);
	
	controller.mPausedHandlerFired = true;
}