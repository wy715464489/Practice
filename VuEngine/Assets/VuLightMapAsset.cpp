//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Light Map Asset class
// 
//*****************************************************************************

#include "VuLightMapAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuLightMapAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuLightMapAsset);

//*****************************************************************************
VuLightMapAsset::VuLightMapAsset():
	mRGB16(0)
{
}

//*****************************************************************************
void VuLightMapAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("LightMaps");

	VuAssetUtil::addFileProperty(schema, "File", "tga");
}

//*****************************************************************************
bool VuLightMapAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuTgaLoader tgaLoader;
	if ( tgaLoader.load(VuFile::IF()->getRootPath() + fileName) != VuTgaLoader::OK )
		return false;

	// convert to 16-bit rgb
	VuArray<VUBYTE> rgb16;
	if ( !VuImageUtil::convertToRGB565(tgaLoader, rgb16) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();

	writer.writeValue(width);
	writer.writeValue(height);

	// make sure dimensions are a power of 2 + 1
	if ( VuBitCount(width - 1) != 1 || VuBitCount(height - 1) != 1 )
		return VUWARNING("Water maps must be powers of 2 + 1");

	writer.writeData(&rgb16[0], rgb16.size());

	return true;
}

//*****************************************************************************
bool VuLightMapAsset::load(VuBinaryDataReader &reader)
{
	reader.readValue(mWidth);
	reader.readValue(mHeight);

	mRGB16.resize(mWidth*mHeight*2);
	reader.readData(&mRGB16[0], mRGB16.size());

	return true;
}

//*****************************************************************************
void VuLightMapAsset::unload()
{
	mWidth = 0;
	mHeight = 0;
	mRGB16.deallocate();
}
