//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics world implementation
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Dynamics/VuDynamics.h"


class VuDynamicsWorldImpl : public btDiscreteDynamicsWorld
{
public:
	VuDynamicsWorldImpl(btDispatcher *dispatcher, btBroadphaseInterface *pairCache, btConstraintSolver *constraintSolver, btCollisionConfiguration *collisionConfiguration);
	~VuDynamicsWorldImpl();

	// callback
	void					setCallback(VuDynamicsStepCallback *pCallback) { mpCallback = pCallback; }

	virtual int				stepSimulation(btScalar timeStep, int maxSubSteps = 1, btScalar fixedTimeStep = btScalar(1.)/btScalar(60.));

	const btVector3			&getGravity() { return m_gravity; }
	float					getLocalTime() { return m_localTime; }

	virtual btVector3		getGravity () const { return btDiscreteDynamicsWorld::getGravity(); }

private:
	VuDynamicsStepCallback	*mpCallback;
};
