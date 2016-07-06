//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 FxRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuFxRenderTarget.h"

class VuD3d11Texture;


class VuD3d11FxRenderTarget : public VuFxRenderTarget
{
public:
	VuD3d11FxRenderTarget(int width, int height, VuGfxFormat format);
	~VuD3d11FxRenderTarget();

	virtual VuTexture		*getTexture();

	static VuD3d11FxRenderTarget *create(int width, int height, VuGfxFormat format);

	VuD3d11Texture			*mpColorTexture;
	ID3D11RenderTargetView	*mpD3d11ColorView;
};
