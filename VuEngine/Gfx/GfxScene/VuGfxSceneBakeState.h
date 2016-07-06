//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Bake State class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/Json/VuJsonContainer.h"


class VuGfxSceneBakeState
{
public:
	struct Chunk
	{
		Chunk() : mVertexStride(0), mPartVertexOffset(0) {}

		std::string					mShaderFileName;
		VuVertexDeclarationElements	mShaderElements;
		int							mVertexStride;
		std::vector<VUBYTE>			mVertexData;
		std::vector<VUUINT16>		mIndexData;
		int							mPartVertexOffset;
	};

	int			chunkIndex(const std::string &shaderFileName) const;

	typedef std::vector<Chunk> Chunks;
	typedef std::map<std::string, int> Lookup;

	Chunks			mChunks;
	Lookup			mChunkLookup;
	Lookup			mMaterialLookup;
	Lookup			mMeshLookup;
};
