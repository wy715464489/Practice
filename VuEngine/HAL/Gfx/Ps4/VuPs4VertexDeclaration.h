//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the vertex declaration interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"

using namespace sce;


class VuPs4VertexDeclaration : public VuVertexDeclaration
{
public:
	VuPs4VertexDeclaration(const VuVertexDeclarationParams &params);
	~VuPs4VertexDeclaration();

	static VuPs4VertexDeclaration	*create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram);

	static Gnm::DataFormat convert(eGfxDeclType type);

	struct StreamElement
	{
		int				mOffset;
		Gnm::DataFormat	mDataFormat;
	};
	struct Stream
	{
		Stream() : mElementCount(0), mStride(0) {}
		StreamElement	mElements[16];
		int				mElementCount;
		int				mStride;
	};
	typedef std::vector<int> Offsets;

	VUUINT32	mHash;
	void		*mpFetchShader;
	VUUINT32	mShaderModifier;
	Stream		mStreams[2];
	int			mStreamCount;
};
