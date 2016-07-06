//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Material Asset class
// 
//*****************************************************************************

#include "VuCollisionMaterialAsset.h"
#include "VuAssetFactory.h"
#include "VuDBAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"


IMPLEMENT_RTTI(VuCollisionMaterialAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuCollisionMaterialAsset);

//*****************************************************************************
VuCollisionMaterialAsset::VuCollisionMaterialAsset()
{
}

//*****************************************************************************
void VuCollisionMaterialAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	VuJsonContainer surfaceTypes;
	{
		VuDBAsset *pSurfaceDB = VuAssetFactory::IF()->createAsset<VuDBAsset>("SurfaceDB");
		for ( int i = 0; i < pSurfaceDB->getDB().size(); i++ )
			surfaceTypes.append() = pSurfaceDB->getDB()[i]["Name"];
		VuAssetFactory::IF()->releaseAsset(pSurfaceDB);
	}

	VuAssetUtil::addEnumProperty(schema, "Surface Type", surfaceTypes, "<none>");
	VuAssetUtil::addBoolProperty(schema, "Corona Collision", false);
	VuAssetUtil::addFloatProperty(schema, "Hard Edge Threshold", 30.0f);
	VuAssetUtil::addBoolProperty(schema, "Receive Shadows", false);
	VuAssetUtil::addBoolProperty(schema, "Ignore Baked Shadows", false);
}

//*****************************************************************************
bool VuCollisionMaterialAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	// surface type
	std::string surfaceType = "<none>";
	VuDataUtil::getValue(creationInfo["Surface Type"], surfaceType);
	writer.writeString(surfaceType);

	// corona collision
	bool coronaCollision = false;
	VuDataUtil::getValue(creationInfo["Corona Collision"], coronaCollision);
	writer.writeValue(coronaCollision);
	
	// hard edge threshold
	float hardEdgeThreshold = 30.0f;
	VuDataUtil::getValue(creationInfo["Hard Edge Threshold"], hardEdgeThreshold);
	writer.writeValue(hardEdgeThreshold);

	// receive shadows
	bool receiveShadows = false;
	VuDataUtil::getValue(creationInfo["Receive Shadows"], receiveShadows);
	writer.writeValue(receiveShadows);

	// ignore baked shadows
	bool ignoreBakedShadows = false;
	VuDataUtil::getValue(creationInfo["Ignore Baked Shadows"], ignoreBakedShadows);
	writer.writeValue(ignoreBakedShadows);

	return true;
}

//*****************************************************************************
bool VuCollisionMaterialAsset::load(VuBinaryDataReader &reader)
{
	reader.readString(mSurfaceType);
	reader.readValue(mCoronaCollision);
	reader.readValue(mHardEdgeThreshold);
	reader.readValue(mReceiveShadows);
	reader.readValue(mIgnoreBakedShadows);

	return true;
}

//*****************************************************************************
void VuCollisionMaterialAsset::unload()
{
}
