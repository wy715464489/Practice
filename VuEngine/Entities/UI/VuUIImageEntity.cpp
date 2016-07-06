//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Image class
// 
//*****************************************************************************

#include "VuUIImageEntity.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"


IMPLEMENT_RTTI(VuUIImageEntity, VuUIImageBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIImageEntity);


//*****************************************************************************
VuUIImageEntity::VuUIImageEntity()
{
	// properties
	addProperty(mpTextureAssetProperty = new VuAssetProperty<VuTextureAsset>("Texture Asset", mTextureAssetName));
}

//*****************************************************************************
VuTexture *VuUIImageEntity::getTexture() const
{
	VuTextureAsset *pTextureAsset = mpTextureAssetProperty->getAsset();
	
	if(pTextureAsset)
	{
		return pTextureAsset->getTexture();
	}

	return VUNULL;
}