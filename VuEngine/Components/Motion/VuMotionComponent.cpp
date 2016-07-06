//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuMotionComponent class
// 
//*****************************************************************************

#include "VuMotionComponent.h"


IMPLEMENT_RTTI(VuMotionComponent, VuComponent);


//*****************************************************************************
VuMotionComponent::VuMotionComponent(VuEntity *pOwner, VuMotionComponentIF *pIF):
	VuComponent(pOwner),
	mpIF(pIF),
	mpControllingEntity(VUNULL),
	mTransform(VuMatrix::identity()),
	mLinearVelocity(0,0,0),
	mAngularVelocity(0,0,0)
{
}

//*****************************************************************************
VuMotionComponent::~VuMotionComponent()
{
}

//*****************************************************************************
bool VuMotionComponent::takeOwnership(VuEntity *pControllingEntity)
{
	if ( mpControllingEntity )
		return false;

	mpControllingEntity = pControllingEntity;
	mpIF->onMotionActivate();

	return true;
}

//*****************************************************************************
void VuMotionComponent::relinquishOwnership(VuEntity *pControllingEntity)
{
	VUASSERT(mpControllingEntity == pControllingEntity, "VuMotionComponent::relinquishOwnership() mismatch");

	mpControllingEntity = VUNULL;
	mpIF->onMotionDeactivate();

	mTransform.loadIdentity();
	mLinearVelocity.set(0,0,0);
	mAngularVelocity.set(0,0,0);
}
