//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rotate Motion entity
// 
//*****************************************************************************

#include "VuMotionEntity.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"


class VuRotateMotionEntity : public VuMotionEntity
{
	DECLARE_RTTI

public:
	VuRotateMotionEntity();

private:
	virtual void	onActivate();
	virtual void	onDeactivate();
	virtual void	onUpdate(float fdt);

	// properties
	VuVector3		mAxis;
	float			mSpeed;
	bool			mbLocal;

	VuMatrix		mOrigMatrix;
	float			mCurRot;
};


IMPLEMENT_RTTI(VuRotateMotionEntity, VuMotionEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuRotateMotionEntity);


//*****************************************************************************
VuRotateMotionEntity::VuRotateMotionEntity():
	mAxis(0,0,1),
	mSpeed(VU_PIDIV2),
	mbLocal(true),
	mOrigMatrix(VuMatrix::identity()),
	mCurRot(0)
{
	// properties
	addProperty(new VuVector3Property("Axis", mAxis));
	addProperty(new VuAngleProperty("Speed", mSpeed));
	addProperty(new VuBoolProperty("Local", mbLocal));
}

//*****************************************************************************
void VuRotateMotionEntity::onActivate()
{
	if ( VuTransformComponent *pTC = mpTargetMotionComponent->getOwnerEntity()->getTransformComponent() )
		mOrigMatrix = pTC->getWorldTransform();

	mAxis.normalize();

	mCurRot = 0;
}

//*****************************************************************************
void VuRotateMotionEntity::onDeactivate()
{
}

//*****************************************************************************
void VuRotateMotionEntity::onUpdate(float fdt)
{
	#define STEP_DELTA 0.001f // seconds

	mCurRot = VuModAngle(mCurRot + mSpeed*fdt);

	VuMatrix transform = mOrigMatrix;
	VuMatrix transformDelta = mOrigMatrix;

	if ( mbLocal )
	{
		transform.rotateAxisLocal(mAxis, mCurRot);
		transformDelta.rotateAxisLocal(mAxis, mCurRot + mSpeed*STEP_DELTA);
	}
	else
	{
		transform.rotateAxis(mAxis, mCurRot);
		transformDelta.rotateAxis(mAxis, mCurRot + mSpeed*STEP_DELTA);
	}

	VuVector3 vAngVel = (transformDelta.getEulerAngles() - transform.getEulerAngles())/STEP_DELTA;

	mpTargetMotionComponent->setWorldTransform(transform);
	mpTargetMotionComponent->setWorldAngularVelocity(vAngVel);
	mpTargetMotionComponent->setWorldLinearVelocity(VuVector3(0,0,0));
	mpTargetMotionComponent->update();
}
