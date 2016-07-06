//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics world implementation
// 
//*****************************************************************************

#include "VuDynamicsWorldImpl.h"


//*****************************************************************************
VuDynamicsWorldImpl::VuDynamicsWorldImpl(btDispatcher *dispatcher, btBroadphaseInterface *pairCache, btConstraintSolver *constraintSolver, btCollisionConfiguration *collisionConfiguration):
	btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration),
	mpCallback(VUNULL)
{
	m_localTime = 0;
}

//*****************************************************************************
VuDynamicsWorldImpl::~VuDynamicsWorldImpl()
{
}

//*****************************************************************************
int	VuDynamicsWorldImpl::stepSimulation(btScalar timeStep, int maxSubSteps, btScalar fixedTimeStep)
{
	VUASSERT(maxSubSteps, "VuDynamicsWorldImpl::stepSimulation() maxSubSteps <= 0");

	startProfiling(timeStep);

	BT_PROFILE("stepSimulation");

	// fixed timestep with interpolation
	int numSimulationSubSteps = 0;
	float previousLocalTime = m_localTime;
	m_localTime += timeStep;
	if ( m_localTime > fixedTimeStep )
	{
		numSimulationSubSteps = int( m_localTime / fixedTimeStep);
		m_localTime -= numSimulationSubSteps * fixedTimeStep;
	}

	// process some debugging flags
	if (getDebugDrawer())
	{
		gDisableDeactivation = (getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_NoDeactivation) != 0;
	}
	if (numSimulationSubSteps)
	{
		saveKinematicState(fixedTimeStep);


		//clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
		int clampedSimulationSteps = (numSimulationSubSteps > maxSubSteps)? maxSubSteps : numSimulationSubSteps;

		for (int i=0;i<clampedSimulationSteps;i++)
		{
			applyGravity();

			if ( mpCallback )
			{
				float fdt = fixedTimeStep;
				if ( i == 0 )
					fdt -= previousLocalTime;
				mpCallback->onDynamicsAdvanceEnvironment(fdt, true);
				mpCallback->onDynamicsApplyForces(fixedTimeStep);
			}

			internalSingleStepSimulation(fixedTimeStep);

			clearForces();
		}
	}

	if ( mpCallback )
	{
		float fdt = numSimulationSubSteps ? m_localTime : timeStep;
		mpCallback->onDynamicsAdvanceEnvironment(fdt, false);
	}

	return numSimulationSubSteps;
}
