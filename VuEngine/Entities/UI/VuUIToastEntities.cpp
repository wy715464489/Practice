//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Toast entities
// 
//*****************************************************************************

#include "VuEngine/Entities/UI/VuUITextBaseEntity.h"
#include "VuEngine/Entities/UI/VuUIImageBaseEntity.h"
#include "VuEngine/Managers/VuToastManager.h"
#include "VuEngine/Assets/VuTextureAsset.h"


//*****************************************************************************
// VuUIToastImageEntity
//*****************************************************************************

class VuUIToastImageEntity : public VuUIImageBaseEntity
{
	DECLARE_RTTI

public:
	virtual VuTexture *getTexture() const;
};

IMPLEMENT_RTTI(VuUIToastImageEntity, VuUITextBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIToastImageEntity);


//*****************************************************************************
VuTexture *VuUIToastImageEntity::getTexture() const
{
	if ( VuToastManager::IF() )
	{
		VuToast *pToast = VuToastManager::IF()->getActiveToast();
		if ( pToast && pToast->mpImage )
			return pToast->mpImage->getTexture();
	}

	return VUNULL;
}
