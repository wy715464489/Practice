//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 ShadowRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"
#include "VuEngine/Containers/VuArray.h"

class VuD3d11Texture;


class VuD3d11ShadowRenderTarget : public VuShadowRenderTarget
{
public:
	VuD3d11ShadowRenderTarget(int width, int height, int count);
	~VuD3d11ShadowRenderTarget();

	virtual VuTexture		*getColorTexture(int layer);

	static VuD3d11ShadowRenderTarget *create(int width, int height, int count);

	struct ColorBuffer
	{
		VuD3d11Texture			*mpTexture;
		ID3D11RenderTargetView	*mpD3d11View;
	};
	typedef VuArray<ColorBuffer> ColorBuffers;

	VuD3d11Texture			*mpDepthTexture;
	ID3D11DepthStencilView	*mpD3d11DepthStencilView;
	ColorBuffers			mColorBuffers;
};
