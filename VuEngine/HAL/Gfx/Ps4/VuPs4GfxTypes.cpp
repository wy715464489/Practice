//*****************************************************************************
//
//  Copyright (c) 2011-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to Ps4.
// 
//*****************************************************************************

#include "VuPs4GfxTypes.h"

using namespace sce;


//*****************************************************************************
Gnm::DataFormat VuPs4GfxTypes::convert(VuGfxFormat format)
{
	static Gnm::DataFormat sLookup[] =
	{
		Gnm::kDataFormatInvalid,		// VUGFX_FORMAT_UNKNOWN

		Gnm::kDataFormatInvalid,		// VUGFX_FORMAT_D24S8

		Gnm::kDataFormatR8G8Snorm,			// VUGFX_FORMAT_V8U8
		Gnm::kDataFormatR8G8Snorm,			// VUGFX_FORMAT_LIN_V8U8
		Gnm::kDataFormatR16G16B16A16Snorm,	// VUGFX_FORMAT_LIN_R16G16B16A16_SNORM

		Gnm::kDataFormatR8G8B8A8Unorm,	// VUGFX_FORMAT_A8R8G8B8
		Gnm::kDataFormatR8Unorm,		// VUGFX_FORMAT_R8
		Gnm::kDataFormatR8Unorm,		// VUGFX_FORMAT_LIN_R8
		Gnm::kDataFormatR16Unorm,		// VUGFX_FORMAT_R16
		Gnm::kDataFormatR16Unorm,		// VUGFX_FORMAT_LIN_R16
		Gnm::kDataFormatL8A8Unorm,		// VUGFX_FORMAT_L8A8
		Gnm::kDataFormatL8A8Unorm,		// VUGFX_FORMAT_LIN_L8A8

		Gnm::kDataFormatR16Float,		// VUGFX_FORMAT_R16F
		Gnm::kDataFormatR32Float,		// VUGFX_FORMAT_R32F
		Gnm::kDataFormatR16G16Float,	// VUGFX_FORMAT_R16G16F
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup) / sizeof(sLookup[0]) == VUGFX_FORMAT_COUNT);

	return sLookup[format];
}

//*****************************************************************************
Gnm::CompareFunc VuPs4GfxTypes::convert(VuGfxCompFunc compFunc)
{
	static Gnm::CompareFunc sLookup[] =
	{
		Gnm::kCompareFuncNever,			// VUGFX_COMP_NEVER,
		Gnm::kCompareFuncLess,			// VUGFX_COMP_LESS,
		Gnm::kCompareFuncEqual,			// VUGFX_COMP_EQUAL,
		Gnm::kCompareFuncLessEqual,		// VUGFX_COMP_LESSEQUAL,
		Gnm::kCompareFuncGreater,		// VUGFX_COMP_GREATER,
		Gnm::kCompareFuncNotEqual,		// VUGFX_COMP_NOTEQUAL,
		Gnm::kCompareFuncGreaterEqual,	// VUGFX_COMP_GREATEREQUAL,
		Gnm::kCompareFuncAlways,		// VUGFX_COMP_ALWAYS,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_COMP_FUNC_COUNT);

	return sLookup[compFunc];
}

//*****************************************************************************
Gnm::BlendMultiplier VuPs4GfxTypes::convert(VuGfxBlendMode blendMode)
{
	static Gnm::BlendMultiplier sLookup[] =
	{
		Gnm::kBlendMultiplierZero,				// VUGFX_BLEND_ZERO,
		Gnm::kBlendMultiplierOne,				// VUGFX_BLEND_ONE,
		Gnm::kBlendMultiplierSrcColor,			// VUGFX_BLEND_SRCCOLOR,
		Gnm::kBlendMultiplierOneMinusSrcColor,	// VUGFX_BLEND_INVSRCCOLOR,
		Gnm::kBlendMultiplierSrcAlpha,			// VUGFX_BLEND_SRCALPHA,
		Gnm::kBlendMultiplierOneMinusSrcAlpha,	// VUGFX_BLEND_INVSRCALPHA,
		Gnm::kBlendMultiplierDestAlpha,			// VUGFX_BLEND_DESTALPHA,
		Gnm::kBlendMultiplierOneMinusDestAlpha,	// VUGFX_BLEND_INVDESTALPHA,
		Gnm::kBlendMultiplierDestColor,			// VUGFX_BLEND_DESTCOLOR,
		Gnm::kBlendMultiplierOneMinusDestColor,	// VUGFX_BLEND_INVDESTCOLOR,
		Gnm::kBlendMultiplierSrcAlphaSaturate,	// VUGFX_BLEND_SRCALPHASAT,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_BLEND_MODE_COUNT);

	return sLookup[blendMode];
}

//*****************************************************************************
Gnm::PrimitiveSetupCullFaceMode VuPs4GfxTypes::convert(VuGfxCullMode cullMode)
{
	static Gnm::PrimitiveSetupCullFaceMode sLookup[] =
	{
		Gnm::kPrimitiveSetupCullFaceNone,	// VUGFX_CULL_NONE,
		Gnm::kPrimitiveSetupCullFaceBack,	// VUGFX_CULL_CW,
		Gnm::kPrimitiveSetupCullFaceFront,	// VUGFX_CULL_CCW,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_CULL_MODE_COUNT);

	return sLookup[cullMode];
}

//*****************************************************************************
Gnm::PrimitiveType VuPs4GfxTypes::convert(VuGfxPrimitiveType primitiveType)
{
	// primitive type lookup table
	static Gnm::PrimitiveType sLookup[] =
	{
		Gnm::kPrimitiveTypePointList,	// VUGFX_PT_POINTLIST,
		Gnm::kPrimitiveTypeLineList,	// VUGFX_PT_LINELIST,
		Gnm::kPrimitiveTypeLineStrip,	// VUGFX_PT_LINESTRIP,
		Gnm::kPrimitiveTypeTriList,		// VUGFX_PT_TRIANGLELIST,
		Gnm::kPrimitiveTypeTriStrip,	// VUGFX_PT_TRIANGLESTRIP,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);

	return sLookup[primitiveType];
}
