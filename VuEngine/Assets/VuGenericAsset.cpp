//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic Asset class
// 
//*****************************************************************************

#include "VuGenericAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


IMPLEMENT_RTTI(VuGenericAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuGenericAsset);


//*****************************************************************************
void VuGenericAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	VuAssetUtil::addFileProperty(schema, "File", "*");
}

//*****************************************************************************
bool VuGenericAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuArray<VUBYTE> data;
	if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, data) )
		return VUWARNING("Unable to load generic asset %s.", fileName.c_str());

	writer.writeArray(data);

	return true;
}

//*****************************************************************************
bool VuGenericAsset::load(VuBinaryDataReader &reader)
{
	reader.readArray(mData);

	return true;
}

//*****************************************************************************
void VuGenericAsset::unload()
{
	mData.deallocate();
}