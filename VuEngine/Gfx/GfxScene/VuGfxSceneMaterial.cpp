//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneMaterial class
// 
//*****************************************************************************

#include "VuGfxSceneMaterial.h"
#include "VuGfxSceneBakeState.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuGfxSceneMaterial::VuGfxSceneMaterial(int index):
	mIndex(index),
	mpMaterialAsset(VUNULL)
{
}

//*****************************************************************************
VuGfxSceneMaterial::~VuGfxSceneMaterial()
{
	VuAssetFactory::IF()->releaseAsset(mpMaterialAsset);
}

//*****************************************************************************
bool VuGfxSceneMaterial::load(VuBinaryDataReader &reader)
{
	// shader asset
	std::string materialAssetName;
	reader.readString(materialAssetName);

	mpMaterialAsset = VuAssetFactory::IF()->createAsset<VuMaterialAsset>(materialAssetName);
	if ( mpMaterialAsset == VUNULL )
		return false;

	return true;
}

//*****************************************************************************
bool VuGfxSceneMaterial::bake(const VuJsonContainer &data, VuBinaryDataWriter &writer, VuAssetDependencies &dependencies)
{
	// material asset
	const std::string &assetName = data["MaterialAsset"].asString();
	writer.writeString(assetName);

	dependencies.addAsset(VuMaterialAsset::msRTTI.mstrType, assetName);

	return true;
}
