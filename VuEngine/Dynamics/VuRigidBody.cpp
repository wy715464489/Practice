//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rigid Body class
// 
//*****************************************************************************

#include "VuRigidBody.h"


//*****************************************************************************
VuRigidBody::VuRigidBody(const btRigidBody::btRigidBodyConstructionInfo &info, VuEntity *pEntity, VUINT16 collisionGroup, VUINT16 collisionMask):
	btRigidBody(info),
	mpEntity(pEntity),
	mpContactCallback(VUNULL),
	mSurfaceType(0),
	mCollisionGroup(collisionGroup),
	mCollisionMask(collisionMask),
	mExtendedFlags(0),
	mpShadowValues(VUNULL)
{
}

//*****************************************************************************
void VuRigidBody::setSurfaceType(const char *surfaceType)
{
	setSurfaceType(VuDynamics::IF()->getSurfaceTypeID(surfaceType));
}

//*****************************************************************************
void VuRigidBody::setCollisionGroup(VUINT16 group)
{
	mCollisionGroup = group;

	if ( btBroadphaseProxy *pProxy = getBroadphaseHandle() )
		pProxy->m_collisionFilterGroup = group;
}

//*****************************************************************************
void VuRigidBody::setCollisionMask(VUINT16 mask)
{
	mCollisionMask = mask;

	if ( btBroadphaseProxy *pProxy = getBroadphaseHandle() )
		pProxy->m_collisionFilterMask = mask;
}
