//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics ray test
// 
//*****************************************************************************

#include "VuDynamicsRayTest.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"


namespace VuDynamicsRayTest
{

class RayTestWrapper : public btCollisionWorld::RayResultCallback
{
public:
	RayTestWrapper(VuResult &result, unsigned int flags) : mResult(result)
	{
		if ( !(flags & TestBackfaces) )
		{
			m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
		}
	}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const
	{
		btCollisionObject *pColObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
		if ( pColObj->getInternalType() != btCollisionObject::CO_RIGID_BODY )
			return false;

		return mResult.needsCollision(static_cast<VuRigidBody *>(pColObj));
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
	{
		btVector3 normal = rayResult.m_hitNormalLocal;
		if ( !normalInWorldSpace )
			normal = rayResult.m_collisionObject->getWorldTransform().getBasis()*normal;

		int triangleIndex = rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_triangleIndex : -1;
		if ( !mResult.addResult(static_cast<const VuRigidBody *>(rayResult.m_collisionObject), rayResult.m_hitFraction, triangleIndex, VuDynamicsUtil::toVuVector3(normal)) )
			return 1.0f;

		mResult.mbHasHit = true;

		return rayResult.m_hitFraction;
	}

	VuResult	&mResult;
};

//*****************************************************************************
void test(const VuVector3 &from, const VuVector3 &to, VuResult &result, unsigned int flags)
{
	RayTestWrapper wrapper(result, flags);
	VuDynamics::IF()->getDynamicsWorld()->rayTest(VuDynamicsUtil::toBtVector3(from), VuDynamicsUtil::toBtVector3(to), wrapper);
}

} // namespace VuDynamicsRayTest
