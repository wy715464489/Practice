//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to OpenGL ES.
// 
//*****************************************************************************

#include "VuOglesGfxTypes.h"


//*****************************************************************************
GLenum VuOglesGfxTypes::convert(VuGfxCompFunc compFunc)
{
	// compare function lookup table
	static GLenum sLookup[] =
	{
		GL_NEVER,		// VUGFX_COMP_NEVER,
		GL_LESS,		// VUGFX_COMP_LESS,
		GL_EQUAL,		// VUGFX_COMP_EQUAL,
		GL_LEQUAL,		// VUGFX_COMP_LESSEQUAL,
		GL_GREATER,		// VUGFX_COMP_GREATER,
		GL_NOTEQUAL,	// VUGFX_COMP_NOTEQUAL,
		GL_GEQUAL,		// VUGFX_COMP_GREATEREQUAL,
		GL_ALWAYS ,		// VUGFX_COMP_ALWAYS,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_COMP_FUNC_COUNT);

	return sLookup[compFunc];
}

//*****************************************************************************
GLenum VuOglesGfxTypes::convert(VuGfxBlendMode blendMode)
{
	static GLenum sLookup[] =
	{
		GL_ZERO,				// VUGFX_BLEND_ZERO,
		GL_ONE,					// VUGFX_BLEND_ONE,
		GL_SRC_COLOR,			// VUGFX_BLEND_SRCCOLOR,
		GL_ONE_MINUS_SRC_COLOR,	// VUGFX_BLEND_INVSRCCOLOR,
		GL_SRC_ALPHA,			// VUGFX_BLEND_SRCALPHA,
		GL_ONE_MINUS_SRC_ALPHA,	// VUGFX_BLEND_INVSRCALPHA,
		GL_DST_ALPHA,			// VUGFX_BLEND_DESTALPHA,
		GL_ONE_MINUS_DST_ALPHA,	// VUGFX_BLEND_INVDESTALPHA,
		GL_DST_COLOR,			// VUGFX_BLEND_DESTCOLOR,
		GL_ONE_MINUS_DST_COLOR,	// VUGFX_BLEND_INVDESTCOLOR,
		GL_SRC_ALPHA_SATURATE,	// VUGFX_BLEND_SRCALPHASAT,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_BLEND_MODE_COUNT);

	return sLookup[blendMode];
}

//*****************************************************************************
GLint VuOglesGfxTypes::convert(VuGfxTextureAddress textureAddress)
{
	static GLint sLookup[] =
	{
		GL_REPEAT,			// VUGFX_ADDRESS_WRAP,
		GL_CLAMP_TO_EDGE,	// VUGFX_ADDRESS_CLAMP,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_TEXTURE_ADDRESS_COUNT);

	return sLookup[textureAddress];
}

//*****************************************************************************
GLint VuOglesGfxTypes::convert(VuGfxTextureFilterType magFilter)
{
	static GLint sLookup[] =
	{
		GL_NEAREST,	// VUGFX_TEXF_NONE,
		GL_NEAREST,	// VUGFX_TEXF_POINT,
		GL_LINEAR,	// VUGFX_TEXF_LINEAR,
		GL_LINEAR,	// VUGFX_TEXF_ANISOTROPIC,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);

	return sLookup[magFilter];
}

//*****************************************************************************
GLint VuOglesGfxTypes::convert(VuGfxTextureFilterType minFilter, VuGfxTextureFilterType mipFilter)
{
	static GLint sLookup[][VUGFX_TEXTURE_FILTER_TYPE_COUNT] =
	{
		// VUGFX_TEXF_NONE, VUGFX_TEXF_POINT,          VUGFX_TEXF_LINEAR,        VUGFX_TEXF_ANISOTROPIC
		{ GL_NEAREST,       GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR },	// VUGFX_TEXF_NONE,
		{ GL_NEAREST,       GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR },	// VUGFX_TEXF_POINT,
		{ GL_LINEAR,        GL_LINEAR_MIPMAP_NEAREST,  GL_LINEAR_MIPMAP_LINEAR,  GL_LINEAR_MIPMAP_LINEAR  },	// VUGFX_TEXF_LINEAR,
		{ GL_LINEAR,        GL_LINEAR_MIPMAP_NEAREST,  GL_LINEAR_MIPMAP_LINEAR,  GL_LINEAR_MIPMAP_LINEAR  },	// VUGFX_TEXF_ANISOTROPIC,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(GLint) == VUGFX_TEXTURE_FILTER_TYPE_COUNT*VUGFX_TEXTURE_FILTER_TYPE_COUNT);

	return sLookup[minFilter][mipFilter];
}

//*****************************************************************************
GLenum VuOglesGfxTypes::convert(VuGfxPrimitiveType primitiveType)
{
	// primitive type lookup table
	static GLenum sLookup[] =
	{
		GL_POINTS,			// VUGFX_PT_POINTLIST,
		GL_LINES,			// VUGFX_PT_LINELIST,
		GL_LINE_STRIP,		// VUGFX_PT_LINESTRIP,
		GL_TRIANGLES,		// VUGFX_PT_TRIANGLELIST,
		GL_TRIANGLE_STRIP,	// VUGFX_PT_TRIANGLESTRIP,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);

	return sLookup[primitiveType];
}

//*****************************************************************************
GLenum VuOglesGfxTypes::convert(VuGfxStencilOp stencilOp)
{
	// stencil op lookup table
	static GLenum sLookup[] =
	{
		GL_KEEP,		// VUGFX_STENCIL_OP_KEEP
		GL_ZERO,		// VUGFX_STENCIL_OP_ZERO
		GL_REPLACE,		// VUGFX_STENCIL_OP_REPLACE
		GL_INCR,		// VUGFX_STENCIL_OP_INCR
		GL_INCR_WRAP,	// VUGFX_STENCIL_OP_INCR_WRAP
		GL_DECR,		// VUGFX_STENCIL_OP_DECR
		GL_DECR_WRAP,	// VUGFX_STENCIL_OP_DECR_WRAP
		GL_INVERT,		// VUGFX_STENCIL_OP_INVERT
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_STENCIL_OP_COUNT);

	return sLookup[stencilOp];
}