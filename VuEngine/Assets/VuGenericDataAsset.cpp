//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic Data Asset class
// 
//*****************************************************************************

#include "VuGenericDataAsset.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuGenericDataAsset, VuAsset);


//*****************************************************************************
bool VuGenericDataAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
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

	return true;
}

//*****************************************************************************
bool VuGenericDataAsset::load(VuBinaryDataReader &reader)
{
	int dataSize;
	reader.readValue(dataSize);

	VuJsonBinaryReader jsonReader;
	if ( !jsonReader.loadFromMemory(mDataContainer, reader.cur(), dataSize) )
		return VUWARNING("Unable to load baked data asset %s: %s", getAssetName().c_str(), jsonReader.getLastError().c_str());

	reader.skip(dataSize);

	return true;
}

//*****************************************************************************
void VuGenericDataAsset::unload()
{
	mDataContainer.clear();
}
