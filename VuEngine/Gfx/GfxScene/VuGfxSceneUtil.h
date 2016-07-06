//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxScene utility
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"


namespace VuGfxSceneUtil
{
	typedef std::set<std::string> Names;

	void	optimizeVertexDeclaration(const std::string &platform, bool bSkinning, const VuVertexDeclarationElements &elements, VuVertexDeclarationElements &optElements);
	void	gatherSceneMaterialNames(const VuJsonContainer &scene, Names &names);
	void	cleanUpMaterials(const VuJsonContainer &creationInfo, const VuJsonContainer &scene, VuJsonContainer &materials, const std::string &defaultMaterialAsset);
}