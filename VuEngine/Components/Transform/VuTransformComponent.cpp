//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TransformComponent class
// 
//*****************************************************************************

#include "VuTransformComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"


IMPLEMENT_RTTI(VuTransformComponent, VuComponent);


//*****************************************************************************
VuTransformComponent::VuTransformComponent(VuEntity *pOwner) : VuComponent(pOwner),
	mLocalTransform(VuMatrix::identity()),
	mWorldTransform(VuMatrix::identity()),
	mLocalRotation(0,0,0),
	mWorldRotation(0,0,0),
	mLocalScale(1,1,1),
	mWorldScale(1,1,1),
	mpWatcher(VUNULL),
	mMask((VUUINT32)~0)
{
	addProperties();
}

//*****************************************************************************
VuTransformComponent::~VuTransformComponent()
{
	delete mpWatcher;
}

//*****************************************************************************
void VuTransformComponent::onPostLoad()
{
	calcTransformFromEulerPos(mLocalTransform, mLocalRotation, mLocalTransform.getTrans());
	recalcWorldTransform();
	recalcWorldScale();
	updateChildren(true);
}

//*****************************************************************************
void VuTransformComponent::onGameReset()
{
	calcTransformFromEulerPos(mLocalTransform, mLocalRotation, mLocalTransform.getTrans());
	recalcWorldTransform();
	recalcWorldScale();
	updateChildren(true);
}

//*****************************************************************************
void VuTransformComponent::setLocalTransform(const VuMatrix &xform, bool notify)
{
	mLocalTransform = xform;

	// recalculate euler angles
	mLocalRotation = mLocalTransform.getEulerAngles();

	recalcWorldTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setLocalTransform(const VuVector3 &pos, const VuVector3 &rot, bool notify)
{
	mLocalTransform.setTrans(pos);
	mLocalRotation = rot;

	calcTransformFromEulerPos(mLocalTransform, mLocalRotation, mLocalTransform.getTrans());

	recalcWorldTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setLocalPosition(const VuVector3 &pos, bool notify)
{
	mLocalTransform.setTrans(pos);

	recalcWorldTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setLocalRotation(const VuVector3 &rot, bool notify)
{
	mLocalRotation = rot;

	calcTransformFromEulerPos(mLocalTransform, mLocalRotation, mLocalTransform.getTrans());

	recalcWorldTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setLocalScale(const VuVector3 &scale, bool notify)
{
	mLocalScale = scale;

	recalcWorldScale();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setWorldTransform(const VuMatrix &xform, bool notify)
{
	mWorldTransform = xform;

	// recalculate euler angles
	mWorldRotation = mWorldTransform.getEulerAngles();

	recalcLocalTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setWorldTransform(const VuVector3 &pos, const VuVector3 &rot, bool notify)
{
	mWorldTransform.setTrans(pos);
	mWorldRotation = rot;

	calcTransformFromEulerPos(mWorldTransform, mWorldRotation, mWorldTransform.getTrans());

	recalcLocalTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setWorldPosition(const VuVector3 &pos, bool notify)
{
	mWorldTransform.setTrans(pos);

	recalcLocalTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setWorldRotation(const VuVector3 &rot, bool notify)
{
	mWorldRotation = rot;

	calcTransformFromEulerPos(mWorldTransform, mWorldRotation, mWorldTransform.getTrans());

	recalcLocalTransform();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::setWorldScale(const VuVector3 &scale, bool notify)
{
	mWorldScale = scale;

	recalcLocalScale();
	updateChildren(notify);

	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::addProperties()
{
	addProperty(new VuVector3Property("Position", (VuVector3 &)mLocalTransform.mT)) -> setWatcher(this, &VuTransformComponent::propertiesModified);
	addProperty(new VuRotation3dProperty("Rotation", mLocalRotation)) -> setWatcher(this, &VuTransformComponent::propertiesModified);
	addProperty(new VuVector3Property("Scale", mLocalScale)) -> setWatcher(this, &VuTransformComponent::propertiesModified);
}

//*****************************************************************************
void VuTransformComponent::propertiesModified()
{
	calcTransformFromEulerPos(mLocalTransform, mLocalRotation, mLocalTransform.getTrans());
	recalcWorldTransform();
	recalcWorldScale();
	updateChildren(true);
	notifyWatcher();
}

//*****************************************************************************
void VuTransformComponent::notifyWatcher()
{
	if ( mpWatcher )
		mpWatcher->execute();
}

//*****************************************************************************
void VuTransformComponent::recalcLocalTransform()
{
	if ( getOwnerEntity()->getParentEntity() )
	{
		VuMatrix invParentTransform = getOwnerEntity()->getParentEntity()->getTransformComponent()->getWorldTransform();
		invParentTransform.fastInvert();
		mLocalTransform = mWorldTransform*invParentTransform;
		mLocalRotation = mLocalTransform.getEulerAngles();
	}
	else
	{
		mLocalTransform = mWorldTransform;
		mLocalRotation = mWorldRotation;
	}
}

//*****************************************************************************
void VuTransformComponent::recalcWorldTransform()
{
	if ( getOwnerEntity()->getParentEntity() )
	{
		mWorldTransform = mLocalTransform*getOwnerEntity()->getParentEntity()->getTransformComponent()->getWorldTransform();
		mWorldRotation = mWorldTransform.getEulerAngles();
	}
	else
	{
		mWorldTransform = mLocalTransform;
		mWorldRotation = mLocalRotation;
	}
}

//*****************************************************************************
void VuTransformComponent::recalcLocalScale()
{
	if ( getOwnerEntity()->getParentEntity() )
	{
		mLocalScale = mWorldScale/getOwnerEntity()->getParentEntity()->getTransformComponent()->getWorldScale();
	}
	else
	{
		mLocalScale = mWorldScale;
	}
}

//*****************************************************************************
void VuTransformComponent::recalcWorldScale()
{
	if ( getOwnerEntity()->getParentEntity() )
	{
		mWorldScale = mLocalScale*getOwnerEntity()->getParentEntity()->getTransformComponent()->getWorldScale();
	}
	else
	{
		mWorldScale = mLocalScale;
	}
}

//*****************************************************************************
void VuTransformComponent::updateChildren(bool notify)
{
	for ( int i = 0; i < getOwnerEntity()->getChildEntityCount(); i++ )
	{
		VuTransformComponent *pChildTC = getOwnerEntity()->getChildEntity(i)->getTransformComponent();

		pChildTC->mWorldTransform = pChildTC->mLocalTransform*mWorldTransform;
		pChildTC->mWorldRotation = pChildTC->mWorldTransform.getEulerAngles();
		pChildTC->mWorldScale = pChildTC->mLocalScale*mWorldScale;

		if ( notify )
			pChildTC->notifyWatcher();

		pChildTC->updateChildren(notify);
	}
}

//*****************************************************************************
void VuTransformComponent::calcTransformFromEulerPos(VuMatrix &transform, const VuVector3 &euler, VuVector3 pos)
{
	transform.loadIdentity();
	transform.rotateXYZ(euler);
	transform.setTrans(pos);
}