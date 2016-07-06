//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneShadowShader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuGfx.h"

class VuShadowShaderFlavor;

class VuShadowShader
{
public:
	VuShadowShader();
	~VuShadowShader();

	bool					init();
	void					release();

	VuShaderProgram			*getShaderProgram(bool bAnimated, bool bAlphaTest) const;

private:
	VuShadowShaderFlavor	*mpFlavors;
};
