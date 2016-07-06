//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Types used by Gfx library.
// 
//*****************************************************************************

#pragma once


// texture types
enum VuGfxTextureType {
	VUGFX_TEXTURE_TYPE_DEFAULT,
	VUGFX_TEXTURE_TYPE_BUMP,
	VUGFX_TEXTURE_TYPE_SDF,

	VUGFX_TEXTURE_TYPE_COUNT
};

// platform format types
enum VuGfxFormatDX {
	VUGFX_FORMAT_DX_32BIT,
	VUGFX_FORMAT_DX_S3TC,

	VUGFX_FORMAT_DX_COUNT
};

enum VuGfxFormatIOS {
	VUGFX_FORMAT_IOS_32BIT,
	VUGFX_FORMAT_IOS_S3TC,
	VUGFX_FORMAT_IOS_PVRTC,

	VUGFX_FORMAT_IOS_COUNT
};

enum VuGfxFormatOGLES {
	VUGFX_FORMAT_OGLES_32BIT,
	VUGFX_FORMAT_OGLES_ETC1_DXT5,

	VUGFX_FORMAT_OGLES_COUNT
};

// clear flags
enum {
	VUGFX_CLEAR_COLOR	= (1<<0),
	VUGFX_CLEAR_DEPTH	= (1<<1),
};

// usage flags
enum {
	VUGFX_USAGE_DYNAMIC			= (1<<0), // The resource will be updated on the fly (only exists in video memory).
};

// compare functions
enum VuGfxCompFunc {
	VUGFX_COMP_NEVER,
	VUGFX_COMP_LESS,
	VUGFX_COMP_EQUAL,
	VUGFX_COMP_LESSEQUAL,
	VUGFX_COMP_GREATER,
	VUGFX_COMP_NOTEQUAL,
	VUGFX_COMP_GREATEREQUAL,
	VUGFX_COMP_ALWAYS,

	VUGFX_COMP_FUNC_COUNT
};

// blend modes
enum VuGfxBlendMode {
	VUGFX_BLEND_ZERO,
	VUGFX_BLEND_ONE,
	VUGFX_BLEND_SRCCOLOR,
	VUGFX_BLEND_INVSRCCOLOR,
	VUGFX_BLEND_SRCALPHA,
	VUGFX_BLEND_INVSRCALPHA,
	VUGFX_BLEND_DESTALPHA,
	VUGFX_BLEND_INVDESTALPHA,
	VUGFX_BLEND_DESTCOLOR,
	VUGFX_BLEND_INVDESTCOLOR,
	VUGFX_BLEND_SRCALPHASAT,

	VUGFX_BLEND_MODE_COUNT
};

// cull modes
enum VuGfxCullMode {
	VUGFX_CULL_NONE,
	VUGFX_CULL_CW,
	VUGFX_CULL_CCW,

	VUGFX_CULL_MODE_COUNT
};

// texture address modes
enum VuGfxTextureAddress {
	VUGFX_ADDRESS_WRAP,
	VUGFX_ADDRESS_CLAMP,

	VUGFX_TEXTURE_ADDRESS_COUNT
};

// texture filter types
enum VuGfxTextureFilterType {
	VUGFX_TEXF_NONE,
	VUGFX_TEXF_POINT,
	VUGFX_TEXF_LINEAR,
	VUGFX_TEXF_ANISOTROPIC,

	VUGFX_TEXTURE_FILTER_TYPE_COUNT
};

// primitive types
enum VuGfxPrimitiveType {
	VUGFX_PT_POINTLIST,
	VUGFX_PT_LINELIST,
	VUGFX_PT_LINESTRIP,
	VUGFX_PT_TRIANGLELIST,
	VUGFX_PT_TRIANGLESTRIP,

	VUGFX_PRIMITIVE_TYPE_COUNT
};

// format
enum VuGfxFormat {
	VUGFX_FORMAT_UNKNOWN,

	// buffer formats
	VUGFX_FORMAT_D24S8,			// 32-bit z-buffer bit depth using 24 bits for the depth channel and 8 bits for the stencil channel.

	// signed formats
	VUGFX_FORMAT_V8U8,			// 16-bit bump-map format using 8 bits each for u and v data.
	VUGFX_FORMAT_LIN_V8U8,
	VUGFX_FORMAT_LIN_R16G16B16A16_SNORM,

	// unsigned formats
	VUGFX_FORMAT_A8R8G8B8,		// 32-bit ARGB pixel format with alpha, using 8 bits per channel.
	VUGFX_FORMAT_R8,			// A single-component, 8-bit unsigned-integer format.
	VUGFX_FORMAT_LIN_R8,
	VUGFX_FORMAT_R16,			// A single-component, 16-bit unsigned-integer format.
	VUGFX_FORMAT_LIN_R16,
	VUGFX_FORMAT_L8A8,			// 16-bit using 8 bits each for alpha and luminance.
	VUGFX_FORMAT_LIN_L8A8,

	// float Formats
	VUGFX_FORMAT_R16F,			// 16-bit float format using 16 bits for the red channel.
	VUGFX_FORMAT_R32F,			// 32-bit float format using 32 bits for the red channel.
	VUGFX_FORMAT_R16G16F,		// 32-bit float format using 16 bits per channel (red/green).

	VUGFX_FORMAT_COUNT
};

// format
enum VuGfxMultiSampleType {
	VUGFX_MULTISAMPLE_NONE,
	VUGFX_MULTISAMPLE_2,
	VUGFX_MULTISAMPLE_4,

	VUGFX_MULTISAMPLE_TYPE_COUNT
};

// text drawing flags
enum {
	VUGFX_TEXT_DRAW_LEFT		= 0x00,
	VUGFX_TEXT_DRAW_RIGHT		= 0x01,
	VUGFX_TEXT_DRAW_HCENTER		= 0x02,

	VUGFX_TEXT_DRAW_TOP			= 0x00,
	VUGFX_TEXT_DRAW_BOTTOM		= 0x04,
	VUGFX_TEXT_DRAW_BASELINE	= 0x08,
	VUGFX_TEXT_DRAW_VCENTER		= 0x10,

	VUGFX_TEXT_DRAW_CLIP		= 0x20,
	VUGFX_TEXT_DRAW_WORDBREAK	= 0x40,

	VUGFX_TEXT_DRAW_SYNCHRONOUS	= 0x1000,
};

// declaration type
enum eGfxDeclType {
	VUGFX_DECL_TYPE_INVALID = -1,

	VUGFX_DECL_TYPE_FLOAT1,		// One-component float expanded to (float, 0, 0, 1). 
	VUGFX_DECL_TYPE_FLOAT2,		// Two-component float expanded to (float, float, 0, 1).
	VUGFX_DECL_TYPE_FLOAT3,		// Three-component float expanded to (float, float, float, 1). 
	VUGFX_DECL_TYPE_FLOAT4,		// Four-component float expanded to (float, float, float, float).
	VUGFX_DECL_TYPE_BYTE4,		// Four-component, signed byte.
	VUGFX_DECL_TYPE_BYTE4N,		// Four-component byte with each byte normalized by dividing with 127.0f.
	VUGFX_DECL_TYPE_UBYTE4,		// Four-component, unsigned byte.
	VUGFX_DECL_TYPE_UBYTE4N,	// Four-component byte with each byte normalized by dividing with 255.0f.
	VUGFX_DECL_TYPE_SHORT2N,	// Normalized, two-component, signed short, expanded to (first short/32767.0, second short/32767.0, 0, 1). 
	VUGFX_DECL_TYPE_SHORT4N,	// Normalized, four-component, signed short, expanded to (first short/32767.0, second short/32767.0, third short/32767.0, fourth short/32767.0).
	VUGFX_DECL_TYPE_FLOAT16_2,	// Two-component, 16-bit, floating point expanded to (value, value, 0, 1). 
	VUGFX_DECL_TYPE_FLOAT16_4,	// Four-component, 16-bit, floating point expanded to (value, value, value, value). 

	VUGFX_DECL_TYPE_COUNT
};

// declaration usage
enum eGfxDeclUsage {
	VUGFX_DECL_USAGE_INVALID = -1,

	VUGFX_DECL_USAGE_POSITION,
	VUGFX_DECL_USAGE_NORMAL,
	VUGFX_DECL_USAGE_COLOR,
	VUGFX_DECL_USAGE_TANGENT,
	VUGFX_DECL_USAGE_BLENDWEIGHT,
	VUGFX_DECL_USAGE_BLENDINDICES,
	VUGFX_DECL_USAGE_TEXCOORD,

	VUGFX_DECL_USAGE_COUNT
};

// stencil operations
enum VuGfxStencilOp {
	VUGFX_STENCIL_OP_KEEP,		// Keeps the current value.
	VUGFX_STENCIL_OP_ZERO,		// Sets the stencil buffer value to 0.
	VUGFX_STENCIL_OP_REPLACE,	// Sets the stencil buffer value to ref, as specified by setStencilFunc.
	VUGFX_STENCIL_OP_INCR,		// Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
	VUGFX_STENCIL_OP_INCR_WRAP,	// Increments the current stencil buffer value. Wraps stencil buffer value to zero when incrementing the maximum representable unsigned value.
	VUGFX_STENCIL_OP_DECR,		// Decrements the current stencil buffer value. Clamps to 0.
	VUGFX_STENCIL_OP_DECR_WRAP,	// Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum representable unsigned value when decrementing a stencil buffer value of zero.
	VUGFX_STENCIL_OP_INVERT,	// Bitwise inverts the current stencil buffer value.

	VUGFX_STENCIL_OP_COUNT
};

// limitations
enum {
	VUGFX_MAX_BONE_COUNT = 48,
};
