//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx Scene Chunk class
// 
//*****************************************************************************

#include "VuGfxSceneChunk.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuGfxSceneChunk::VuGfxSceneChunk(int index):
	mIndex(index),
	mpVertexBuffer(VUNULL),
	mpIndexBuffer(VUNULL),
	mpGfxSortMesh(VUNULL)
{
}

//*****************************************************************************
VuGfxSceneChunk::~VuGfxSceneChunk()
{
	if ( mpVertexBuffer )
		mpVertexBuffer->removeRef();

	if ( mpIndexBuffer )
		mpIndexBuffer->removeRef();

	if ( mpGfxSortMesh )
		VuGfxSort::IF()->releaseMesh(mpGfxSortMesh);
}

//*****************************************************************************
void VuGfxSceneChunk::load(VuBinaryDataReader &reader)
{
	// stride
	reader.readValue(mVertexStride);

	// verts
	mpVertexBuffer = VuVertexBuffer::load(reader);

	// indices
	mpIndexBuffer = VuIndexBuffer::load(reader);

	// create gfx sort mesh
	VuGfxSortMeshDesc desc;
	desc.mpVertexBuffer = mpVertexBuffer;
	desc.mpIndexBuffer = mpIndexBuffer;
	mpGfxSortMesh = VuGfxSort::IF()->createMesh(desc);
}

//*****************************************************************************
bool VuGfxSceneChunk::bake(VuGfxSceneBakeState::Chunk &chunk, VuBinaryDataWriter &writer)
{
	// stride
	writer.writeValue(chunk.mVertexStride);

	// vertex buffer
	if ( !VuVertexBuffer::bake(chunk.mVertexData, writer) )
		return false;

	// index buffer
	if ( !VuIndexBuffer::bake(chunk.mIndexData, writer) )
		return false;

	return true;
}
