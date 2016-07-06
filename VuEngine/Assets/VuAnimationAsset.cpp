//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Asset class
// 
//*****************************************************************************

#include "VuAnimationAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Animation/VuAnimation.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Json/VuJsonReader.h"


IMPLEMENT_RTTI(VuAnimationAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuAnimationAsset);


//*****************************************************************************
void VuAnimationAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Models");

	VuAssetUtil::addFileProperty(schema, "File", "json");
	VuAssetUtil::addBoolProperty(schema, "Additive", false);
}

//*****************************************************************************
bool VuAnimationAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load animated model asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	// make sure this is an animation
	if ( doc["VuAnimation"].isNull() )
		return VUWARNING("%s was not exported as an animation!", fileName.c_str());

	// create animation
	VuAnimation *pAnimation = new VuAnimation;
	if ( !pAnimation->load(doc["VuAnimation"], creationInfo["Additive"].asBool()) )
		return VUWARNING("Unable to create animation from %s", fileName.c_str());

	// save to binary data
	pAnimation->save(writer);
	pAnimation->removeRef();

	return true;
}

//*****************************************************************************
bool VuAnimationAsset::load(VuBinaryDataReader &reader)
{
	// load from binary data
	mpAnimation = new VuAnimation;
	mpAnimation->load(reader);

	return true;
}

//*****************************************************************************
void VuAnimationAsset::unload()
{
	if ( mpAnimation )
	{
		mpAnimation->removeRef();
		mpAnimation = VUNULL;
	}
}
