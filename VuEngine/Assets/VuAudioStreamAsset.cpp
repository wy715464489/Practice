//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Asset class
// 
//*****************************************************************************

#include "VuAudioStreamAsset.h"
#include "VuAssetUtil.h"


IMPLEMENT_RTTI(VuAudioStreamAsset, VuGenericAsset);
IMPLEMENT_ASSET_REGISTRATION(VuAudioStreamAsset);


//*****************************************************************************
void VuAudioStreamAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Audio");

	VuAssetUtil::addFileProperty(schema, "File", "");
}

