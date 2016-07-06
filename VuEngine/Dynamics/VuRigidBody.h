//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rigid Body class
// 
//*****************************************************************************

#pragma once

#include "VuDynamics.h"

class VuEntity;


class VuRigidBody : public btRigidBody
{
public:
	VuRigidBody(const btRigidBody::btRigidBodyConstructionInfo &info, VuEntity *pEntity, VUINT16 collisionGroup, VUINT16 collisionMask);

	void						setContactCallback(VuRigidBodyContactCallback *pCB)	{ mpContactCallback = pCB; }
	void						setSurfaceType(VUUINT8 surfaceType)					{ mSurfaceType = surfaceType; }
	void						setSurfaceType(const char *surfaceType);
	void						setCollisionGroup(VUINT16 group);
	void						setCollisionMask(VUINT16 mask);
	void						setExtendedFlags(VUUINT32 flags)	{ mExtendedFlags = flags; }

	VuEntity					*getEntity() const			{ return mpEntity; }
	VuRigidBodyContactCallback	*getContactCallback() const	{ return mpContactCallback; }
	VUUINT8						getSurfaceType() const		{ return mSurfaceType; }
	VUINT16						getCollisionGroup() const	{ return mCollisionGroup; }
	VUINT16						getCollisionMask() const	{ return mCollisionMask; }
	VUUINT32					getExtendedFlags() const	{ return mExtendedFlags; }

	const VuVector3				&getVuLinearVelocity() const		{ return (const VuVector3 &)getLinearVelocity(); }
	const VuVector3				&getVuAngularVelocity() const		{ return (const VuVector3 &)getAngularVelocity(); }
	const VuVector3				&getVuCenterOfMassPosition() const	{ return (const VuVector3 &)getCenterOfMassPosition(); }
	VuMatrix					getVuCenterOfMassTransform() const	{ return VuDynamicsUtil::toVuMatrix(getCenterOfMassTransform()); }

	void						setVuLinearVelocity(const VuVector3 &linVel)	{ setLinearVelocity(VuDynamicsUtil::toBtVector3(linVel)); }
	void						setVuAngularVelocity(const VuVector3 &angVel)	{ setAngularVelocity(VuDynamicsUtil::toBtVector3(angVel)); }

	void						setShadowValues(const VUUINT8 *pShadowValues) { mpShadowValues = pShadowValues; }
	const VUUINT8				*getShadowValues() const { return mpShadowValues; }

private:
	VuEntity					*mpEntity;
	VuRigidBodyContactCallback	*mpContactCallback;
	VUUINT8						mSurfaceType;
	VUINT16						mCollisionGroup;
	VUINT16						mCollisionMask;
	VUUINT32					mExtendedFlags;
	const VUUINT8				*mpShadowValues;
};
