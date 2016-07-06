//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Entity
// 
//*****************************************************************************

#include "VuPfxEntity.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Pfx/VuPfx.h"


IMPLEMENT_RTTI(VuPfxEntity, VuEntity);


//*****************************************************************************
VuPfxEntity::VuPfxEntity(): VuEntity(IGNORE_REPOSITORY),
	mpPfxSystemInstance(VUNULL),
	mHandleSlot(0),
	mHandleCount(0)
{
	// components
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	mp3dDrawComponent->setDrawMethod(this, &VuPfxEntity::draw);
	mp3dDrawComponent->setDrawShadowMethod(this, &VuPfxEntity::drawShadow);
}

//*****************************************************************************
void VuPfxEntity::onGameInitialize()
{
	// set defaults
	enableReflection(false);
	enableShadow(false);
}

//*****************************************************************************
void VuPfxEntity::onGameRelease()
{
	mp3dDrawComponent->hide();
}

//*****************************************************************************
void VuPfxEntity::enableReflection(bool bEnable)
{
	mp3dDrawComponent->enableReflection(bEnable);
}

//*****************************************************************************
void VuPfxEntity::enableShadow(bool bEnable)
{
	mp3dDrawComponent->enableShadow(bEnable);
}

//*****************************************************************************
void VuPfxEntity::draw(const VuGfxDrawParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->draw(params);
}

//*****************************************************************************
void VuPfxEntity::drawShadow(const VuGfxDrawShadowParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->drawShadow(params);
}
