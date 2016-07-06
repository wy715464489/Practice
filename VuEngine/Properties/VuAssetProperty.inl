//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset property inline implementation
// 
//*****************************************************************************

#include "VuEngine/Assets/VuAssetFactory.h"


//*****************************************************************************
template<class AssetType>
VuAssetProperty<AssetType>::VuAssetProperty(const char *strName, std::string &pValue):
	VuAssetNameProperty(AssetType::msRTTI.mstrType, strName, pValue),
	mpAsset(VUNULL)
{
}

//*****************************************************************************
template<class AssetType>
VuAssetProperty<AssetType>::~VuAssetProperty()
{
	if ( mpAsset )
		VuAssetFactory::IF()->releaseAsset(mpAsset);
}

//*****************************************************************************
template<class AssetType>
void VuAssetProperty<AssetType>::onValueChanged()
{
	if ( mpAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpAsset);
		mpAsset = VUNULL;
	}

	if ( VuAssetFactory::IF()->doesAssetExist<AssetType>(mValue) )
		mpAsset = VuAssetFactory::IF()->createAsset<AssetType>(mValue);
}