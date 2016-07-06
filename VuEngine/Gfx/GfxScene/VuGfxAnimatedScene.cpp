//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Scene class
// 
//*****************************************************************************

#include "VuGfxAnimatedScene.h"
#include "VuGfxSceneMesh.h"
#include "VuGfxSceneBakeState.h"
#include "VuGfxSceneMaterial.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"


//*****************************************************************************
VuGfxAnimatedScene::VuGfxAnimatedScene()
{
}

//*****************************************************************************
void VuGfxAnimatedScene::clear()
{
	VuGfxScene::clear();
}

//*****************************************************************************
bool VuGfxAnimatedScene::load(const VuJsonContainer &data)
{
	VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
	if ( !bake(VuJsonContainer::null, bakeParams, data, bakeParams.mWriter) )
		return false;

	VuBinaryDataReader reader(bakeParams.mData);
	return load(reader);
}

//*****************************************************************************
bool VuGfxAnimatedScene::load(VuBinaryDataReader &reader)
{
	if ( !VuGfxScene::load(reader) )
		return false;

	// gather info
	gatherSceneInfo();

	// warnings
	if ( VuEngine::IF()->gameMode() )
	{
		for ( Materials::iterator iter = mMaterials.begin(); iter != mMaterials.end(); iter++ )
			if ( !(*iter)->mpMaterialAsset->mbSkinning )
				VUPRINTF("Warning:  Animated scene using non-skinned material %s\n", (*iter)->mpMaterialAsset->getAssetName().c_str());
	}

	return true;
}

//*****************************************************************************
bool VuGfxAnimatedScene::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, VuBinaryDataWriter &writer)
{
	VuGfxSceneBakeState bakeState;

	if ( !VuGfxScene::bake(creationInfo, bakeParams, data, bakeState, true, false, writer) )
		return false;

	return true;
}

//*****************************************************************************
void VuGfxAnimatedScene::gatherSceneInfo()
{
	mAnimatedInfo.mAabb.reset();

	for ( Meshes::iterator iter = mMeshes.begin(); iter != mMeshes.end(); iter++ )
		mAnimatedInfo.mAabb.addAabb((*iter)->getAabb());

	if ( !mAnimatedInfo.mAabb.isValid() )
		mAnimatedInfo.mAabb.addPoint(VuVector3(0,0,0));
}
