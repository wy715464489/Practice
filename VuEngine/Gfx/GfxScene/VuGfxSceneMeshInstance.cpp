//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSceneMeshInstance class
// 
//*****************************************************************************

#include "VuGfxSceneMeshInstance.h"
#include "VuGfxSceneMesh.h"
#include "VuGfxSceneMeshPart.h"
#include "VuGfxScene.h"
#include "VuGfxSceneBakeState.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuGfxSceneMeshInstance::VuGfxSceneMeshInstance() :
	mpMesh(VUNULL)
{
}

//*****************************************************************************
VuGfxSceneMeshInstance::~VuGfxSceneMeshInstance()
{
	if ( mpMesh )
		mpMesh->removeRef();
}

//*****************************************************************************
bool VuGfxSceneMeshInstance::bake(const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, VuBinaryDataWriter &writer)
{
	// mesh index
	std::string meshName = data["Mesh"].asString();
	int meshIndex = bakeState.mMeshLookup[meshName];
	writer.writeValue(meshIndex);

	return true;
}

//*****************************************************************************
void VuGfxSceneMeshInstance::load(VuBinaryDataReader &reader)
{
	reader.readValue(mMeshIndex);
}

//*****************************************************************************
bool VuGfxSceneMeshInstance::fixup(const VuGfxScene *pScene)
{
	mpMesh = pScene->mMeshes[mMeshIndex];
	mpMesh->addRef();

	return true;
}

//*****************************************************************************
void VuGfxSceneMeshInstance::gatherSceneInfo(VuGfxStaticSceneInfo &sceneInfo, const VuMatrix &mat)
{
	// instance info
	sceneInfo.mNumMeshInstances++;
	sceneInfo.mAabb.addAabb(mpMesh->getAabb(), mat);

	for ( VuGfxSceneMesh::Parts::const_iterator iter = mpMesh->mParts.begin(); iter != mpMesh->mParts.end(); iter++ )
	{
		sceneInfo.mNumDrawnVerts += (*iter)->mVertCount;
		sceneInfo.mNumDrawnTris += (*iter)->mTriCount;
	}
}