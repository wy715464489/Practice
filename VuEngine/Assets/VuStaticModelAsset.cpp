//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Model Asset class
// 
//*****************************************************************************

#include "VuStaticModelAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Gfx/GfxScene/VuGfxStaticScene.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Json/VuJsonReader.h"


IMPLEMENT_RTTI(VuStaticModelAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuStaticModelAsset);


//*****************************************************************************
bool VuStaticModelAsset::substitute(const VuAsset *pSubstAsset)
{
	// make sure type matches
	if ( !pSubstAsset->isDerivedFrom(msRTTI) )
		return false;

	const VuStaticModelAsset *pSubstStaticModelAsset = static_cast<const VuStaticModelAsset *>(pSubstAsset);

	VuGfxSort::IF()->flush();

	unload();

	if ( (mpGfxStaticScene = pSubstStaticModelAsset->getGfxStaticScene()) != VUNULL)
		mpGfxStaticScene->addRef();

	return true;
}

//*****************************************************************************
void VuStaticModelAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Models");

	VuAssetUtil::addFileProperty(schema, "File", "json");
	VuAssetUtil::addBoolProperty(schema, "FlipX", false, "Flip mesh on X-Axis");

	VuAssetUtil::addMaterialAssignments(schema, VuFile::IF()->getRootPath() + creationInfo["File"].asString(), "Default");
}

//*****************************************************************************
bool VuStaticModelAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();
	bool flipX = creationInfo["FlipX"].asBool();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load static model asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	// make sure this is a static model
	if ( doc["VuGfxScene"].isNull() )
		return VUWARNING("%s was not exported as a static model!", fileName.c_str());

	// create scene
	if ( !VuGfxStaticScene::bake(creationInfo, bakeParams, doc["VuGfxScene"], flipX, writer) )
		return VUWARNING("Unable to create static model from %s", fileName.c_str());

	return true;
}

//*****************************************************************************
bool VuStaticModelAsset::load(VuBinaryDataReader &reader)
{
	// load from binary data
	mpGfxStaticScene = new VuGfxStaticScene;
	if ( !mpGfxStaticScene->load(reader) )
	{
		unload();
		return false;
	}

	return true;
}

//*****************************************************************************
void VuStaticModelAsset::unload()
{
	if ( mpGfxStaticScene )
	{
		mpGfxStaticScene->removeRef();
		mpGfxStaticScene = VUNULL;
	}
}

//*****************************************************************************
void VuStaticModelAsset::editorReload()
{
	const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), getType(), getAssetName());
	VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
	if ( bake(creationInfo, bakeParams) )
	{
		VuBinaryDataReader reader(bakeParams.mData);

		mpGfxStaticScene->clear();
		if ( !mpGfxStaticScene->load(reader) )
			unload();
	}
}
