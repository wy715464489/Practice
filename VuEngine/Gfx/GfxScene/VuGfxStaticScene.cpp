//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Scene class
// 
//*****************************************************************************

#include "VuGfxStaticScene.h"
#include "VuGfxSceneNode.h"
#include "VuGfxSceneBakeState.h"
#include "VuGfxSceneMaterial.h"
#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuGfxStaticScene::VuGfxStaticScene()
{
}

//*****************************************************************************
void VuGfxStaticScene::clear()
{
	// release nodes
	for ( Nodes::iterator iter = mNodes.begin(); iter != mNodes.end(); iter++ )
		(*iter)->removeRef();
	mNodes.clear();

	VuGfxScene::clear();
}

//*****************************************************************************
bool VuGfxStaticScene::load(const VuJsonContainer &data)
{
	VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
	if ( !bake(VuJsonContainer::null, bakeParams, data, false, bakeParams.mWriter) )
		return false;

	VuBinaryDataReader reader(bakeParams.mData);
	return load(reader);
}

//*****************************************************************************
bool VuGfxStaticScene::load(VuBinaryDataReader &reader)
{
	if ( !VuGfxScene::load(reader) )
		return false;

	// nodes
	int nodeCount;
	reader.readValue(nodeCount);
	mNodes.resize(nodeCount);
	for ( Nodes::iterator iter = mNodes.begin(); iter != mNodes.end(); iter++ )
	{
		*iter = new VuGfxSceneNode;
		(*iter)->load(reader);
	}

	for ( Nodes::iterator iter = mNodes.begin(); iter != mNodes.end(); iter++ )
		(*iter)->fixup(this, VuMatrix::identity());

	// gather info
	gatherSceneInfo();

	// warnings
	for ( Materials::iterator iter = mMaterials.begin(); iter != mMaterials.end(); iter++ )
		if ( (*iter)->mpMaterialAsset->mbSkinning )
			VUPRINTF("Warning:  Static scene using skinned material %s\n", (*iter)->mpMaterialAsset->getAssetName().c_str());

	return true;
}

//*****************************************************************************
bool VuGfxStaticScene::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, bool flipX, VuBinaryDataWriter &writer)
{
	VuGfxSceneBakeState bakeState;

	if ( !VuGfxScene::bake(creationInfo, bakeParams, data, bakeState, false, flipX, writer) )
		return false;

	// nodes
	const VuJsonContainer &nodes = data["Nodes"];
	int nodeCount = nodes.size();
	writer.writeValue(nodeCount);
	for ( int iNode = 0; iNode < nodes.size(); iNode++ )
		if ( !VuGfxSceneNode::bake(nodes[iNode], bakeState, flipX, writer) )
			return false;

	return true;
}

//*****************************************************************************
void VuGfxStaticScene::gatherSceneInfo()
{
	// recurse into nodes
	for ( Nodes::iterator iter = mNodes.begin(); iter != mNodes.end(); iter++ )
		(*iter)->gatherSceneInfo(mStaticInfo, VuMatrix::identity());
}