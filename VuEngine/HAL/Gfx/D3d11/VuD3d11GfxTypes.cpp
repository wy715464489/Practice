//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to D3d11.
// 
//*****************************************************************************

#include "VuD3d11GfxTypes.h"


//*****************************************************************************
D3D11_COMPARISON_FUNC VuD3d11GfxTypes::convert(VuGfxCompFunc compFunc)
{
	// compare function lookup table
	static D3D11_COMPARISON_FUNC sLookup[] =
	{
		D3D11_COMPARISON_NEVER,			// VUGFX_COMP_NEVER,
		D3D11_COMPARISON_LESS,			// VUGFX_COMP_LESS,
		D3D11_COMPARISON_EQUAL,			// VUGFX_COMP_EQUAL,
		D3D11_COMPARISON_LESS_EQUAL,	// VUGFX_COMP_LESSEQUAL,
		D3D11_COMPARISON_GREATER,		// VUGFX_COMP_GREATER,
		D3D11_COMPARISON_NOT_EQUAL,		// VUGFX_COMP_NOTEQUAL,
		D3D11_COMPARISON_GREATER_EQUAL,	// VUGFX_COMP_GREATEREQUAL,
		D3D11_COMPARISON_ALWAYS,		// VUGFX_COMP_ALWAYS,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_COMP_FUNC_COUNT);

	return sLookup[compFunc];
}

//*****************************************************************************
D3D11_BLEND VuD3d11GfxTypes::convert(VuGfxBlendMode blendMode)
{
	static D3D11_BLEND sLookup[] =
	{
		D3D11_BLEND_ZERO,			// VUGFX_BLEND_ZERO,
		D3D11_BLEND_ONE,			// VUGFX_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,		// VUGFX_BLEND_SRCCOLOR,
		D3D11_BLEND_INV_SRC_COLOR,	// VUGFX_BLEND_INVSRCCOLOR,
		D3D11_BLEND_SRC_ALPHA,		// VUGFX_BLEND_SRCALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,	// VUGFX_BLEND_INVSRCALPHA,
		D3D11_BLEND_DEST_ALPHA,		// VUGFX_BLEND_DESTALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,	// VUGFX_BLEND_INVDESTALPHA,
		D3D11_BLEND_DEST_COLOR,		// VUGFX_BLEND_DESTCOLOR,
		D3D11_BLEND_INV_DEST_COLOR,	// VUGFX_BLEND_INVDESTCOLOR,
		D3D11_BLEND_SRC_ALPHA_SAT,	// VUGFX_BLEND_SRCALPHASAT,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_BLEND_MODE_COUNT);

	return sLookup[blendMode];
}

//*****************************************************************************
D3D11_CULL_MODE VuD3d11GfxTypes::convert(VuGfxCullMode cullMode)
{
	static D3D11_CULL_MODE sLookup[] =
	{
		D3D11_CULL_NONE,	// VUGFX_CULL_NONE,
		D3D11_CULL_BACK,	// VUGFX_CULL_CW,
		D3D11_CULL_FRONT,	// VUGFX_CULL_CCW,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_CULL_MODE_COUNT);

	return sLookup[cullMode];
}

//*****************************************************************************
D3D11_TEXTURE_ADDRESS_MODE VuD3d11GfxTypes::convert(VuGfxTextureAddress textureAddress)
{
	static D3D11_TEXTURE_ADDRESS_MODE sLookup[] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,		// VUGFX_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,	// VUGFX_ADDRESS_CLAMP,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_TEXTURE_ADDRESS_COUNT);

	return sLookup[textureAddress];
}

//*****************************************************************************
D3D11_FILTER VuD3d11GfxTypes::convert(VuGfxTextureFilterType magFilter, VuGfxTextureFilterType minFilter, VuGfxTextureFilterType mipFilter)
{
	VUUINT32 filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	if ( magFilter >= VUGFX_TEXF_LINEAR ) filter |= D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	if ( minFilter >= VUGFX_TEXF_LINEAR ) filter |= D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	if ( mipFilter >= VUGFX_TEXF_LINEAR ) filter |= D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;

	return (D3D11_FILTER)filter;
}

//*****************************************************************************
D3D11_PRIMITIVE_TOPOLOGY VuD3d11GfxTypes::convert(VuGfxPrimitiveType primitiveType)
{
	// primitive type lookup table
	static D3D11_PRIMITIVE_TOPOLOGY sLookup[] =
	{
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,		// VUGFX_PT_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,		// VUGFX_PT_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,		// VUGFX_PT_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// VUGFX_PT_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// VUGFX_PT_TRIANGLESTRIP,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);

	return sLookup[primitiveType];
}

//*****************************************************************************
DXGI_FORMAT VuD3d11GfxTypes::convert(VuGfxFormat format)
{
	static DXGI_FORMAT sLookup[] =
	{
		DXGI_FORMAT_UNKNOWN,			// VUGFX_FORMAT_UNKNOWN

		DXGI_FORMAT_D24_UNORM_S8_UINT,	// VUGFX_FORMAT_D24S8

		DXGI_FORMAT_R8G8_SNORM,			// VUGFX_FORMAT_V8U8
		DXGI_FORMAT_R8G8_SNORM,			// VUGFX_FORMAT_LIN_V8U8
		DXGI_FORMAT_R16G16B16A16_SNORM,	// VUGFX_FORMAT_LIN_R16G16B16A16_SNORM

		DXGI_FORMAT_R8G8B8A8_UNORM,		// VUGFX_FORMAT_A8R8G8B8
		DXGI_FORMAT_R8_UNORM,			// VUGFX_FORMAT_R8
		DXGI_FORMAT_R8_UNORM,			// VUGFX_FORMAT_LIN_R8
		DXGI_FORMAT_R16_UNORM,			// VUGFX_FORMAT_R16
		DXGI_FORMAT_R16_UNORM,			// VUGFX_FORMAT_LIN_R16
		DXGI_FORMAT_UNKNOWN,			// VUGFX_FORMAT_L8A8
		DXGI_FORMAT_UNKNOWN,			// VUGFX_FORMAT_LIN_L8A8

		DXGI_FORMAT_R16_FLOAT,			// VUGFX_FORMAT_R16F
		DXGI_FORMAT_R32_FLOAT,			// VUGFX_FORMAT_R32F
		DXGI_FORMAT_R16G16_FLOAT,		// VUGFX_FORMAT_R16G16F
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup) / sizeof(sLookup[0]) == VUGFX_FORMAT_COUNT);

	return sLookup[format];
}

//*****************************************************************************
DXGI_FORMAT VuD3d11GfxTypes::convert(eGfxDeclType declType)
{
	static DXGI_FORMAT sLookup[] =
	{
		DXGI_FORMAT_R32_FLOAT,			// VUGFX_DECL_TYPE_FLOAT1,		
		DXGI_FORMAT_R32G32_FLOAT,		// VUGFX_DECL_TYPE_FLOAT2,		
		DXGI_FORMAT_R32G32B32_FLOAT,	// VUGFX_DECL_TYPE_FLOAT3,		
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// VUGFX_DECL_TYPE_FLOAT4,		
		DXGI_FORMAT_R8G8B8A8_SINT,		// VUGFX_DECL_TYPE_BYTE4,		
		DXGI_FORMAT_R8G8B8A8_SNORM,		// VUGFX_DECL_TYPE_BYTE4N,	
		DXGI_FORMAT_R8G8B8A8_UINT,		// VUGFX_DECL_TYPE_UBYTE4,		
		DXGI_FORMAT_R8G8B8A8_UNORM,		// VUGFX_DECL_TYPE_UBYTE4N,	
		DXGI_FORMAT_R16G16_SNORM,		// VUGFX_DECL_TYPE_SHORT2N,	
		DXGI_FORMAT_R16G16B16A16_SNORM,	// VUGFX_DECL_TYPE_SHORT4N,	
		DXGI_FORMAT_R16G16_FLOAT,		// VUGFX_DECL_TYPE_FLOAT16_2,	
		DXGI_FORMAT_R16G16B16A16_FLOAT,	// VUGFX_DECL_TYPE_FLOAT16_4,	
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_DECL_TYPE_COUNT);

	return sLookup[declType];
}

//*****************************************************************************
const char *VuD3d11GfxTypes::convert(eGfxDeclUsage declUsage)
{
	static const char *sLookup[] =
	{
		"POSITION",		// VUGFX_DECL_USAGE_POSITION,
		"NORMAL",		// VUGFX_DECL_USAGE_NORMAL,
		"COLOR",		// VUGFX_DECL_USAGE_COLOR,
		"TANGENT",		// VUGFX_DECL_USAGE_TANGENT,
		"BLENDWEIGHT",	// VUGFX_DECL_USAGE_BLENDWEIGHT,
		"BLENDINDICES",	// VUGFX_DECL_USAGE_BLENDINDICES,
		"TEXCOORD",		// VUGFX_DECL_USAGE_TEXCOORD,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_DECL_USAGE_COUNT);

	return sLookup[declUsage];
}
