//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 DepthRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuDepthRenderTarget.h"

class VuD3d11Texture;


class VuD3d11DepthRenderTarget : public VuDepthRenderTarget
{
public:
	VuD3d11DepthRenderTarget(int width, int height);
	~VuD3d11DepthRenderTarget();

	virtual VuTexture		*getTexture();

	static VuD3d11DepthRenderTarget *create(int width, int height);

	VuD3d11Texture			*mpDepthTexture;
	VuD3d11Texture			*mpColorTexture;
	ID3D11RenderTargetView	*mpD3d11ColorView;
	ID3D11DepthStencilView	*mpD3d11DepthStencilView;
};
