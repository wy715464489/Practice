//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ragdoll class
// 
//*****************************************************************************

#include "VuRagdoll.h"
#include "VuEngine/Animation/VuSkeleton.h"
#include "VuEngine/Animation/VuAnimationTransform.h"
#include "VuEngine/Animation/VuAnimationUtil.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Dynamics/Util/VuDynamicsUtil.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Water/VuWaterUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Util/VuDataUtil.h"


// constants
#define MAX_BODY_COUNT 32


//*****************************************************************************
VuRagdoll::VuRagdoll(btDynamicsWorld *pDynamicsWorld):
	mpDynamicsWorld(pDynamicsWorld),
	mpSkeleton(VUNULL),
	mFluidsDensityModifier(1.0f),
	mFluidsLiftCoeff(0.01f),
	mActive(false),
	mScale(1.0f)
{
}

//*****************************************************************************
VuRagdoll::~VuRagdoll()
{
	clear();
}

//*****************************************************************************
bool VuRagdoll::configure(VuSkeleton *pSkeleton, const VuJsonContainer &data, VuEntity *pEntity, const Params &params)
{
	VUASSERT(mActive == false, "VuRagdoll::configure() called while active");

	clear();

	mParams = params;

	mpSkeleton = pSkeleton;
	mpSkeleton->addRef();

	float mScale = 1.0f;
	data["Scale"].getValue(mScale);

	float linearDamping = data["LinearDamping"].asFloat();
	float angularDamping = data["AngularDamping"].asFloat();
	float linearSleepingThreshold = data["LinearSleepingThreshold"].asFloat();
	float angularSleepingThreshold = data["AngularSleepingThreshold"].asFloat();

	data["FluidsDensityModifier"].getValue(mFluidsDensityModifier);
	data["FluidsLiftCoeff"].getValue(mFluidsLiftCoeff);

	const char *surfaceType = data["SurfaceType"].asCString();
	VUUINT8 surfaceID = VuDynamics::IF()->getSurfaceTypeID(surfaceType);

	// bodies
	const VuJsonContainer &bodies = data["Bodies"];
	for ( int i = 0; i < bodies.size(); i++ )
	{
		const VuJsonContainer &bodyData = bodies[i];

		Body body;
		bodyData["Name"].getValue(body.mName);

		// bone index
		const std::string &bone = bodyData["Bone"].asString();
		body.mBoneIndex = mpSkeleton->getBoneIndex(bone.c_str());
		if ( body.mBoneIndex == -1 )
		{
			VUPRINTF("VuRagdoll::configure() bone '%s' not found\n", bone.c_str());
			clear();
			return false;
		}
		else
		{
			// transform
			VuMatrix modelTransform;
			getTransformData(mScale, bodyData["Pos"], bodyData["Rot"], modelTransform);
			body.mTransform = modelTransform*mpSkeleton->mpInvModelPoseMatrices[body.mBoneIndex];
			body.mInvTransform = body.mTransform;
			body.mInvTransform.invert();

			// shape
			body.mRadius = bodyData["Radius"].asFloat()*mScale;
			body.mHeight = bodyData["Height"].asFloat()*mScale;
			btCollisionShape *pShape = new btCapsuleShapeX(body.mRadius, body.mHeight);
		
			// rigid body
			float mass = bodyData["Mass"].asFloat();

			btVector3 inertia;
			pShape->calculateLocalInertia(mass, inertia);

			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, VUNULL, pShape, inertia);
			body.mpRigidBody = new VuRigidBody(rbInfo, pEntity, COL_ENGINE_RAGDOLL, params.mCollisionMask);

			if ( VuDynamics::IF() )
			{
				body.mpRigidBody->setGravity(VuDynamicsUtil::toBtVector3(VuDynamics::IF()->getGravity()));
				body.mpRigidBody->setDamping(linearDamping, angularDamping);
				body.mpRigidBody->setSleepingThresholds(linearSleepingThreshold, angularSleepingThreshold);
				body.mpRigidBody->setSurfaceType(surfaceID);
			}

			// buoyancy
			body.mFluidsDensity = bodyData["FluidsDensity"].asFloat();

			// variables
			body.mSubmergedRatio = 0.0f;
			
			// add body
			mBodies.push_back(body);
		}
	}

	// set motion states (done now because memory is now locked)
	for ( int i = 0; i < (int)mBodies.size(); i++ )
		mBodies[i].mpRigidBody->setMotionState(&mBodies[i]);

	// hinge constraints
	const VuJsonContainer &hingeConstraints = data["HingeConstraints"];
	for ( int i = 0; i < hingeConstraints.size(); i++ )
	{
		const VuJsonContainer &constraintData = hingeConstraints[i];

		const char *bodyA = constraintData["BodyA"].asCString();
		const char *bodyB = constraintData["BodyB"].asCString();

		int bodyIndexA = getBodyIndex(bodyA);
		int bodyIndexB = getBodyIndex(bodyB);
		if ( bodyIndexA == -1 )
		{
			VUPRINTF("VuRagdoll::configure() body '%s' not found\n", bodyA);
			clear();
			return false;
		}
		else if ( bodyIndexB == -1 )
		{
			VUPRINTF("VuRagdoll::configure() body '%s' not found\n", bodyB);
			clear();
			return false;
		}
		else 
		{
			VuRigidBody &rigidBodyA = *mBodies[bodyIndexA].mpRigidBody;
			VuRigidBody &rigidBodyB = *mBodies[bodyIndexB].mpRigidBody;

			VuMatrix modelTransform;
			getTransformData(mScale, constraintData["Pos"], constraintData["Rot"], modelTransform);
			VuMatrix xformA = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexA].mBoneIndex]*mBodies[bodyIndexA].mInvTransform;
			VuMatrix xformB = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexB].mBoneIndex]*mBodies[bodyIndexB].mInvTransform;

			float lowLimit = VuDegreesToRadians(constraintData["LowLimit"].asFloat());
			float highLimit = VuDegreesToRadians(constraintData["HighLimit"].asFloat());

			btHingeConstraint *pHinge = new btHingeConstraint(rigidBodyA, rigidBodyB, VuDynamicsUtil::toBtTransform(xformA), VuDynamicsUtil::toBtTransform(xformB));
			pHinge->setLimit(lowLimit, highLimit);
			pHinge->setDbgDrawSize(0.1f);

			mConstraints.push_back(pHinge);
		}
	}

	// cone twist constraints
	const VuJsonContainer &coneTwistConstraints = data["ConeTwistConstraints"];
	for ( int i = 0; i < coneTwistConstraints.size(); i++ )
	{
		const VuJsonContainer &constraintData = coneTwistConstraints[i];

		const char *bodyA = constraintData["BodyA"].asCString();
		const char *bodyB = constraintData["BodyB"].asCString();

		int bodyIndexA = getBodyIndex(bodyA);
		int bodyIndexB = getBodyIndex(bodyB);
		if ( bodyIndexA == -1 )
		{
			VUPRINTF("VuRagdoll::configure() body '%s' not found\n", bodyA);
			clear();
			return false;
		}
		else if ( bodyIndexB == -1 )
		{
			VUPRINTF("VuRagdoll::configure() body '%s' not found\n", bodyB);
			clear();
			return false;
		}
		else 
		{
			VuRigidBody &rigidBodyA = *mBodies[bodyIndexA].mpRigidBody;
			VuRigidBody &rigidBodyB = *mBodies[bodyIndexB].mpRigidBody;

			VuMatrix modelTransform;
			getTransformData(mScale, constraintData["Pos"], constraintData["Rot"], modelTransform);
			VuMatrix xformA = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexA].mBoneIndex]*mBodies[bodyIndexA].mInvTransform;
			VuMatrix xformB = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexB].mBoneIndex]*mBodies[bodyIndexB].mInvTransform;
			xformB = VuMatrix::translation(xformB.getTrans());

			float swing1Limit = VuDegreesToRadians(constraintData["Swing1Limit"].asFloat());
			float swing2Limit = VuDegreesToRadians(constraintData["Swing2Limit"].asFloat());
			float twistLimit = VuDegreesToRadians(constraintData["TwistLimit"].asFloat());

			btConeTwistConstraint *pConeTwist = new btConeTwistConstraint(rigidBodyA, rigidBodyB, VuDynamicsUtil::toBtTransform(xformA), VuDynamicsUtil::toBtTransform(xformB));
			pConeTwist->setLimit(swing1Limit, swing2Limit, twistLimit);
			pConeTwist->setDbgDrawSize(0.1f);

			mConstraints.push_back(pConeTwist);
		}
	}

	// point 2 point constraints
	const VuJsonContainer &point2PointConstraints = data["Point2PointConstraints"];
	for ( int i = 0; i < point2PointConstraints.size(); i++ )
	{
		const VuJsonContainer &constraintData = point2PointConstraints[i];

		const char *bodyA = constraintData["BodyA"].asCString();
		const char *bodyB = constraintData["BodyB"].asCString();

		int bodyIndexA = getBodyIndex(bodyA);
		int bodyIndexB = getBodyIndex(bodyB);
		if ( bodyIndexA == -1 )
		{
			VUPRINTF("VuRagdoll::configure() body '%s' not found\n", bodyA);
			clear();
			return false;
		}
		else if ( bodyIndexB == -1 )
		{
			VuRigidBody &rigidBodyA = *mBodies[bodyIndexA].mpRigidBody;

			VuVector3 pos(0,0,0);
			VuDataUtil::getValue(constraintData["Pos"], pos);

			VuMatrix modelTransform;
			modelTransform.loadIdentity();
			modelTransform.setTrans(pos*mScale);
			VuMatrix xformA = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexA].mBoneIndex]*mBodies[bodyIndexA].mInvTransform;

			btPoint2PointConstraint *pPoint2Point = new btPoint2PointConstraint(rigidBodyA, VuDynamicsUtil::toBtVector3(xformA.getTrans()));
			pPoint2Point->setDbgDrawSize(0.1f);

			mConstraints.push_back(pPoint2Point);
		}
		else 
		{
			VuRigidBody &rigidBodyA = *mBodies[bodyIndexA].mpRigidBody;
			VuRigidBody &rigidBodyB = *mBodies[bodyIndexB].mpRigidBody;

			VuVector3 pos(0,0,0);
			VuDataUtil::getValue(constraintData["Pos"], pos);

			VuMatrix modelTransform;
			modelTransform.loadIdentity();
			modelTransform.setTrans(pos*mScale);
			VuMatrix xformA = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexA].mBoneIndex]*mBodies[bodyIndexA].mInvTransform;
			VuMatrix xformB = modelTransform*mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexB].mBoneIndex]*mBodies[bodyIndexB].mInvTransform;
			xformB = VuMatrix::translation(xformB.getTrans());

			btPoint2PointConstraint *pPoint2Point = new btPoint2PointConstraint(rigidBodyA, rigidBodyB, VuDynamicsUtil::toBtVector3(xformA.getTrans()), VuDynamicsUtil::toBtVector3(xformB.getTrans()));
			pPoint2Point->setDbgDrawSize(0.1f);

			mConstraints.push_back(pPoint2Point);
		}
	}

	// fixed bones (non-simulated)
	for ( int iBone = 1; iBone < mpSkeleton->mBoneCount; iBone++ )
	{
		bool found = false;
		for ( int iBody = 0; iBody < (int)mBodies.size(); iBody++ )
			if ( mBodies[iBody].mBoneIndex == iBone )
				found = true;

		if ( !found )
		{
			FixedBone fixedBone;

			fixedBone.mIndex = iBone;
			fixedBone.mLocalMatrix.loadIdentity();

			mFixedBones.push_back(fixedBone);
		}
	}

	return true;
}

//*****************************************************************************
void VuRagdoll::clear()
{
	stopSimulation();

	for ( int i = 0; i < (int)mBodies.size(); i++ )
	{
		delete mBodies[i].mpRigidBody->getCollisionShape();
		delete mBodies[i].mpRigidBody;
	}
	mBodies.clear();

	for ( int i = 0; i < (int)mConstraints.size(); i++ )
		delete mConstraints[i];
	mConstraints.clear();
	
	mFixedBones.clear();

	if ( mpSkeleton )
	{
		mpSkeleton->removeRef();
		mpSkeleton = VUNULL;
	}
}

//*****************************************************************************
void VuRagdoll::attachTo(VuRigidBody *pRigidBody, const VuVector3 &offset, const char *bodyName)
{
	int bodyIndexB = getBodyIndex(bodyName);
	if ( bodyIndexB == -1 )
	{
		VUPRINTF("VuRagdoll::attachTo() body '%s' not found\n", bodyName);
		return;
	}

	VuRigidBody &rigidBodyA = *pRigidBody;
	VuRigidBody &rigidBodyB = *mBodies[bodyIndexB].mpRigidBody;

	VuMatrix xformA;
	xformA.loadIdentity();
	xformA.setTrans(offset);

	VuMatrix xformB = mpSkeleton->mpInvModelPoseMatrices[mBodies[bodyIndexB].mBoneIndex]*mBodies[bodyIndexB].mInvTransform;

	btGeneric6DofConstraint *pConstraint = new btGeneric6DofConstraint(rigidBodyA, rigidBodyB, VuDynamicsUtil::toBtTransform(xformA), VuDynamicsUtil::toBtTransform(xformB), true);
	pConstraint->setLinearLowerLimit(btVector3(0,0,0));
	pConstraint->setLinearUpperLimit(btVector3(0,0,0));
	pConstraint->setAngularLowerLimit(btVector3(0,0,0));
	pConstraint->setAngularUpperLimit(btVector3(0,0,0));

	pConstraint->setDbgDrawSize(0.1f);

	mConstraints.push_back(pConstraint);
}

//*****************************************************************************
void VuRagdoll::startSimulation(const VuMatrix &modelMat, const VuAnimationTransform *pLocalPose, const VuVector3 &linVel, const VuVector3 &angVel)
{
	if ( !mActive )
	{
		mActive = true;

		int boneCount = mpSkeleton->mBoneCount;

		// calculate model space matrices
		VuAnimationTransform *pModelPose = (VuAnimationTransform *)VuScratchPad::get();
		VuMatrix *pModelMatrices = (VuMatrix *)(pModelPose + boneCount);
		VuAnimationUtil::transformLocalPoseToModelPose(boneCount, mpSkeleton->mpParentIndices, pLocalPose, pModelPose, pModelMatrices);

		for ( int iBody = 0; iBody < (int)mBodies.size(); iBody++ )
		{
			Body &body = mBodies[iBody];

			body.mWorldTransform = body.mTransform*pModelMatrices[body.mBoneIndex]*modelMat;

			VuVector3 bodyLinVel = linVel + VuCross(angVel, body.mWorldTransform.getTrans() - mBodies[0].mWorldTransform.getTrans());
			VuVector3 bodyAngVel = angVel;

			body.mpRigidBody->setLinearVelocity(VuDynamicsUtil::toBtVector3(bodyLinVel));
			body.mpRigidBody->setAngularVelocity(VuDynamicsUtil::toBtVector3(bodyAngVel));
			body.mpRigidBody->setInterpolationLinearVelocity(VuDynamicsUtil::toBtVector3(bodyLinVel));
			body.mpRigidBody->setInterpolationAngularVelocity(VuDynamicsUtil::toBtVector3(bodyAngVel));
			body.mpRigidBody->setWorldTransform(VuDynamicsUtil::toBtTransform(body.mWorldTransform));
			body.mpRigidBody->setInterpolationWorldTransform(VuDynamicsUtil::toBtTransform(body.mWorldTransform));

			body.mSubmergedRatio = 0.0f;

			if ( mpDynamicsWorld )
				mpDynamicsWorld->addRigidBody(body.mpRigidBody);
			else
				VuDynamics::IF()->addRigidBody(body.mpRigidBody);
		}

		for ( int i = 0; i < (int)mConstraints.size(); i++ )
		{
			if ( mConstraints[i]->getConstraintType() == POINT2POINT_CONSTRAINT_TYPE )
			{
				btPoint2PointConstraint *pP2PC = (btPoint2PointConstraint *)mConstraints[i];
				if ( &pP2PC->getRigidBodyB() == &pP2PC->getFixedBody() )
					pP2PC->setPivotB(pP2PC->getRigidBodyA().getCenterOfMassTransform()(pP2PC->getPivotInA()));
			}

			if ( mpDynamicsWorld )
				mpDynamicsWorld->addConstraint(mConstraints[i], true);
			else
				VuDynamics::IF()->addConstraint(mConstraints[i], true);
		}

		for ( int i = 0; i < (int)mFixedBones.size(); i++ )
			pLocalPose[mFixedBones[i].mIndex].toMatrix(mFixedBones[i].mLocalMatrix);

		// register dynamics methods
		if ( !mpDynamicsWorld && VuWater::IF() && mParams.mWaterSimulation )
			VuDynamics::IF()->registerStepCallback(this);
	}
}

//*****************************************************************************
void VuRagdoll::stopSimulation()
{
	if ( mActive )
	{
		mActive = false;

		if ( mpDynamicsWorld )
		{
			for ( int i = 0; i < (int)mBodies.size(); i++ )
				mpDynamicsWorld->removeRigidBody(mBodies[i].mpRigidBody);

			for ( int i = 0; i < (int)mConstraints.size(); i++ )
				mpDynamicsWorld->removeConstraint(mConstraints[i]);
		}
		else
		{
			for ( int i = 0; i < (int)mBodies.size(); i++ )
				VuDynamics::IF()->removeRigidBody(mBodies[i].mpRigidBody);

			for ( int i = 0; i < (int)mConstraints.size(); i++ )
				VuDynamics::IF()->removeConstraint(mConstraints[i]);

			// unregister dynamics methods
			VuDynamics::IF()->unregisterStepCallback(this);
		}
	}
}

//*****************************************************************************
void VuRagdoll::updateModelMatrices(const VuMatrix &modelMat, VuMatrix *pModelMatrices) const
{
	VuMatrix invModelMat = modelMat;
	invModelMat.invert();

	// simulated bones
	for ( Bodies::const_iterator iter = mBodies.begin(); iter != mBodies.end(); iter++ )
		pModelMatrices[iter->mBoneIndex] = iter->mInvTransform*iter->mWorldTransform*invModelMat;

	// fixed bones
	for ( FixedBones::const_iterator iter = mFixedBones.begin(); iter != mFixedBones.end(); iter++ )
	{
		int parentIndex = mpSkeleton->mpParentIndices[iter->mIndex];
		pModelMatrices[iter->mIndex] = iter->mLocalMatrix*pModelMatrices[parentIndex];
	}
}

//*****************************************************************************
int VuRagdoll::getBodyIndex(const char *bodyName) const
{
	for ( int i = 0; i < (int)mBodies.size(); i++ )
		if ( mBodies[i].mName == bodyName )
			return i;

	return -1;
}

//*****************************************************************************
int VuRagdoll::getTypedConstraintIndex(int index) const
{
	btTypedConstraintType constraintType = getConstraint(index)->getConstraintType();

	int typedIndex = 0;
	for ( int i = 0; i < index; i++ )
		if ( getConstraint(i)->getConstraintType() == constraintType )
			typedIndex++;

	return typedIndex;
}

//*****************************************************************************
int VuRagdoll::getConstraintIndex(btTypedConstraintType type, int typedIndex) const
{
	for ( int i = 0; i < getConstraintCount(); i++ )
	{
		if ( getConstraint(i)->getConstraintType() == type )
		{
			if ( typedIndex == 0 )
				return i;

			typedIndex--;
		}
	}

	return 0;
}

//*****************************************************************************
void VuRagdoll::drawDebugBodies(const VuCamera &camera, const VuColor &color) const
{
	if ( mActive )
	{
		for ( int i = 0; i < (int)mBodies.size(); i++ )
		{
			const Body &body = mBodies[i];

			VuMatrix shapeMat = body.mWorldTransform;
			shapeMat.rotateYLocal(VU_PIDIV2);

			VuGfxUtil::IF()->drawCapsuleSolid(color, body.mHeight, body.mRadius, 8, shapeMat, camera.getViewProjMatrix());
		}
	}
}

//*****************************************************************************
void VuRagdoll::onDynamicsApplyForces(float fdt)
{
	VuWaterPhysicsVertex waterVerts[32];

	// get surface data
	{
		VuAabb aabb;
		for ( int i = 0; i < (int)mBodies.size(); i++ )
		{
			waterVerts[i].mPosition = VuDynamicsUtil::toVuVector3(mBodies[i].mpRigidBody->getWorldTransform().getOrigin());
			aabb.addPoint(waterVerts[i].mPosition);
		}

		VuWaterSurfaceDataParams wsdParams(VuWaterSurfaceDataParams::VT_PHYSICS);

		wsdParams.mVertCount = (int)mBodies.size();
		wsdParams.mBoundingAabb = aabb;
		wsdParams.mBoundingCenter = aabb.getCenter();
		wsdParams.mBoundingRadius = aabb.getExtents().mag();
		wsdParams.mpPhysicsVertex = waterVerts;
		wsdParams.mStride = sizeof(waterVerts[0]);

		VuWater::IF()->getSurfaceData(wsdParams);
	}

	for ( int i = 0; i < (int)mBodies.size(); i++ )
	{
		Body &body = mBodies[i];
		const VuWaterPhysicsVertex &waterVert = waterVerts[i];

		if ( body.mFluidsDensity > 0.0f )
		{
			float radius = body.mRadius + 0.5f*body.mHeight;
			float mass = 1.0f/body.mpRigidBody->getInvMass();
			float buoyancy = body.mFluidsDensity*mFluidsDensityModifier;

			// calculate forces
			if ( waterVert.mPosition.mZ - radius < waterVert.mHeight )
			{
				float fVolume = (4.0f/3.0f)*VU_PI*radius*radius*radius;
				float fSurfaceArea = 4.0f*VU_PI*radius*radius;
				float fFrontalArea = VU_PI*radius*radius;

				float objectDensity = mass/fVolume;
				float fluidDensity = objectDensity/buoyancy;

				body.mSubmergedRatio = (waterVert.mHeight - (waterVert.mPosition.mZ - radius))/(2.0f*radius);
				body.mSubmergedRatio = VuMin(body.mSubmergedRatio, 1.0f);

				fVolume *= body.mSubmergedRatio;
				fSurfaceArea *= body.mSubmergedRatio;
				fFrontalArea *= body.mSubmergedRatio;

				VuVector3 linVel = body.mpRigidBody->getVuLinearVelocity();
				linVel -= waterVert.mDxyzDt;

				VuVector3 totalForce(0,0,0);

				// buoyancy
				{
					VuVector3 vForce(0.0f, 0.0f, -fVolume*fluidDensity*body.mpRigidBody->getGravity().z());

					totalForce += vForce;
				}

				// linear drag
				{
					VuVector3 vForce = -(0.5f*fluidDensity*fFrontalArea*SPHERE_DRAG_COEFFICIENT*linVel.mag())*linVel;

					totalForce += vForce;
				}

				// lift
				{
					const float DENSITY = 1000.0f;
					VuVector2 latVel(linVel.mX, linVel.mY);
					float area = VU_PI*radius*radius;
					area *= body.mSubmergedRatio;
					float force = 0.5f*DENSITY*latVel.magSquared()*area*mFluidsLiftCoeff;
					totalForce.mZ += force;
				}

				body.mpRigidBody->applyCentralForce(VuDynamicsUtil::toBtVector3(totalForce));
			}
			else
			{
				body.mSubmergedRatio = 0.0f;
			}
		}
	}
}

//*****************************************************************************
void VuRagdoll::getTransformData(float scale, const VuJsonContainer &posData, const VuJsonContainer &rotData, VuMatrix &xform)
{
	VuVector3 vPos(0,0,0);
	VuVector3 vRot(0,0,0);
	VuDataUtil::getValue(posData, vPos);
	VuDataUtil::getValue(rotData, vRot);
	vRot = VuDegreesToRadians(vRot);

	xform.setEulerAngles(vRot);
	xform.setTrans(vPos*scale);
}

//*****************************************************************************
void VuRagdoll::Body::getWorldTransform(btTransform& worldTrans) const
{
	worldTrans = VuDynamicsUtil::toBtTransform(mWorldTransform);
}

//*****************************************************************************
void VuRagdoll::Body::setWorldTransform(const btTransform& worldTrans)
{
	mWorldTransform = VuDynamicsUtil::toVuMatrix(worldTrans);
}
