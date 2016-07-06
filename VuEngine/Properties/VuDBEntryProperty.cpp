//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DBEntry property class
// 
//*****************************************************************************

#include "VuDBEntryProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Util/VuHashedString.h"


//*****************************************************************************
VuDBEntryProperty::VuDBEntryProperty(const char *strName, std::string &pValue, const char *strDBName):
	VuStringEnumProperty(strName, pValue)
{
	mpDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>(strDBName);
}

//*****************************************************************************
VuDBEntryProperty::~VuDBEntryProperty()
{
	VuAssetFactory::IF()->releaseAsset(mpDBAsset);
}

//*****************************************************************************
int VuDBEntryProperty::getChoiceCount() const
{
	if ( mpDBAsset )
		return mpDBAsset->getMemberCount();
	return 0;
}

//*****************************************************************************
const char *VuDBEntryProperty::getChoice(int index) const
{
	if ( mpDBAsset )
		return mpDBAsset->getMemberKey(index);
	return VUNULL;
}

//*****************************************************************************
const VuJsonContainer &VuDBEntryProperty::getEntryData() const
{
	return getDB()[mValue];
}

//*****************************************************************************
const VuJsonContainer &VuDBEntryProperty::getDB() const
{
	if ( mpDBAsset )
		return mpDBAsset->getDB();
	return VuJsonContainer::null;
}

//*****************************************************************************
void VuDBEntryProperty::reloadDB()
{
	VuDBAsset *pOldDBAsset = mpDBAsset;
	mpDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>(pOldDBAsset->getAssetName());
	VuAssetFactory::IF()->releaseAsset(pOldDBAsset);
}
