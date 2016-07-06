//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to OpenGL ES.
// 
//*****************************************************************************

#pragma once


#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuGfxTypes.h"


namespace VuOglesGfxTypes
{

	GLenum		convert(VuGfxCompFunc compFunc);
	GLenum		convert(VuGfxBlendMode blendMode);
	GLint		convert(VuGfxTextureAddress textureAddress);
	GLint		convert(VuGfxTextureFilterType magFilter);
	GLint		convert(VuGfxTextureFilterType minFilter, VuGfxTextureFilterType mipFilter);
	GLenum		convert(VuGfxPrimitiveType primitiveType);
	int			vertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount);
	GLenum		convert(VuGfxStencilOp stencilOp);

} // namespace VuOglGfxTypes
