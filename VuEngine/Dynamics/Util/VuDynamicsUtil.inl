//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics utility inline functionality
// 
//*****************************************************************************


//*****************************************************************************
VuVector3 VuDynamicsUtil::toVuVector3(const btVector3 &vec)
{
	return VuVector3(vec.getX(), vec.getY(), vec.getZ());
}

//*****************************************************************************
btVector3 VuDynamicsUtil::toBtVector3(const VuVector3 &vec)
{
	return btVector3(vec.mX, vec.mY, vec.mZ);
}

//*****************************************************************************
VuMatrix VuDynamicsUtil::toVuMatrix(const btTransform &trans)
{
	const btVector3 &x = trans.getBasis()[0];
	const btVector3 &y = trans.getBasis()[1];
	const btVector3 &z = trans.getBasis()[2];
	const btVector3 &t = trans.getOrigin();

	return VuMatrix(
		VuVector4(x.getX(), y.getX(), z.getX(), 0),
		VuVector4(x.getY(), y.getY(), z.getY(), 0),
		VuVector4(x.getZ(), y.getZ(), z.getZ(), 0),
		VuVector4(t.getX(), t.getY(), t.getZ(), 1)
	);
}

//*****************************************************************************
btTransform VuDynamicsUtil::toBtTransform(const VuMatrix &mat)
{
	const VuVector3 &x = mat.getAxisX();
	const VuVector3 &y = mat.getAxisY();
	const VuVector3 &z = mat.getAxisZ();
	const VuVector3 &t = mat.getTrans();

	return btTransform(
		btMatrix3x3(
			x.mX, y.mX, z.mX,
			x.mY, y.mY, z.mY,
			x.mZ, y.mZ, z.mZ
		),
		btVector3(t.mX, t.mY, t.mZ)
	);
}

//*****************************************************************************
inline btDbvtAabbMm VuDynamicsUtil::toBtAabb(const VuAabb &aabb)
{
	return btDbvtAabbMm::FromMM(toBtVector3(aabb.mMin), toBtVector3(aabb.mMax));
}

//*****************************************************************************
VuVector3 VuDynamicsUtil::pointVelocityWorld(const btRigidBody &rb, const VuVector3 &vPosWorld)
{
	VuVector3 vRBPos = toVuVector3(rb.getCenterOfMassPosition());
	VuVector3 vRBLinearVel = toVuVector3(rb.getLinearVelocity());
	VuVector3 vRBAngularVel = toVuVector3(rb.getAngularVelocity());

	return vRBLinearVel + VuCross(vRBAngularVel, vPosWorld - vRBPos);
}

//*****************************************************************************
void VuDynamicsUtil::applyForceWorld(btRigidBody &rb, const VuVector3 &vForce, const VuVector3 &vPosWorld)
{
	VuVector3 vRBPos = toVuVector3(rb.getCenterOfMassPosition());

	rb.applyForce(VuDynamicsUtil::toBtVector3(vForce), VuDynamicsUtil::toBtVector3(vPosWorld - vRBPos));
}

//*****************************************************************************
void VuDynamicsUtil::applyImpulseWorld(btRigidBody &rb, const VuVector3 &vImpulse, const VuVector3 &vPosWorld)
{
	VuVector3 vRBPos = toVuVector3(rb.getCenterOfMassPosition());

	rb.applyImpulse(VuDynamicsUtil::toBtVector3(vImpulse), VuDynamicsUtil::toBtVector3(vPosWorld - vRBPos));
}

//*****************************************************************************
void VuDynamicsUtil::applyCentralForce(btRigidBody &rb, const VuVector3 &vForce)
{
	VUASSERT(VuIsFinite(vForce.mX) && VuIsFinite(vForce.mY) && VuIsFinite(vForce.mZ), "VuDynamicsUtil::applyCentralForce() bad force");

	rb.applyCentralForce(VuDynamicsUtil::toBtVector3(vForce));
}

//*****************************************************************************
void VuDynamicsUtil::applyCentralImpulse(btRigidBody &rb, const VuVector3 &vImpulse)
{
	VUASSERT(VuIsFinite(vImpulse.mX) && VuIsFinite(vImpulse.mY) && VuIsFinite(vImpulse.mZ), "VuDynamicsUtil::applyCentralImpulse() bad impulse");

	rb.applyCentralImpulse(VuDynamicsUtil::toBtVector3(vImpulse));
}

//*****************************************************************************
void VuDynamicsUtil::applyTorque(btRigidBody &rb, const VuVector3 &vTorque)
{
	VUASSERT(VuIsFinite(vTorque.mX) && VuIsFinite(vTorque.mY) && VuIsFinite(vTorque.mZ), "VuDynamicsUtil::applyTorque() bad torque");

	rb.applyTorque(VuDynamicsUtil::toBtVector3(vTorque));
}
