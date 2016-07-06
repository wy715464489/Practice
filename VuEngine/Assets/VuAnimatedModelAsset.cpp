//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Model Asset class
// 
//*****************************************************************************

#include "VuAnimatedModelAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Animation/VuSkeleton.h"
#include "VuEngine/Gfx/GfxScene/VuGfxAnimatedScene.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Json/VuJsonReader.h"


IMPLEMENT_RTTI(VuAnimatedModelAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuAnimatedModelAsset);


//*****************************************************************************
void VuAnimatedModelAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Models");

	VuAssetUtil::addFileProperty(schema, "File", "json");

	VuAssetUtil::addMaterialAssignments(schema, VuFile::IF()->getRootPath() + creationInfo["File"].asString(), "DefaultAnimated");
}

//*****************************************************************************
bool VuAnimatedModelAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load animated model asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	// make sure this is an animated model
	if ( doc["VuAnimatedModel"].isNull() )
		return VUWARNING("%s was not exported as an animated model!", fileName.c_str());

	// create animated scene
	if ( !VuGfxAnimatedScene::bake(creationInfo, bakeParams, doc["VuAnimatedModel"], writer) )
		return VUWARNING("Unable to create animated model from %s", fileName.c_str());

	// create skeleton
	VuSkeleton *pSkeleton = new VuSkeleton;
	if ( !pSkeleton->load(doc["VuAnimatedModel"]["Skeleton"]) )
		return VUWARNING("Unable to create animated model sksleton from %s", fileName.c_str());

	// save to binary data
	pSkeleton->save(writer);

	// clean up
	pSkeleton->removeRef();

	return true;
}

//*****************************************************************************
bool VuAnimatedModelAsset::load(VuBinaryDataReader &reader)
{
	mpGfxAnimatedScene = new VuGfxAnimatedScene;
	if ( !mpGfxAnimatedScene->load(reader) )
	{
		unload();
		return false;
	}

	// load from binary data
	mpSkeleton = new VuSkeleton;
	mpSkeleton->load(reader);

	return true;
}

//*****************************************************************************
void VuAnimatedModelAsset::unload()
{
	if ( mpGfxAnimatedScene )
	{
		mpGfxAnimatedScene->removeRef();
		mpGfxAnimatedScene = VUNULL;
	}

	if ( mpSkeleton )
	{
		mpSkeleton->removeRef();
		mpSkeleton = VUNULL;
	}
}

//*****************************************************************************
void VuAnimatedModelAsset::editorReload()
{
	const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), getType(), getAssetName());
	VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
	if ( bake(creationInfo, bakeParams) )
	{
		VuBinaryDataReader reader(bakeParams.mData);

		mpGfxAnimatedScene->clear();
		if ( !mpGfxAnimatedScene->load(reader) )
			unload();
	}
}
