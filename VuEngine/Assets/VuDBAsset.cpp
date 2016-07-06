//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DB Asset class
// 
//*****************************************************************************

#include "VuDBAsset.h"
#include "VuAssetUtil.h"


IMPLEMENT_RTTI(VuDBAsset, VuGenericDataAsset);
IMPLEMENT_ASSET_REGISTRATION(VuDBAsset);


//*****************************************************************************
void VuDBAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("DBs");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}

//*****************************************************************************
bool VuDBAsset::load(VuBinaryDataReader &reader)
{
	if ( !VuGenericDataAsset::load(reader) )
		return false;

	if ( getDataContainer().isObject() )
		getDataContainer().getMemberKeys(mMemberKeys);

	return true;
}
