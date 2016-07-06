//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Model Skin class
// 
//*****************************************************************************

#include "VuModelSkin.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/GfxScene/VuGfxScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMaterial.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuModelSkin::~VuModelSkin()
{
	clear();
}

//*****************************************************************************
void VuModelSkin::clear()
{
	VuGfxSort::IF()->flush();

	for ( int i = 0; i < mSkinMaterials.size(); i++ )
		VuGfxSort::IF()->releaseMaterial(mSkinMaterials[i]);
	mSkinMaterials.clear();
	mShaderCount = 0;
}

//*****************************************************************************
void VuModelSkin::build(const VuGfxScene *pGfxScene, const VuJsonContainer &data)
{
	// keep reference for old material around, since some textures may be shared
	SkinMaterials oldMaterials;
	for ( int i = 0; i < mSkinMaterials.size(); i++ )
		oldMaterials.push_back(VuGfxSort::IF()->duplicateMaterial(mSkinMaterials[i]));

	clear();

	if ( pGfxScene )
	{
		mSkinMaterials.resize(VuMaterialAsset::NUM_FLAVORS*(int)pGfxScene->mMaterials.size());
		mShaderCount = (int)pGfxScene->mMaterials.size();

		// global substitutions
		const VuJsonContainer &globalTextureData = data["Textures"];
		const VuJsonContainer &globalConstantData = data["Constants"];

		for ( int shaderIndex = 0; shaderIndex < mShaderCount; shaderIndex++ )
		{
			VuMaterialAsset *pMaterialAsset = pGfxScene->mMaterials[shaderIndex]->mpMaterialAsset;
			VuGfxSortMaterialDesc desc = pMaterialAsset->mpGfxSortMaterials[VuMaterialAsset::FLV_OPAQUE]->mDesc;

			// per-material substitutions
			const VuJsonContainer &materialTextureData = data["Materials"][pMaterialAsset->getAssetName()]["Textures"];
			const VuJsonContainer &materialConstantData = data["Materials"][pMaterialAsset->getAssetName()]["Constants"];

			// substitute textures
			for ( int i = 0; i < desc.mTextureArray.mCount; i++ )
			{
				VuGfxSortMaterialDesc::VuTextureArrayEntry &entry = desc.mTextureArray.maTextures[i];
				if ( materialTextureData[entry.mName].isString() )
				{
					memset(entry.mAssetName, 0, sizeof(entry.mAssetName)); // for correct CRC32
					VU_STRCPY(entry.mAssetName, sizeof(entry.mAssetName), materialTextureData[entry.mName].asCString());
				}
				else if ( globalTextureData[entry.mName].isString() )
				{
					memset(entry.mAssetName, 0, sizeof(entry.mAssetName)); // for correct CRC32
					VU_STRCPY(entry.mAssetName, sizeof(entry.mAssetName), globalTextureData[entry.mName].asCString());
				}
			}

			// adjust constants
			for ( int i = 0; i < desc.mConstantArray.mCount; i++ )
			{
				VuGfxSortMaterialDesc::VuConstantArrayEntry &entry = desc.mConstantArray.maConstants[i];
				if ( materialConstantData.hasMember(entry.mName) )
				{
					if ( entry.mType == VuGfxSortMaterialDesc::CONST_INT )
					{
						entry.mValue.mInt = materialConstantData[entry.mName].asInt();
					}
					else if ( entry.mType == VuGfxSortMaterialDesc::CONST_FLOAT )
					{
						entry.mValue.mFloat = materialConstantData[entry.mName].asFloat();
					}
					else if ( entry.mType == VuGfxSortMaterialDesc::CONST_FLOAT3 )
					{
						entry.mValue.mFloat3[0] = materialConstantData[entry.mName]["X"].asFloat();
						entry.mValue.mFloat3[1] = materialConstantData[entry.mName]["Y"].asFloat();
						entry.mValue.mFloat3[2] = materialConstantData[entry.mName]["Z"].asFloat();
					}
					else if ( entry.mType == VuGfxSortMaterialDesc::CONST_FLOAT4 )
					{
						entry.mValue.mFloat4[0] = materialConstantData[entry.mName]["X"].asFloat();
						entry.mValue.mFloat4[1] = materialConstantData[entry.mName]["Y"].asFloat();
						entry.mValue.mFloat4[2] = materialConstantData[entry.mName]["Z"].asFloat();
						entry.mValue.mFloat4[3] = materialConstantData[entry.mName]["W"].asFloat();
					}
				}
				else if ( globalConstantData.hasMember(entry.mName) )
				{
					if ( entry.mType == VuGfxSortMaterialDesc::CONST_INT )
					{
						entry.mValue.mInt = globalConstantData[entry.mName].asInt();
					}
					else if ( entry.mType == VuGfxSortMaterialDesc::CONST_FLOAT )
					{
						entry.mValue.mFloat = globalConstantData[entry.mName].asFloat();
					}
					else if ( entry.mType == VuGfxSortMaterialDesc::CONST_FLOAT3 )
					{
						entry.mValue.mFloat3[0] = globalConstantData[entry.mName]["X"].asFloat();
						entry.mValue.mFloat3[1] = globalConstantData[entry.mName]["Y"].asFloat();
						entry.mValue.mFloat3[2] = globalConstantData[entry.mName]["Z"].asFloat();
					}
				}
			}

			for ( int iFlavor = 0; iFlavor < VuMaterialAsset::NUM_FLAVORS; iFlavor++ )
			{
				VuGfxSortMaterial *pOldMat = pMaterialAsset->mpGfxSortMaterials[iFlavor];
				VuGfxSortMaterial *pNewMat = VuGfxSort::IF()->createMaterial(pOldMat->mpPipelineState, desc);
				mSkinMaterials[iFlavor*mShaderCount + shaderIndex] = pNewMat;
			}
		}
	}

	// release old materials
	for ( int i = 0; i < oldMaterials.size(); i++ )
		VuGfxSort::IF()->releaseMaterial(oldMaterials[i]);
}
