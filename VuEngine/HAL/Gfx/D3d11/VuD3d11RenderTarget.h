//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 D3d11 RenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuRenderTarget.h"

class VuD3d11Texture;


class VuD3d11RenderTarget : public VuRenderTarget
{
public:
	VuD3d11RenderTarget(int width, int height);
	~VuD3d11RenderTarget();

	virtual VuTexture		*getColorTexture();
	virtual void			readPixels(VuArray<VUBYTE> &rgb);

	static VuD3d11RenderTarget *create(int width, int height);

	VuD3d11Texture			*mpDepthTexture;
	VuD3d11Texture			*mpColorTexture;
	ID3D11DepthStencilView	*mpD3d11DepthStencilView;
	ID3D11RenderTargetView	*mpD3d11ColorView;
};
