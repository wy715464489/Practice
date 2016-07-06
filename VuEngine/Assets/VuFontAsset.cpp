//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Font Asset class
// 
//*****************************************************************************

#include "VuFontAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Gfx/Font/VuFont.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuFontAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuFontAsset);


//*****************************************************************************
void VuFontAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Fonts");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}

//*****************************************************************************
bool VuFontAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load font asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	// bake font
	if ( !VuFont::bake(doc["VuFont"], writer) )
		return VUWARNING("Unable to bake font %s.", fileName.c_str());

	return true;
}

//*****************************************************************************
bool VuFontAsset::load(VuBinaryDataReader &reader)
{
	mpFont = new VuFont;
	if ( !mpFont->load(reader) )
		return VUWARNING("Unable to load baked font %s.", getAssetName().c_str());

	return true;
}

//*****************************************************************************
void VuFontAsset::unload()
{
	delete mpFont;
	mpFont = VUNULL;
}
