//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water utility functionality
// 
//*****************************************************************************

#include "VuWaterUtil.h"
#include "VuWater.h"
#include "VuEngine/Dynamics/VuRigidBody.h"


void VuWaterUtil::estimateSphereForces(VuEstimateSphereForcesParams &params)
{
	VuVector3 totalForce(0,0,0);
	bool submerged = false;

	VuWaterPhysicsVertex waterVert = VuWater::IF()->getPhysicsVertex(params.mPosition);

	// calculate forces
	float radius = params.mRadius;
	if ( waterVert.mPosition.mZ - radius < waterVert.mHeight )
	{
		submerged = true;

		float fVolume = (4.0f/3.0f)*VU_PI*radius*radius*radius;
		float fSurfaceArea = 4.0f*VU_PI*radius*radius;
		float fFrontalArea = VU_PI*radius*radius;

		float objectDensity = params.mMass/fVolume;
		float fluidDensity = objectDensity/params.mBuoyancy;

		float submergedRatio = (waterVert.mHeight - (waterVert.mPosition.mZ - radius))/(2.0f*radius);
		submergedRatio = VuMin(submergedRatio, 1.0f);

		fVolume *= submergedRatio;
		fSurfaceArea *= submergedRatio;
		fFrontalArea *= submergedRatio;

		VuVector3 linVel = params.mpRigidBody->getVuLinearVelocity();
		linVel -= waterVert.mDxyzDt;

		// buoyancy
		{
			VuVector3 vForce(0.0f, 0.0f, -fVolume*fluidDensity*params.mpRigidBody->getGravity().z());

			totalForce += vForce;
		}

		// linear drag
		{
			VuVector3 vForce = -(0.5f*fluidDensity*fFrontalArea*params.mDragCoeff*linVel.mag())*linVel;

			totalForce += vForce;
		}

		// lift
		{
			VuVector2 latVel(linVel.mX, linVel.mY);
			float area = VU_PI*radius*radius;
			area *= submergedRatio;
			float force = 0.5f*fluidDensity*latVel.magSquared()*area*params.mLiftCoeff;
			totalForce.mZ += force;
		}
	}

	// output
	params.mTotalForce = totalForce;
	params.mWaterVel = waterVert.mDxyzDt;
	params.mWaterHeight = waterVert.mHeight;
	params.mSubmerged = submerged;
}