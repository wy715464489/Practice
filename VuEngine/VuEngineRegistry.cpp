//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EngineRegistry class
// 
//*****************************************************************************

#include "VuEngineRegistry.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetPackFile.h"
#include "VuEngine/Entities/VuEntityFactory.h"


//*****************************************************************************
void VuEngineRegistry::addAssetTypes()
{
	REGISTER_ASSET(VuAnimatedModelAsset,     19, true);
	REGISTER_ASSET(VuAnimationAsset,          2, true);
	REGISTER_ASSET(VuAudioBankAsset,          2, true);
	REGISTER_ASSET(VuAudioProjectAsset,       1, true);
	REGISTER_ASSET(VuAudioStreamAsset,        2, false);
	REGISTER_ASSET(VuCollisionMaterialAsset,  3, true);
	REGISTER_ASSET(VuCollisionMeshAsset,     15, true);
	REGISTER_ASSET(VuCompiledShaderAsset,     9, true);
	REGISTER_ASSET(VuCubeTextureAsset,       20, true);
	REGISTER_ASSET(VuDBAsset,                 1, true);
	REGISTER_ASSET(VuFluidsMeshAsset,         5, true);
	REGISTER_ASSET(VuFontAsset,               3, true);
	REGISTER_ASSET(VuGenericAsset,            1, true);
	REGISTER_ASSET(VuLightMapAsset,           2, true);
	REGISTER_ASSET(VuMaterialAsset,          19, true);
	REGISTER_ASSET(VuPfxAsset,                8, true);
	REGISTER_ASSET(VuProjectAsset,            5, true);
	REGISTER_ASSET(VuSpreadsheetAsset,        2, true);
	REGISTER_ASSET(VuStaticModelAsset,       20, true);
	REGISTER_ASSET(VuStringAsset,             1, true);
	REGISTER_ASSET(VuTemplateAsset,           1, true);
	REGISTER_ASSET(VuTextureAsset,           26, true);
	REGISTER_ASSET(VuTimedEventAsset,         1, true);
	REGISTER_ASSET(VuWaterMapAsset,           3, true);
}
