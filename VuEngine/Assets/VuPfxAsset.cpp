//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Asset class
// 
//*****************************************************************************

#include "VuPfxAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxGroup.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuFastContainer.h"


IMPLEMENT_RTTI(VuPfxAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuPfxAsset);


//*****************************************************************************
void VuPfxAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Pfx");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}

//*****************************************************************************
bool VuPfxAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	const std::string &fileName = creationInfo["File"].asString();

	VuJsonContainer data;
	VuJsonReader reader;
	if ( !reader.loadFromFile(data, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load pfx asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	VuFastContainer::serialize(data, bakeParams.mWriter);

	return true;
}

//*****************************************************************************
bool VuPfxAsset::load(VuBinaryDataReader &reader)
{
	if ( VuPfx::IF() )
	{
		int size = reader.remaining();

		const VuFastContainer *pFastContainer = VuFastContainer::createInPlace(reader.cur());
		bool success = VuPfx::IF()->addProject(getAssetName().c_str(), *pFastContainer);

		if ( !success )
			return false;
	}

	return true;
}

//*****************************************************************************
void VuPfxAsset::unload()
{
	if ( VuPfxManager::IF() )
	{
		VuPfxManager::IF()->killAllEntities();
	}

	if ( VuPfx::IF() )
	{
		VuPfx::IF()->removeProject(getAssetName().c_str());
	}
}
