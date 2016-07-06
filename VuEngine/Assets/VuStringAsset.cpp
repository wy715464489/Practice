//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String Asset class
// 
//*****************************************************************************

#include "VuStringAsset.h"
#include "VuAssetUtil.h"


IMPLEMENT_RTTI(VuStringAsset, VuGenericDataAsset);
IMPLEMENT_ASSET_REGISTRATION(VuStringAsset);


//*****************************************************************************
void VuStringAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Strings");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}
