//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the cube texture interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuTexture.h"


class VuD3d11CubeTexture : public VuCubeTexture
{
public:
	VuD3d11CubeTexture(int edgeLength, int levelCount, const VuTextureState &state);
	~VuD3d11CubeTexture();

	virtual bool				reload(VuBinaryDataReader &reader, int skipLevels);

	static VuD3d11CubeTexture	*load(VuBinaryDataReader &reader, int skipLevels);

	ID3D11Texture2D				*mpD3dCubeTexture;
	ID3D11ShaderResourceView	*mpD3dResourceView;
	ID3D11SamplerState			*mpD3dSamplerState;
};
