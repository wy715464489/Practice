//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  InstigatorComponent class
// 
//*****************************************************************************

#include "VuInstigatorComponent.h"
#include "VuEngine/Managers/VuTriggerManager.h"


IMPLEMENT_RTTI(VuInstigatorComponent, VuComponent);


//*****************************************************************************
VuInstigatorComponent::VuInstigatorComponent(VuEntity *pOwnerEntity) : VuComponent(pOwnerEntity),
	mOffset(0,0,0),
	mRadius(0),
	mbRegistered(false)
{
}

//*****************************************************************************
VuInstigatorComponent::~VuInstigatorComponent()
{
	disable();
}

//*****************************************************************************
void VuInstigatorComponent::setMask(VUUINT32 mask)
{
	if ( mbRegistered )
		VuTriggerManager::IF()->removeInstigator(this);

	mMask = mask;

	if ( mbRegistered )
		VuTriggerManager::IF()->addInstigator(this);
}

//*****************************************************************************
void VuInstigatorComponent::enable()
{
	if ( !mbRegistered )
		VuTriggerManager::IF()->addInstigator(this);

	mbRegistered = true;
}

//*****************************************************************************
void VuInstigatorComponent::disable()
{
	if ( mbRegistered )
		VuTriggerManager::IF()->removeInstigator(this);

	mbRegistered = false;
}

//*****************************************************************************
void VuInstigatorComponent::snap()
{
	if ( mbRegistered )
		VuTriggerManager::IF()->snapInstigator(this);
}