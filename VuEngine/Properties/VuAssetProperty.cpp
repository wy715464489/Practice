//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset property class
// 
//*****************************************************************************

#include "VuAssetProperty.h"


//*****************************************************************************
VuAssetNameProperty::VuAssetNameProperty(const char *strAssetType, const char *strName, std::string &pValue):
	VuStringEnumProperty(strName, pValue),
	mstrAssetType(strAssetType)
{
}

//*****************************************************************************
int VuAssetNameProperty::getChoiceCount() const
{
	return (int)VuAssetFactory::IF()->getAssetNames(mstrAssetType).size();
}

//*****************************************************************************
const char *VuAssetNameProperty::getChoice(int index) const
{
	return VuAssetFactory::IF()->getAssetNames(mstrAssetType)[index].c_str();
}

//*****************************************************************************
VuBaseAssetProperty::VuBaseAssetProperty(const char *strAssetType, const char *strName, std::string &pValue):
	VuAssetNameProperty(strAssetType, strName, pValue),
	mpAsset(VUNULL)
{
}

//*****************************************************************************
VuBaseAssetProperty::~VuBaseAssetProperty()
{
	if ( mpAsset )
		VuAssetFactory::IF()->releaseAsset(mpAsset);
}

//*****************************************************************************
void VuBaseAssetProperty::onValueChanged()
{
	if ( mpAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpAsset);
		mpAsset = VUNULL;
	}

	if ( VuAssetFactory::IF()->doesAssetExist(mstrAssetType, mValue) )
		mpAsset = VuAssetFactory::IF()->createAsset(mstrAssetType, mValue);
}