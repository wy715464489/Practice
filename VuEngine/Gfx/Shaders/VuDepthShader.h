//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneDepthShader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuGfx.h"

class VuDepthShaderFlavor;
class VuShaderProgram;
class VuMatrix;


class VuDepthShader
{
public:
	VuDepthShader();
	~VuDepthShader();

	bool				init();
	void				release();

	VuShaderProgram		*getShaderProgram(bool bAnimated, bool bAlphaTest) const;

private:
	VuDepthShaderFlavor	*mpFlavors;
};
