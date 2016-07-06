//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OffsetAttachComponent class
// 
//*****************************************************************************

#include "VuOffsetAttachComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"


IMPLEMENT_RTTI(VuOffsetAttachComponent, VuAttachComponent);


//*****************************************************************************
VuOffsetAttachComponent::VuOffsetAttachComponent(VuEntity *pOwner):
	VuAttachComponent(pOwner)
{
}

//*****************************************************************************
VuOffsetAttachComponent::~VuOffsetAttachComponent()
{
}

//*****************************************************************************
bool VuOffsetAttachComponent::attach(VuMotionComponent *pMotionComponent, const VuMatrix &transform, const char *node)
{
	if ( !pMotionComponent->takeOwnership(getOwnerEntity()) )
		return false;

	VuAttachment attachment;
	attachment.mTransform = transform;
	attachment.mpMotionComponent = pMotionComponent;
	mAttachments.push_back(attachment);

	return true;
}

//*****************************************************************************
void VuOffsetAttachComponent::detach(VuMotionComponent *pMotionComponent)
{
	for ( int i = 0; i < mAttachments.size(); i++ )
	{
		if ( mAttachments[i].mpMotionComponent == pMotionComponent )
		{
			mAttachments.swap(i, mAttachments.size() - 1);
			mAttachments.pop_back();
			pMotionComponent->relinquishOwnership(getOwnerEntity());
		}
	}
}

//*****************************************************************************
void VuOffsetAttachComponent::update(const VuMatrix &transform, const VuVector3 &linVel, const VuVector3 &angVel)
{
	for ( int i = 0; i < mAttachments.size(); i++ )
	{
		VuAttachment &attachment = mAttachments[i];
		VuMotionComponent *pMC = attachment.mpMotionComponent;

		// calcualate world-space quantities
		VuMatrix worldTransform = attachment.mTransform*transform;
		VuVector3 worldLinVel = linVel + VuCross(angVel, attachment.mTransform.getTrans());
		VuVector3 worldAngVel = angVel;

		// update motion component
		pMC->setWorldTransform(worldTransform);
		pMC->setWorldLinearVelocity(worldLinVel);
		pMC->setWorldAngularVelocity(worldAngVel);
		pMC->update();
	}
}
