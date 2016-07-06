//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the vertex declaration interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"


class VuOglesVertexDeclaration : public VuVertexDeclaration
{
public:
	VuOglesVertexDeclaration(const VuVertexDeclarationParams &params);
	~VuOglesVertexDeclaration();

	void		build();

	static VuOglesVertexDeclaration	*create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram);

	struct VuAttrib
	{
		int		mUsage;
		int		mSize;
		GLenum	mType;
		bool	mNormalized;
		int		mOffset;
	};
	enum { MAX_ATTRIBUTES = 8 };

	VUUINT32	mHash;
	VuAttrib	mEnabledAttribs[MAX_ATTRIBUTES];
	int			mEnabledAttribCount;
	int			mDisabledAttribs[MAX_ATTRIBUTES];
	int			mDisabledAttribCount;
};
