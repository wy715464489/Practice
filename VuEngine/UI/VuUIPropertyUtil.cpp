//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Helpers for complex UI entity properties.
// 
//*****************************************************************************

#include "VuUIPropertyUtil.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"


//*****************************************************************************
VuStaticIntEnumProperty::Choice VuUIStringFormatProperties::sOptAlignH[] =
{
	{ "left", VuStringFormat::ALIGN_LEFT },
	{ "center", VuStringFormat::ALIGN_CENTER },
	{ "right", VuStringFormat::ALIGN_RIGHT },
	{ "right-ea-left", VuStringFormat::ALIGN_RIGHT_EA_LEFT },
	{ VUNULL }
};
VuStaticIntEnumProperty::Choice VuUIStringFormatProperties::sOptAlignV[] =
{
	{ "top", VuStringFormat::ALIGN_TOP },
	{ "center", VuStringFormat::ALIGN_CENTER },
	{ "bottom", VuStringFormat::ALIGN_BOTTOM },
	{ "baseline", VuStringFormat::ALIGN_BASELINE },
	{ VUNULL }
};

//*****************************************************************************
VuStaticIntEnumProperty::Choice VuUIAnchorProperties::sOptAnchorH[] =
{
	{ "none", VuUIAnchor::ANCHOR_NONE },
	{ "left", VuUIAnchor::ANCHOR_LEFT },
	{ "right", VuUIAnchor::ANCHOR_RIGHT },
	{ "left-right", VuUIAnchor::ANCHOR_LEFT_RIGHT },
	{ VUNULL }
};
VuStaticIntEnumProperty::Choice VuUIAnchorProperties::sOptAnchorV[] =
{
	{ "none", VuUIAnchor::ANCHOR_NONE },
	{ "top", VuUIAnchor::ANCHOR_TOP },
	{ "bottom", VuUIAnchor::ANCHOR_BOTTOM },
	{ "top-bottom", VuUIAnchor::ANCHOR_TOP_BOTTOM },
	{ VUNULL }
};

//*****************************************************************************
VuUIImageProperties::VuUIImageProperties():
	mpTextureAssetProperty(VUNULL)
{
}

//*****************************************************************************
void VuUIImageProperties::addProperties(VuProperties &properties, const char *strName)
{
	properties.add(mpTextureAssetProperty = new VuAssetProperty<VuTextureAsset>(strName, mTextureAssetName));
}

//*****************************************************************************
VuTextureAsset *VuUIImageProperties::getTextureAsset() const
{
	if ( mpTextureAssetProperty )
		return mpTextureAssetProperty->getAsset();

	return VUNULL;
}

//*****************************************************************************
VuTexture *VuUIImageProperties::getTexture() const
{
	if ( VuTextureAsset *pTextureAsset = getTextureAsset() )
		return pTextureAsset->getTexture();

	return VUNULL;
}
