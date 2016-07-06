//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AnimatedAttachComponent class
// 
//*****************************************************************************

#include "VuAnimatedAttachComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Gfx/Model/VuAnimatedModelInstance.h"
#include "VuEngine/Animation/VuSkeleton.h"


IMPLEMENT_RTTI(VuAnimatedAttachComponent, VuAttachComponent);


//*****************************************************************************
VuAnimatedAttachComponent::VuAnimatedAttachComponent(VuEntity *pOwner, const VuAnimatedModelInstance *pAnimatedModelInstance):
	VuAttachComponent(pOwner),
	mpAnimatedModelInstance(pAnimatedModelInstance)
{
}

//*****************************************************************************
VuAnimatedAttachComponent::~VuAnimatedAttachComponent()
{
}

//*****************************************************************************
bool VuAnimatedAttachComponent::attach(VuMotionComponent *pMotionComponent, const VuMatrix &transform, const char *node)
{
	if ( !pMotionComponent->takeOwnership(getOwnerEntity()) )
		return false;

	// determine bone index (if bone is not found, use root)
	int boneIndex = mpAnimatedModelInstance->getSkeleton()->getBoneIndex(node);
	boneIndex = VuMax(boneIndex, 0);

	VuAttachment attachment;
	attachment.mTransform = transform;
	attachment.mpMotionComponent = pMotionComponent;
	attachment.mBoneIndex = boneIndex;
	mAttachments.push_back(attachment);

	return true;
}

//*****************************************************************************
void VuAnimatedAttachComponent::detach(VuMotionComponent *pMotionComponent)
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
void VuAnimatedAttachComponent::update(const VuMatrix &transform, const VuVector3 &linVel, const VuVector3 &angVel)
{
	for ( int i = 0; i < mAttachments.size(); i++ )
	{
		VuAttachment &attachment = mAttachments[i];
		VuMotionComponent *pMC = attachment.mpMotionComponent;

		// get model-space transform/velocities for node
		VuMatrix modelTransform = mpAnimatedModelInstance->getModelMatrices()[attachment.mBoneIndex];
		VuVector3 modelLinVel = VuVector3(0,0,0); // todo: obtain/calculate model-space linear velocity
		VuVector3 modelAngVel = VuVector3(0,0,0); // todo: obtain/calculate model-space angular velocity

		// calculate model-space transform/velocities for attachment
		modelTransform = attachment.mTransform*modelTransform;
		modelLinVel = modelLinVel + VuCross(modelAngVel, attachment.mTransform.getTrans());

		// calcualate world-space velocity
		VuMatrix worldTransform = modelTransform*transform;
		VuVector3 worldLinVel = linVel + modelTransform.transformNormal(modelLinVel) + VuCross(angVel, modelTransform.getTrans());
		VuVector3 worldAngVel = angVel + modelTransform.transformNormal(modelAngVel);

		// update motion component
		pMC->setWorldTransform(worldTransform);
		pMC->setWorldLinearVelocity(worldLinVel);
		pMC->setWorldAngularVelocity(worldAngVel);
		pMC->update();
	}
}
