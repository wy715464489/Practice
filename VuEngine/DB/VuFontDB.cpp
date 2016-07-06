//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Font DB class
// 
//*****************************************************************************

#include "VuFontDB.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Assets/VuFontAsset.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Dev/VuDev.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFontDB, VuFontDB);


//*****************************************************************************
bool VuFontDB::init()
{
	mpFontDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("FontDB");
	if ( mpFontDBAsset == VUNULL )
		return false;

	for ( int i = 0; i < mpFontDBAsset->getDB().numMembers(); i++ )
	{
		const std::string &name = mpFontDBAsset->getDB().getMemberKey(i);
		const VuJsonContainer &data = mpFontDBAsset->getDB()[name];

		VUUINT32 hash = VuHash::fnv32String(name.c_str());
		VuEntry &entry = mEntries[hash];

		// font asset
		const std::string &fontAssetName = data["FontAsset"].asString();
		if ( VuAssetFactory::IF()->doesAssetExist<VuFontAsset>(fontAssetName) )
			entry.mpFontAsset = VuAssetFactory::IF()->createAsset<VuFontAsset>(fontAssetName);

		// params
		if ( data["ForceUC"].asBool() ) entry.mParams.mFlags |= VuFontDrawParams::FORCE_UPPER_CASE;
		if ( data["ForceLC"].asBool() ) entry.mParams.mFlags |= VuFontDrawParams::FORCE_LOWER_CASE;
		VuDataUtil::getValue(data["Size"], entry.mParams.mSize);
		VuDataUtil::getValue(data["Weight"], entry.mParams.mWeight);
		VuDataUtil::getValue(data["Softness"], entry.mParams.mSoftness);
		VuDataUtil::getValue(data["Color"], entry.mParams.mColor);
		VuDataUtil::getValue(data["OutlineWeight"], entry.mParams.mOutlineWeight);
		VuDataUtil::getValue(data["OutlineSoftness"], entry.mParams.mOutlineSoftness);
		VuDataUtil::getValue(data["OutlineColor"], entry.mParams.mOutlineColor);
		VuDataUtil::getValue(data["TabSize"], entry.mParams.mTabSize);
		VuDataUtil::getValue(data["Slant"], entry.mParams.mSlant);
		VuDataUtil::getValue(data["Stretch"], entry.mParams.mStretch);
	}

	return true;
}

//*****************************************************************************
void VuFontDB::release()
{
	for ( Entries::iterator iter = mEntries.begin(); iter != mEntries.end(); iter++ )
		VuAssetFactory::IF()->releaseAsset(iter->second.mpFontAsset);
	mEntries.clear();

	VuAssetFactory::IF()->releaseAsset(mpFontDBAsset);
}

//*****************************************************************************
const VuFontDB::VuEntry &VuFontDB::getFont(const char *name) const
{
	VUUINT32 hash = VuHash::fnv32String(name);
	Entries::const_iterator iter = mEntries.find(hash);
	if ( iter != mEntries.end() )
		return iter->second;

	return mDefaultEntry;
}

//*****************************************************************************
int VuFontDB::getFontCount()
{
	return mpFontDBAsset->getMemberCount();
}

//*****************************************************************************
const char *VuFontDB::getFontName(int index)
{
	return mpFontDBAsset->getMemberKey(index);
}

//*****************************************************************************
void VuFontDB::reload()
{
	VuAssetFactory::IF()->forgetAsset<VuDBAsset>("FontDB");
	release();
	init();
}

//*****************************************************************************
VuFont *VuFontDB::VuEntry::font() const
{
	if ( mpFontAsset )
		return mpFontAsset->getFont();

	return VuDev::IF()->getFont();
}
