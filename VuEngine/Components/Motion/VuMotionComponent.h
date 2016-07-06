//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MotionComponent class
// 
//*****************************************************************************

#pragma once

#include "VuMotionComponentIF.h"
#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Method/VuMethod.h"


class VuMotionComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Motion)
	DECLARE_RTTI

public:
	VuMotionComponent(VuEntity *pOwner, VuMotionComponentIF *pIF);
	~VuMotionComponent();

	// accessors used by moving entity
	const VuMatrix	&getWorldTransform()		{ return mTransform; }
	const VuVector3	&getWorldLinearVelocity()	{ return mLinearVelocity; }
	const VuVector3	&getWorldAngularVelocity()	{ return mAngularVelocity; }

	// used by VuMotionEntity
	bool			takeOwnership(VuEntity *pControllingEntity);
	void			relinquishOwnership(VuEntity *pControllingEntity);

	// used by motion implementations derived from VuMotionEntity
	void				update()													{ mpIF->onMotionUpdate(); }
	void				setWorldTransform(const VuMatrix &transform)				{ mTransform = transform; }
	void				setWorldLinearVelocity(const VuVector3 &linearVelocity)		{ mLinearVelocity = linearVelocity; }
	void				setWorldAngularVelocity(const VuVector3 &angularVelocity)	{ mAngularVelocity = angularVelocity; }

protected:
	VuMotionComponentIF	*mpIF;
	VuEntity			*mpControllingEntity;
	VuMatrix			mTransform;
	VuVector3			mLinearVelocity;
	VuVector3			mAngularVelocity;
};
