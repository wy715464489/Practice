//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water utility functionality
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"

class VuRigidBody;


namespace VuWaterUtil
{
	struct VuEstimateSphereForcesParams
	{
		// in
		VuRigidBody	*mpRigidBody;
		VuVector3	mPosition;
		float		mMass;
		float		mRadius;
		float		mBuoyancy;
		float		mDragCoeff;
		float		mLiftCoeff;

		// out
		VuVector3	mTotalForce;
		VuVector3	mWaterVel;
		float		mWaterHeight;
		bool		mSubmerged;
	};
	void estimateSphereForces(VuEstimateSphereForcesParams &params);
}


// constants
#define SPHERE_DRAG_COEFFICIENT (0.47f)
