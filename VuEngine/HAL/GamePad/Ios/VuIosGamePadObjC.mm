//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Objective-C support for Ios Game Pad.
//
//*****************************************************************************

#include "VuIosGamePadObjC.h"
#import <GameController/GCController.h>


//*****************************************************************************
void VuIosGamePadObjC::update(int maxPadCount)
{
	unsigned int connectedMask = 0;
	
	for ( GCController *controller in [GCController controllers])
	{
		// not connected yet?
		int index = controller.playerIndex;
		if ( index == GCControllerPlayerIndexUnset )
		{
			if ( getAvailableIndex(index) )
			{
				controller.playerIndex = index;
				VUPRINTF("Controller connected to slot %d\n", index);
			}
		}
		
		// pause handler
		if ( controller.controllerPausedHandler == nil )
		{
			controller.controllerPausedHandler = ^(GCController *controller)
			{
				if ( controller.playerIndex >= 0 && controller.playerIndex < maxPadCount )
				{
					onPauseHandlerFired(controller.playerIndex);
				}
			};
		}
		
		if ( index >= 0 && index < maxPadCount)
		{
			connectedMask |= (1<<index);
			
			if ( GCExtendedGamepad *extendedGamePad = controller.extendedGamepad )
			{
				float lsx = extendedGamePad.leftThumbstick.xAxis.value;
				float lsy = extendedGamePad.leftThumbstick.yAxis.value;
				float rsx = extendedGamePad.rightThumbstick.xAxis.value;
				float rsy = extendedGamePad.rightThumbstick.yAxis.value;
				float lt = extendedGamePad.leftTrigger.value;
				float rt = extendedGamePad.rightTrigger.value;
				
				unsigned int buttons = 0;
				buttons |= int(extendedGamePad.buttonA.pressed) << BUTTON_A;
				buttons |= int(extendedGamePad.buttonB.pressed) << BUTTON_B;
				buttons |= int(extendedGamePad.buttonX.pressed) << BUTTON_X;
				buttons |= int(extendedGamePad.buttonY.pressed) << BUTTON_Y;
				buttons |= int(extendedGamePad.leftShoulder.pressed) << BUTTON_LEFT_SHOULDER;
				buttons |= int(extendedGamePad.rightShoulder.pressed) << BUTTON_RIGHT_SHOULDER;
				buttons |= int(extendedGamePad.dpad.up.pressed) << BUTTON_DPAD_UP;
				buttons |= int(extendedGamePad.dpad.down.pressed) << BUTTON_DPAD_DOWN;
				buttons |= int(extendedGamePad.dpad.left.pressed) << BUTTON_DPAD_LEFT;
				buttons |= int(extendedGamePad.dpad.right.pressed) << BUTTON_DPAD_RIGHT;
				
				setExtendedState(index, lsx, lsy, rsx, rsy, lt, rt, buttons);
			}
			else if ( GCGamepad *gamePad = controller.gamepad )
			{
				unsigned int buttons = 0;
				buttons |= int(gamePad.buttonA.pressed) << BUTTON_SIMPLE_A;
				buttons |= int(gamePad.buttonB.pressed) << BUTTON_SIMPLE_B;
				buttons |= int(gamePad.buttonX.pressed) << BUTTON_SIMPLE_X;
				buttons |= int(gamePad.buttonY.pressed) << BUTTON_SIMPLE_Y;
				buttons |= int(gamePad.leftShoulder.pressed) << BUTTON_SIMPLE_LEFT_SHOULDER;
				buttons |= int(gamePad.rightShoulder.pressed) << BUTTON_SIMPLE_RIGHT_SHOULDER;
				buttons |= int(gamePad.dpad.up.pressed) << BUTTON_SIMPLE_DPAD_UP;
				buttons |= int(gamePad.dpad.down.pressed) << BUTTON_SIMPLE_DPAD_DOWN;
				buttons |= int(gamePad.dpad.left.pressed) << BUTTON_SIMPLE_DPAD_LEFT;
				buttons |= int(gamePad.dpad.right.pressed) << BUTTON_SIMPLE_DPAD_RIGHT;
				
				setSimpleState(index, buttons);
			}
		}
	}

	// update connected state
	for ( int i = 0; i < maxPadCount; i++ )
		setConnectedState(i, connectedMask & (1<<i) ? true : false);
}
