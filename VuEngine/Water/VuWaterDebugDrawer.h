//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  WaterDebugDrawer class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"

class Vu3dDrawComponent;
class VuGfxDrawParams;
class VuDbrtNode;


class VuWaterDebugDrawer : public VuEntity
{
public:
	VuWaterDebugDrawer();
	~VuWaterDebugDrawer();

private:
	void		draw3d(const VuGfxDrawParams &params);
	void		draw2d();

	// components
	Vu3dDrawComponent	*mp3dDrawComponent;

	bool				mbDraw3d;
	bool				mbDraw2d;
};
