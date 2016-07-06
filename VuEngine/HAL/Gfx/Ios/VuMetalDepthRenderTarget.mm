//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal DepthRenderTarget interface class.
//
//*****************************************************************************

#include "VuMetalDepthRenderTarget.h"


//*****************************************************************************
VuMetalDepthRenderTarget::VuMetalDepthRenderTarget(int width, int height):
	VuDepthRenderTarget(width, height)
{
}

//*****************************************************************************
VuMetalDepthRenderTarget::~VuMetalDepthRenderTarget()
{
}

//*****************************************************************************
VuTexture *VuMetalDepthRenderTarget::getTexture()
{
	VUASSERT(0, "Not implemented!");
	
	return VUNULL;
}

//*****************************************************************************
VuMetalDepthRenderTarget *VuMetalDepthRenderTarget::create(int width, int height)
{
	VUASSERT(0, "Not implemented!");

	return VUNULL;
}
