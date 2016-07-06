//*****************************************************************************
//
//  Copyright (c) 200-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics utility
// 
//*****************************************************************************

#include "VuDynamicsUtil.h"
#include "VuDynamicsRayTest.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"


//*****************************************************************************
float VuDynamicsUtil::collisionImpulse(const VuRigidBody *pRB0, const VuRigidBody *pRB1, const VuVector3 &pos, const VuVector3 &nor1)
{
	float fImpulse = 0;

	if ( pRB0->hasContactResponse() && pRB1->hasContactResponse() )
	{
		// compute relative velocity
		VuVector3 vRelV = VuDynamicsUtil::pointVelocityWorld(*pRB0, pos) - VuDynamicsUtil::pointVelocityWorld(*pRB1, pos);
		float fRelVel = VuDot(nor1, vRelV);
		if ( fRelVel < 0 )
		{
			btVector3 vPosWorld = VuDynamicsUtil::toBtVector3(pos);
			btVector3 vNorWorld = VuDynamicsUtil::toBtVector3(nor1);

			// compute relative positions of contact point
			btVector3 vRelP0 = vPosWorld - pRB0->getCenterOfMassTransform().getOrigin();
			btVector3 vRelP1 = vPosWorld - pRB1->getCenterOfMassTransform().getOrigin();

			// compute numerator = -(1+e)*rvel
			float fNum = -fRelVel;

			// compute denominator0 = (1/m0) + n.((IInv0*(r0 x n)) x r0)
			float fDen0 = pRB0->getInvMass() + vNorWorld.dot((pRB0->getInvInertiaTensorWorld()*vRelP0.cross(vNorWorld)).cross(vRelP0));

			// compute denominator1 = (1/m1) + n.((IInv1*(r1 x n)) x r1)
			float fDen1 = pRB1->getInvMass() + vNorWorld.dot((pRB1->getInvInertiaTensorWorld()*vRelP1.cross(vNorWorld)).cross(vRelP1));

			// compute impulse
			fImpulse = (fNum/(fDen0 + fDen1));
		}
	}

	return fImpulse;
}

//*****************************************************************************
VuAabb VuDynamicsUtil::getCollisionShapeAabb(const btCollisionShape *shape, const VuMatrix &transform)
{
	btVector3 btMin, btMax;
	shape->getAabb(toBtTransform(transform), btMin, btMax);

	VuAabb aabb;
	aabb.mMin = toVuVector3(btMin);
	aabb.mMax = toVuVector3(btMax);

	return aabb;
}

//*****************************************************************************
class VuShadowValueTestResult : public VuDynamicsRayTest::VuClosestResult
{
public:
	virtual bool needsCollision(VuRigidBody *pRigidBody)
	{
		if ( pRigidBody->getShadowValues() )
			return true;

		return false;
	}
};

//*****************************************************************************
bool VuDynamicsUtil::getShadowValue(const VuVector3 &pos, const VuVector3 &ray, float &shadowValue)
{
	// cast ray
	VuShadowValueTestResult rayTestResult;
	VuDynamicsRayTest::test(pos, pos + ray, rayTestResult);
	if ( rayTestResult.mbHasHit )
	{
		VuVector3 contact = pos + ray*rayTestResult.mHitFraction;

		if (const VUUINT8 *pShadowValues = rayTestResult.mpRigidBody->getShadowValues())
		{
			if (const VuCollisionMeshAsset *pAsset = static_cast<const VuCollisionMeshAsset *>(rayTestResult.mpRigidBody->getCollisionShape()->getUserPointer()))
			{
				int triIndex = rayTestResult.mTriangleIndex;

				if ( !(pAsset->getTriangleMaterial(triIndex).mFlags & VuCollisionMeshAsset::VuMaterial::IGNORE_BAKED_SHADOW) )
				{
					int iv0 = pAsset->getIndex(triIndex * 3 + 0);
					int iv1 = pAsset->getIndex(triIndex * 3 + 1);
					int iv2 = pAsset->getIndex(triIndex * 3 + 2);

					VUUINT8 shadow0 = pShadowValues[iv0];
					VUUINT8 shadow1 = pShadowValues[iv1];
					VUUINT8 shadow2 = pShadowValues[iv2];

					// optimization
					if ((shadow0 == shadow1) && (shadow0 == shadow2))
					{
						shadowValue = shadow0 / 255.0f;
					}
					else
					{
						// calculate interpolation factors
						float f01, f02;
	//					const VuCollisionMeshAsset::VuTriangle &tri = pAsset->getTriangle(triIndex);

						VuMatrix transform = rayTestResult.mpRigidBody->getVuCenterOfMassTransform();

						VuVector3 v0 = pAsset->getVert(pAsset->getIndex(triIndex * 3 + 0));
						VuVector3 v1 = pAsset->getVert(pAsset->getIndex(triIndex * 3 + 1));
						VuVector3 v2 = pAsset->getVert(pAsset->getIndex(triIndex * 3 + 2));

						v0 = transform.transform(v0);
						v1 = transform.transform(v1);
						v2 = transform.transform(v2);

						VuVector3 offset = contact - v0;
						VuVector3 edge01 = v1 - v0;
						VuVector3 edge02 = v2 - v0;

						float m02 = edge02.mag();
						if (m02 > FLT_EPSILON)
							m02 = 1.0f / m02;

						f02 = VuDot(edge02, offset)*m02*m02;
						f02 = VuClamp(f02, 0.0f, 1.0f);

						VuVector3 edgemid01 = edge01 - f02*edge02;
						VuVector3 origin0 = v0 + f02*edge02;
						VuVector3 offset01 = contact - origin0;

						float m01 = edgemid01.mag();
						if (m01 > FLT_EPSILON)
							m01 = 1.0f / m01;

						f01 = VuDot(edgemid01, offset01)*m01*m01;
						f01 = VuClamp(f01, 0.0f, 1.0f);

						// modulate shadow value
						float i02 = f02*shadow2 + (1.0f - f02)*shadow0;
						float value = f01*shadow1 + (1.0f - f01)*i02;

						shadowValue = value / 255.0f;
					}
				}

				return true;
			}
		}
	}

	return false;
}
