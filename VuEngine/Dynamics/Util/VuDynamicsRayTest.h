//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics ray test
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"

class VuRigidBody;


namespace VuDynamicsRayTest
{
	//*****************************************************************************
	// Callbacks for performing ray tests.
	//*****************************************************************************
	class VuResult
	{
	public:
		VuResult() : mbHasHit(false) {}

		virtual bool	needsCollision(VuRigidBody *pRigidBody) { return true; }
		virtual bool	addResult(const VuRigidBody *pRigidBody, float hitFraction, int triangleIndex, const VuVector3 &normal) = 0;
		bool			mbHasHit;
	};
	class VuClosestResult : public VuResult
	{
	public:
		VuClosestResult() : mpRigidBody(VUNULL), mHitFraction(1) {}
		virtual bool	addResult(const VuRigidBody *pRigidBody, float hitFraction, int triangleIndex, const VuVector3 &normal)
		{
			if ( hitFraction <= mHitFraction )
			{
				mpRigidBody = pRigidBody;
				mHitFraction = hitFraction;
				mTriangleIndex = triangleIndex;
				mHitNormal = normal;
			}
			return true;
		}
		const VuRigidBody	*mpRigidBody;
		float				mHitFraction;
		int					mTriangleIndex;
		VuVector3			mHitNormal;
	};

	enum Flags
	{
		TestBackfaces        = 1 << 0,
	};
	void test(const VuVector3 &from, const VuVector3 &to, VuResult &cb, unsigned int flags = 0);
};
