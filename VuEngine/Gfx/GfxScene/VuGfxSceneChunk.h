//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx Scene Chunk class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuGfxSceneBakeState.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"

class VuGfxSceneBakeState;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuVertexBuffer;
class VuIndexBuffer;
class VuGfxSortMesh;


class VuGfxSceneChunk : public VuRefObj
{
protected:
	~VuGfxSceneChunk();
public:
	VuGfxSceneChunk(int index);

	void			load(VuBinaryDataReader &reader);
	static bool		bake(VuGfxSceneBakeState::Chunk &chunk, VuBinaryDataWriter &writer);

	int				mIndex;
	int				mVertexStride;
	VuVertexBuffer	*mpVertexBuffer;
	VuIndexBuffer	*mpIndexBuffer;

	VuGfxSortMesh	*mpGfxSortMesh;
};
