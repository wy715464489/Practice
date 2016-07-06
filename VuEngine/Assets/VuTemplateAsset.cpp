//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Template Asset class
// 
//*****************************************************************************

#include "VuTemplateAsset.h"
#include "VuAssetUtil.h"


IMPLEMENT_RTTI(VuTemplateAsset, VuGenericDataAsset);
IMPLEMENT_ASSET_REGISTRATION(VuTemplateAsset);


//*****************************************************************************
void VuTemplateAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Projects/Templates");

	VuAssetUtil::addFileProperty(schema, "File", "vuprj");
}
