//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneShadowShader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuGfx.h"

class VuDropShadowShaderFlavor;

class VuDropShadowShader
{
public:
	VuDropShadowShader();
	~VuDropShadowShader();

	bool					init();
	void					release();

	VuShaderProgram			*getShaderProgram(bool bAnimated, bool bAlphaTest) const;

private:
	VuDropShadowShaderFlavor	*mpFlavors;
};
