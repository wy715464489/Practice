//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSceneMeshPart class
// 
//*****************************************************************************

#include "VuGfxSceneMeshPart.h"
#include "VuGfxSceneMaterial.h"
#include "VuGfxSceneChunk.h"
#include "VuGfxScene.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuGfxSceneMeshPart::VuGfxSceneMeshPart():
	mpMaterial(VUNULL),
	mMinIndex(0),
	mMaxIndex(0),
	mVertCount(0),
	mStartIndex(0),
	mTriCount(0)
{
}

//*****************************************************************************
VuGfxSceneMeshPart::~VuGfxSceneMeshPart()
{
	if ( mpMaterial )
		mpMaterial->removeRef();

	if ( mpChunk )
		mpChunk->removeRef();
}

//*****************************************************************************
void VuGfxSceneMeshPart::load(VuBinaryDataReader &reader)
{
	reader.readValue(mChunkIndex);
	reader.readValue(mMaterialIndex);
	reader.readValue(mMinIndex);
	reader.readValue(mMaxIndex);
	reader.readValue(mStartIndex);
	reader.readValue(mTriCount);
	reader.readValue(mAabb);

	mVertCount = mMaxIndex - mMinIndex + 1;
}

//*****************************************************************************
bool VuGfxSceneMeshPart::fixup(const VuGfxScene *pScene)
{
	mpMaterial = pScene->mMaterials[mMaterialIndex];
	mpMaterial->addRef();

	mpChunk = pScene->mChunks[mChunkIndex];
	mpChunk->addRef();

	return true;
}

//*****************************************************************************
void VuGfxSceneMeshPart::gatherSceneInfo(VuGfxSceneInfo &sceneInfo)
{
	// instance info
	sceneInfo.mNumMeshParts++;
}