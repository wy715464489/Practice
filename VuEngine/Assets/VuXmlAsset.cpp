//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xml Asset class
// 
//*****************************************************************************

#include "VuXmlAsset.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


IMPLEMENT_RTTI(VuXmlAsset, VuAsset);


//*****************************************************************************
bool VuXmlAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load file
	VuArray<VUBYTE> fileData;
	if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, fileData) )
		return VUWARNING("Unable to load Xml file '%s'.", fileName.c_str());

	writer.writeValue(fileData.size());
	writer.writeData(&fileData[0], fileData.size());

	return true;
}

//*****************************************************************************
bool VuXmlAsset::load(VuBinaryDataReader &reader)
{
	int dataSize;
	reader.readValue(dataSize);

	// don't condense white space
	TiXmlBase::SetCondenseWhiteSpace(false);

	// parse document
	if ( !mXmlDocument.Parse(reinterpret_cast<const char*>(reader.cur()), VUNULL, TIXML_ENCODING_UTF8) )
		return VUWARNING("Unable to load baked Xml asset '%s' (%s).", getAssetName().c_str(), mXmlDocument.ErrorDesc());

	reader.skip(dataSize);

	return true;
}

//*****************************************************************************
void VuXmlAsset::unload()
{
	mXmlDocument.Clear();
}
