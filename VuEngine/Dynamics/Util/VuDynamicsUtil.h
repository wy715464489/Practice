//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics utility functionality
// 
//*****************************************************************************

#pragma once

#include "btBulletDynamicsCommon.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"

class VuRigidBody;


namespace VuDynamicsUtil
{
	// type conversion

	inline VuVector3	toVuVector3(const btVector3 &vec);
	inline btVector3	toBtVector3(const VuVector3 &vec);

	inline VuMatrix		toVuMatrix(const btTransform &trans);
	inline btTransform	toBtTransform(const VuMatrix &mat);

	inline btDbvtAabbMm	toBtAabb(const VuAabb &aabb);


	// rigid body functionality

	inline VuVector3	pointVelocityWorld(const btRigidBody &rb, const VuVector3 &vPosWorld);
	inline void			applyForceWorld(btRigidBody &rb, const VuVector3 &vForce, const VuVector3 &vPosWorld);
	inline void			applyImpulseWorld(btRigidBody &rb, const VuVector3 &vImpulse, const VuVector3 &vPosWorld);
	inline void			applyCentralForce(btRigidBody &rb, const VuVector3 &vForce);
	inline void			applyCentralImpulse(btRigidBody &rb, const VuVector3 &vImpulse);
	inline void			applyTorque(btRigidBody &rb, const VuVector3 &vTorque);
	float				collisionImpulse(const VuRigidBody *pRB0, const VuRigidBody *pRB1, const VuVector3 &pos, const VuVector3 &nor1);


	// collision shapes

	VuAabb	getCollisionShapeAabb(const btCollisionShape *shape, const VuMatrix &transform);


	// shadows

	bool	getShadowValue(const VuVector3 &pos, const VuVector3 &ray, float &shadowValue);

};


#include "VuDynamicsUtil.inl"
