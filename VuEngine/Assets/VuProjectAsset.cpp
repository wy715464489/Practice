//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Project Asset class
// 
//*****************************************************************************

#include "VuProjectAsset.h"
#include "VuAssetUtil.h"
#include "VuTemplateAsset.h"
#include "VuAssetFactory.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


IMPLEMENT_RTTI(VuProjectAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuProjectAsset);


//*****************************************************************************
void VuProjectAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Projects");

	VuAssetUtil::addFileProperty(schema, "File", "vuprj");
}

//*****************************************************************************
bool VuProjectAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuJsonContainer dataContainer;

	VuJsonReader reader;
	if ( !reader.loadFromFile(dataContainer, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load raw data asset %s: %s", fileName.c_str(), reader.getLastError().c_str());
	int dataSize = VuJsonBinaryWriter::calculateDataSize(dataContainer);

	writer.writeValue(dataSize);

	VuJsonBinaryWriter jsonWriter;
	if ( !jsonWriter.saveToMemory(dataContainer, writer.allocate(dataSize), dataSize) )
		return VUWARNING("Unable to bake data asset %s.", fileName.c_str());

	std::string projectName = VuFileUtil::getName(fileName);
	writer.writeString(projectName);

	return true;
}

//*****************************************************************************
int VuProjectAsset::getAssetCount()
{
	int count = 0;

	const VuJsonContainer &assetData = mDataContainer["AssetData"];
	for ( int i = 0; i < assetData.size(); i++ )
		count += assetData[i].size() - 1;

	return count;
}

//*****************************************************************************
void VuProjectAsset::getAssetInfo(int index, std::string &assetType, std::string &assetName)
{
	int count = 0;

	const VuJsonContainer &assetData = mDataContainer["AssetData"];
	for ( int i = 0; i < assetData.size(); i++ )
	{
		const VuJsonContainer &typeData = assetData[i];
		int numAssets = typeData.size() - 1;
		if ( index >= count && index < count + numAssets )
		{
			assetType = typeData[0].asString();
			assetName = typeData[index - count + 1].asString();

			return;
		}

		count += numAssets;
	}
}

//*****************************************************************************
bool VuProjectAsset::load(VuBinaryDataReader &reader)
{
	int dataSize;
	reader.readValue(dataSize);

	VuJsonBinaryReader jsonReader;
	if ( !jsonReader.loadFromMemory(mDataContainer, reader.cur(), dataSize) )
		return VUWARNING("Unable to load baked data asset %s: %s", getAssetName().c_str(), jsonReader.getLastError().c_str());

	reader.skip(dataSize);

	reader.readString(mProjectName);

	return true;
}

//*****************************************************************************
void VuProjectAsset::unload()
{
	mDataContainer.clear();
}
