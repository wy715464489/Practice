//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the vertex declaration interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"


class VuMetalVertexDeclaration : public VuVertexDeclaration
{
public:
	VuMetalVertexDeclaration(const VuVertexDeclarationParams &params);
	~VuMetalVertexDeclaration();
	
	static VuMetalVertexDeclaration	*create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram);

	VUUINT32			mHash;
	MTLVertexDescriptor	*mpMTLVertexDescriptor;
};