//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawComponent class
// 
//*****************************************************************************

#include "Vu3dDrawComponent.h"
#include "Vu3dDrawManager.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


IMPLEMENT_RTTI(Vu3dDrawComponent, VuComponent);


//*****************************************************************************
Vu3dDrawComponent::Vu3dDrawComponent(VuEntity *pOwnerEntity, bool bReflectDefault) : VuComponent(pOwnerEntity),
	mpDrawMethod(VUNULL),
	mpDrawShadowMethod(VUNULL),
	mpDrawPrefetchMethod(VUNULL),
	mbDrawing(true),
	mbReflecting(bReflectDefault),
	mbShadowing(true),
	mbRegistered(false),
	mpDbvtNode(VUNULL),
	mAabb(VuVector3(0, 0, 0), VuVector3(0, 0, 0)),
	mZoneBits(1<<0)
{
	// add properties
	addProperty(new VuBoolProperty("Draw", mbDrawing));
	addProperty(new VuBoolProperty("Reflect", mbReflecting));
	addProperty(new VuBoolProperty("Shadow", mbShadowing));
}

//*****************************************************************************
Vu3dDrawComponent::~Vu3dDrawComponent()
{
	if ( mpDrawPrefetchMethod && Vu3dDrawManager::IF() )
		Vu3dDrawManager::IF()->removePrefetchMethod(mpDrawPrefetchMethod);

	hide();

	delete mpDrawMethod;
	delete mpDrawShadowMethod;
	delete mpDrawPrefetchMethod;
}

//*****************************************************************************
void Vu3dDrawComponent::show()
{
	if ( !mbRegistered )
	{
		Vu3dDrawManager::IF()->add(this);

		mbRegistered = true;
	}
}

//*****************************************************************************
void Vu3dDrawComponent::hide()
{
	if ( mbRegistered )
	{
		Vu3dDrawManager::IF()->remove(this);

		mbRegistered = false;
	}
}

//*****************************************************************************
void Vu3dDrawComponent::updateVisibility(const VuAabb &aabb)
{
	mAabb = aabb;

	if ( mbRegistered )
		Vu3dDrawManager::IF()->updateVisibility(this);
}

//*****************************************************************************
void Vu3dDrawComponent::updateVisibility(const VuAabb &aabb, const VuMatrix &transform)
{
	updateVisibility(VuAabb(aabb, transform));
}
