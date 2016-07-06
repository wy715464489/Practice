//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset class
// 
//*****************************************************************************

#include "VuAsset.h"
#include "VuAssetFactory.h"
#include "VuAssetBakery.h"


IMPLEMENT_RTTI_BASE(VuAsset);


//*****************************************************************************
void VuAsset::editorReload()
{
	if ( const VuAssetTypeInfo *pAssetTypeInfo = VuAssetFactory::IF()->getAssetTypeInfo(getType()) )
	{
		const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), getType(), getAssetName());
		VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
		if ( pAssetTypeInfo->mBakeFn(creationInfo, bakeParams) )
		{
			VuBinaryDataReader reader(bakeParams.mData);

			unload();
			load(reader);
		}
	}
}

//*****************************************************************************
VuAssetBakeParams::VuAssetBakeParams(const std::string &platform, const std::string &sku, const std::string &language):
	mPlatform(platform),
	mSku(sku),
	mLanguage(language),
	mData(0),
	mWriter(mData)
{
	mWriter.configure(platform);
}