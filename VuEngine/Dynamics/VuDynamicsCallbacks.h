//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics callback classes
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"

class btCollisionObject;
class VuRigidBody;


//*****************************************************************************
// Callback for rigid bodies being added to/removed from world.
//*****************************************************************************
class VuRigidBodyCallback
{
public:
	virtual void	onRigidBodyAdded(const VuRigidBody *pRB)	{}
	virtual void	onRigidBodyRemoved(const VuRigidBody *pRB)	{}
};


//*****************************************************************************
// Callback used to hook into dynamics update.
//*****************************************************************************
class VuDynamicsStepCallback
{
public:
	virtual ~VuDynamicsStepCallback() {} 	// some compilers warn about deleting base class objects with a non-virtual destructor
	virtual void	onDynamicsAdvanceEnvironment(float fdt, bool bSimStep)	{}
	virtual void	onDynamicsApplyForces(float fdt)						{}
};


//*****************************************************************************
// Callbacks used to hook into contacts.
//*****************************************************************************
class VuContactPoint
{
public:
	const VuRigidBody	*mpBody0;
	const VuRigidBody	*mpBody1;
	const VuRigidBody	*mpOtherBody;
	VuVector3			mPosWorld;
	VuVector3			mNorWorld;	// global - normal on body 1, rigidbody - normal on other body
	VUUINT8				mSurfaceType0;
	VUUINT8				mSurfaceType1;
	float				mCombinedFriction;
};
class VuGlobalContactCallback
{
public:
	virtual void	onGlobalContactAdded(VuContactPoint &cp)	{}
};
class VuRigidBodyContactCallback
{
public:
	virtual bool	onRigidBodyContactAdded(VuContactPoint &cp)	{ return true; }
};
