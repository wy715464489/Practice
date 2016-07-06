//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxScene utility
// 
//*****************************************************************************

#include "VuGfxSceneUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Assets/VuMaterialAsset.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
void VuGfxSceneUtil::optimizeVertexDeclaration(const std::string &platform, bool bSkinning, const VuVertexDeclarationElements &elements, VuVertexDeclarationElements &optElements)
{
	int offset = 0;
	for ( VuVertexDeclarationElements::const_iterator itElement = elements.begin(); itElement != elements.end(); itElement++ )
	{
		if ( !bSkinning && (itElement->mUsage == VUGFX_DECL_USAGE_BLENDWEIGHT || itElement->mUsage == VUGFX_DECL_USAGE_BLENDINDICES) )
		{
			// skip
		}
		else if ( itElement->mType == VUGFX_DECL_TYPE_FLOAT3 && (itElement->mUsage == VUGFX_DECL_USAGE_NORMAL || itElement->mUsage == VUGFX_DECL_USAGE_TANGENT) )
		{
			if ( VuGfx::supportsVertexDeclType(platform, VUGFX_DECL_TYPE_BYTE4N) )
			{
				optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, VUGFX_DECL_TYPE_BYTE4N, itElement->mUsage, itElement->mUsageIndex));
				offset += 4;
			}
			else if ( VuGfx::supportsVertexDeclType(platform, VUGFX_DECL_TYPE_SHORT4N) )
			{
				optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, VUGFX_DECL_TYPE_SHORT4N, itElement->mUsage, itElement->mUsageIndex));
				offset += 8;
			}
			else
			{
				// no optimization
				optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, itElement->mType, itElement->mUsage, itElement->mUsageIndex));
				offset += itElement->size();
			}
		}
		else if ( itElement->mType == VUGFX_DECL_TYPE_FLOAT2 && itElement->mUsage == VUGFX_DECL_USAGE_TEXCOORD && VuGfx::supportsVertexDeclType(platform, VUGFX_DECL_TYPE_FLOAT16_2) )
		{
			optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, VUGFX_DECL_TYPE_FLOAT16_2, itElement->mUsage, itElement->mUsageIndex));
			offset += 4;
		}
		else if ( itElement->mType == VUGFX_DECL_TYPE_FLOAT3 && itElement->mUsage == VUGFX_DECL_USAGE_BLENDWEIGHT )
		{
			if ( VuGfx::supportsVertexDeclType(platform, VUGFX_DECL_TYPE_UBYTE4N) )
			{
				optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, VUGFX_DECL_TYPE_UBYTE4N, itElement->mUsage, itElement->mUsageIndex));
				offset += 4;
			}
			else
			{
				// no optimization
				optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, itElement->mType, itElement->mUsage, itElement->mUsageIndex));
				offset += itElement->size();
			}
		}
		else
		{
			// no optimization
			optElements.push_back(VuVertexDeclarationElement(itElement->mStream, (VUUINT16)offset, itElement->mType, itElement->mUsage, itElement->mUsageIndex));
			offset += itElement->size();
		}
	}
}

//*****************************************************************************
void VuGfxSceneUtil::gatherSceneMaterialNames(const VuJsonContainer &scene, Names &materialNames)
{
	// mame list of actual materials in scene
	const VuJsonContainer &meshes = scene["Meshes"];
	for ( int iMesh = 0; iMesh < meshes.size(); iMesh++ )
	{
		const VuJsonContainer &parts = meshes[iMesh]["Parts"];
		for ( int iPart = 0; iPart < parts.size(); iPart++ )
			materialNames.insert(parts[iPart]["Material"].asString());
	}
}

//*****************************************************************************
void VuGfxSceneUtil::cleanUpMaterials(const VuJsonContainer &creationInfo, const VuJsonContainer &scene, VuJsonContainer &materials, const std::string &defaultMaterialAsset)
{
	// mame list of actual materials in scene
	std::set<std::string> sceneMaterials;
	gatherSceneMaterialNames(scene, sceneMaterials);

	// add materials
	for ( std::set<std::string>::const_iterator iter = sceneMaterials.begin(); iter != sceneMaterials.end(); iter++ )
	{
		VuJsonContainer &material = materials.append();
		material["Name"].putValue(*iter);
		material["MaterialAsset"] = creationInfo[*iter];

		if ( !VuAssetBakery::IF()->doesAssetExist("VuMaterialAsset", material["MaterialAsset"].asString()) )
			material["MaterialAsset"].putValue(defaultMaterialAsset);
	}
}