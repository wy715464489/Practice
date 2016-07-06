//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to D3d11.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuGfxTypes.h"


namespace VuD3d11GfxTypes
{

	D3D11_COMPARISON_FUNC		convert(VuGfxCompFunc compFunc);
	D3D11_BLEND					convert(VuGfxBlendMode blendMode);
	D3D11_CULL_MODE				convert(VuGfxCullMode cullMode);
	D3D11_TEXTURE_ADDRESS_MODE	convert(VuGfxTextureAddress textureAddress);
	D3D11_FILTER				convert(VuGfxTextureFilterType magFilter, VuGfxTextureFilterType minFilter, VuGfxTextureFilterType mipFilter);
	D3D11_PRIMITIVE_TOPOLOGY 	convert(VuGfxPrimitiveType primitiveType);
	int							vertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount);
	DXGI_FORMAT					convert(VuGfxFormat format);
	DXGI_FORMAT					convert(eGfxDeclType declType);
	const char					*convert(eGfxDeclUsage declUsage);

} // namespace VuD3d11GfxTypes
