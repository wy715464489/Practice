//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"


class VuD3d11DepthStencilState : public VuDepthStencilState
{
public:
	VuD3d11DepthStencilState(const VuDepthStencilStateParams &params);
	~VuD3d11DepthStencilState();

	static VuD3d11DepthStencilState	*create(const VuDepthStencilStateParams &params);

	VUUINT32				mHash;
	ID3D11DepthStencilState	*mpD3dDepthStencilState;
};
