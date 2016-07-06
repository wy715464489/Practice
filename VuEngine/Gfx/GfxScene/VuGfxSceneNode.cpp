//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneNode class
// 
//*****************************************************************************

#include "VuGfxSceneNode.h"
#include "VuGfxSceneMesh.h"
#include "VuGfxSceneMeshInstance.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuGfxSceneNode::VuGfxSceneNode():
	mpMeshInstance(VUNULL)
{
	mTransform.loadIdentity();
}

//*****************************************************************************
VuGfxSceneNode::~VuGfxSceneNode()
{
	// release mesh instance
	if ( mpMeshInstance )
		mpMeshInstance->removeRef();

	// release children
	for ( Children::iterator iter = mChildren.begin(); iter != mChildren.end(); iter++ )
		(*iter)->removeRef();
}

//*****************************************************************************
bool VuGfxSceneNode::bake(const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, bool flipX, VuBinaryDataWriter &writer)
{
	// name
	std::string name = data["Name"].asString();
	writer.writeString(name);

	// transform (optional)
	VuMatrix transform = VuMatrix::identity();
	if ( data.hasMember("Transform") && !VuDataUtil::getValue(data["Transform"], transform) )
		return VUWARNING("Invalid transform in node %s", name.c_str());
	if ( flipX )
		transform.mT.mX *= -1.0f;
	writer.writeValue(transform);

	// mesh instance (optional)
	bool bMeshInstance = data.hasMember("MeshInstance");
	writer.writeValue(bMeshInstance);
	if ( bMeshInstance )
	{
		if ( !VuGfxSceneMeshInstance::bake(data["MeshInstance"], bakeState, writer) )
			return VUWARNING("Unable to bake mesh instance for node %s.", name.c_str());
	}

	// nodes (optional)
	const VuJsonContainer &nodes = data["Nodes"];
	int childCount = nodes.size();
	writer.writeValue(childCount);
	for ( int iNode = 0; iNode < nodes.size(); iNode++ )
		if ( !bake(nodes[iNode], bakeState, flipX, writer) )
			return false;

	return true;
}

//*****************************************************************************
void VuGfxSceneNode::load(VuBinaryDataReader &reader)
{
	reader.readString(mstrName);
	reader.readValue(mTransform);

	// mesh instance
	bool bMeshInstance;
	reader.readValue(bMeshInstance);
	if ( bMeshInstance )
	{
		mpMeshInstance = new VuGfxSceneMeshInstance;
		mpMeshInstance->load(reader);
	}

	// recurse
	int childCount;
	reader.readValue(childCount);
	mChildren.resize(childCount);
	for ( Children::iterator iter = mChildren.begin(); iter != mChildren.end(); iter++ )
	{
		*iter = new VuGfxSceneNode;
		(*iter)->load(reader);
	}
}

//*****************************************************************************
bool VuGfxSceneNode::fixup(const VuGfxScene *pScene, const VuMatrix &mat)
{
	VuMatrix transform = mTransform*mat;

	// fixup instance
	if ( mpMeshInstance )
		if ( !mpMeshInstance->fixup(pScene) )
			return false;

	// fixup children
	for ( Children::iterator iter = mChildren.begin(); iter != mChildren.end(); iter++ )
		if ( !(*iter)->fixup(pScene, transform) )
			return false;

	return true;
}

//*****************************************************************************
void VuGfxSceneNode::gatherSceneInfo(VuGfxStaticSceneInfo &sceneInfo, const VuMatrix &mat)
{
	VuMatrix transform = mTransform*mat;

	// node info
	sceneInfo.mNumNodes++;

	// mesh instance info
	if ( mpMeshInstance )
		mpMeshInstance->gatherSceneInfo(sceneInfo, transform);

	// calculate aabb for this node
	calculateAabbRecursive(mAabb, VuMatrix::identity());

	// recurse
	for ( Children::iterator iter = mChildren.begin(); iter != mChildren.end(); iter++ )
		(*iter)->gatherSceneInfo(sceneInfo, transform);
}

//*****************************************************************************
void VuGfxSceneNode::calculateAabbRecursive(VuAabb &aabb, const VuMatrix &mat)
{
	VuMatrix transform = mTransform*mat;

	if ( mpMeshInstance )
		aabb.addAabb(mpMeshInstance->mpMesh->getAabb(), transform);

	// recurse
	for ( Children::iterator iter = mChildren.begin(); iter != mChildren.end(); iter++ )
		(*iter)->calculateAabbRecursive(aabb, transform);
}