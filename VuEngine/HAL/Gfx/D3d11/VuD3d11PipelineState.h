//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the pipeline state interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuPipelineState.h"

class VuD3d11ShaderProgram;


class VuD3d11PipelineState : public VuPipelineState
{
public:
	VuD3d11PipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	~VuD3d11PipelineState();

	static VuD3d11PipelineState	*create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);

	VUUINT64				mHash;
	VuD3d11ShaderProgram	*mpD3dShaderProgram; // convenience
	ID3D11VertexShader		*mpD3dVertexShader; // convenience
	ID3D11PixelShader		*mpD3dPixelShader; // convenience
	ID3D11InputLayout		*mpD3dInputLayout; // convenience
	ID3D11BlendState		*mpD3dBlendState;
};
