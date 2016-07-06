//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DebugCamera class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Math/VuVector2.h"


class VuDebugCamera : public VuCamera
{
public:
	VuDebugCamera();

	void	tick(float fdt, int padIndex);
	void	frame(const VuAabb &aabb);

	void operator = (const VuCamera &other);

private:
	VuVector3	mLinControl;
	VuVector2	mRotControl;
};