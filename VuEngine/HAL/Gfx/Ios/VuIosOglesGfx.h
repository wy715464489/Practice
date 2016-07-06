//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Ios Gfx Ogles HAL.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/Ogles/VuOglesGfx.h"


class VuIosOglesGfx : public VuOglesGfx
{
public:
	// render targets
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params);
};
