//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  InputManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuJsonContainer;


class VuInputManager : VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuInputManager)

protected:

	// called by engine
	friend class VuEngine;
	virtual bool	init() = 0;
	virtual void	release() = 0;

public:
	enum eMapping { M_INVALID, M_AXIS_POS, M_AXIS_NEG, M_BUTTON, M_KEY };
	enum eConfig { CONFIG_GAMEPAD, CONFIG_KEYBOARD, CONFIG_COUNT };
	enum { MAX_CHANNELS = 8 };

	virtual float	getAxisValue(int padIndex, const char *name) = 0;
	virtual float	getRawAxisValue(int padIndex, const char *name) = 0;
	virtual bool	getButtonValue(int padIndex, const char *name) = 0;
	virtual bool	getButtonWasPressed(int padIndex, const char *name) = 0;
	virtual bool	getButtonWasReleased(int padIndex, const char *name) = 0;

	virtual void	setDefaultMapping(int padIndex, eConfig config) = 0;

	virtual void	setOnScreenButton(int padIndex, const char *name) = 0;
	virtual void	setOnScreenAxis(int padIndex, const char *name, float direction) = 0;

	virtual void	setConfig(eConfig config) = 0;
};
