//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the texture interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuD3d11GfxTypes.h"


class VuD3d11Texture : public VuTexture
{
public:
	VuD3d11Texture(int width, int height, int levelCount, const VuTextureState &state);
	~VuD3d11Texture();

	virtual void				setData(int level, const void *pData, int size);
	virtual bool				reload(VuBinaryDataReader &reader, int skipLevels);

	static VuD3d11Texture		*load(VuBinaryDataReader &reader, int skipLevels);
	static VuD3d11Texture		*create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state);
	
	DXGI_FORMAT					mD3dFormat;
	ID3D11Texture2D				*mpD3dTexture;
	ID3D11ShaderResourceView	*mpD3dResourceView;
	ID3D11SamplerState			*mpD3dSamplerState;
};
