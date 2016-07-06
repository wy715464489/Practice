//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the vertex declaration interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"

class VuD3d11Shader;


class VuD3d11VertexDeclaration : public VuVertexDeclaration
{
public:
	VuD3d11VertexDeclaration(const VuVertexDeclarationParams &params);
	~VuD3d11VertexDeclaration();

	static VuD3d11VertexDeclaration	*create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram);

	static D3D11_INPUT_ELEMENT_DESC	*convert(const VuVertexDeclarationElements &params);

	VUUINT32			mHash;
	ID3D11InputLayout	*mpD3dInputLayout;
	VuD3d11Shader		*mpVertexShader;
};
