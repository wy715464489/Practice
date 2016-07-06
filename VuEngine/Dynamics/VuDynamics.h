//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics class
// 
//*****************************************************************************

#pragma once

#include "VuDynamicsCallbacks.h"
#include "Util/VuDynamicsUtil.h"
#include "VuRigidBody.h"
#include "VuCollisionTypes.h"
#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Method/VuMethod.h"


class VuEngine;
class VuVector3;
class VuCamera;
class VuColor;


class VuDynamics : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuDynamics)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init(bool asynchronous, int maxSubSteps, float fFixedTimeStep) = 0;

public:
	// world
	virtual void		setGravity(const VuVector3 &vGravity) = 0;
	virtual VuVector3	getGravity() = 0;

	virtual float		getLocalTime() = 0;

	// callbacks
	virtual void		registerRigidBodyCallback(VuRigidBodyCallback *pCallback) = 0;
	virtual void		unregisterRigidBodyCallback(VuRigidBodyCallback *pCallback) = 0;

	virtual void		registerStepCallback(VuDynamicsStepCallback *pCallback) = 0;
	virtual void		unregisterStepCallback(VuDynamicsStepCallback *pCallback) = 0;

	virtual void		registerContactCallback(VuGlobalContactCallback *pCallback) = 0;
	virtual void		unregisterContactCallback(VuGlobalContactCallback *pCallback) = 0;

	// adding/removing
	virtual void		addRigidBody(VuRigidBody *pRigidBody) = 0;
	virtual void		removeRigidBody(VuRigidBody *pRigidBody) = 0;
	virtual void		addConstraint(btTypedConstraint *pConstraint, bool disableCollisionsBetweenLinkedBodies = false) = 0;
	virtual void		removeConstraint(btTypedConstraint *pConstraint) = 0;

	// surface types
	typedef std::vector<std::string> SurfaceTypes;
	virtual const SurfaceTypes	&getSurfaceTypes() = 0;
	virtual int					getSurfaceTypeCount() = 0;
	virtual VUUINT8				getSurfaceTypeID(const char *strName) = 0;
	virtual float				getSurfaceFriction(VUUINT8 surfaceTypeID) = 0;
	virtual const VuColor		&getSurfaceColor(VUUINT8 surfaceTypeID) = 0;
	virtual const std::string	&getSurfaceTypeName(VUUINT8 surfaceTypeID) = 0;

	// asynchronous functionality
	virtual bool		isPotentiallyBusy() = 0;
	virtual bool		isBusy() = 0;
	virtual void		flush() = 0;

	// draw
	virtual void		drawCollision(const VuCamera &camera) = 0;

	// internal access
	virtual btDynamicsWorld	*getDynamicsWorld() = 0;
};
