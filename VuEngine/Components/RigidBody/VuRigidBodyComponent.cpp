//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RigidBodyComponent class
// 
//*****************************************************************************

#include "VuRigidBodyComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuBitFieldProperty.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Dynamics/VuStridingMesh.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Light/VuLightUtil.h"


IMPLEMENT_RTTI(VuRigidBodyComponent, VuComponent);



//*****************************************************************************
VuRigidBodyComponent::VuRigidBodyComponent(VuEntity *pOwnerEntity) : VuComponent(pOwnerEntity),
	mCollisionType(CT_NONE),
	mRadius(1),
	mSize(2,2,2),
	mHeight(2),
	mCollisionOffset(0,0,0),
	mLinearDamping(0.0f),
	mAngularDamping(0.0f),
	mLinearSleepingThreshold(0.8f),
	mAngularSleepingThreshold(1.0f),
	mInitiallyActive(false),
	mpStridingMesh(VUNULL),
	mCollisionFlags(btCollisionObject::CF_STATIC_OBJECT),
	mCollisionGroup(0),
	mCollisionMask(0),
	mExtendedFlags(0),
	mCenterOfMass(0,0,0),
	mMass(0),
	mpMotionState(VUNULL),
	mpRigidBodyContactCallback(VUNULL),
	mpShape(VUNULL),
	mpChildShape(VUNULL),
	mpRigidBody(VUNULL),
	mbAdded(false),
	mShadowValues(0)
{
	// add properties
	static VuStaticIntEnumProperty::Choice collisionTypeChoices[] =
	{
		{ "none", CT_NONE },
		{ "sphere", CT_SPHERE },
		{ "box", CT_BOX },
		{ "cylinder x", CT_CYLINDER_X },
		{ "cylinder y", CT_CYLINDER_Y },
		{ "cylinder z", CT_CYLINDER_Z },
		{ "convex hull", CT_CONVEX_HULL },
		{ "triangle mesh", CT_TRIANGLE_MESH },
		{ VUNULL }
	};
	addProperty(new VuStaticIntEnumProperty("Collision Type", mCollisionType, collisionTypeChoices))
		->setWatcher(this, &VuRigidBodyComponent::modified);

	addProperty(mpCollisionMeshAssetProperty = new VuAssetProperty<VuCollisionMeshAsset>("Collision Mesh Asset", mCollisionMeshAssetName))
		->setWatcher(this, &VuRigidBodyComponent::modified);
	mpCollisionMeshAssetProperty->enable(false);

	addProperty(mpSizeProperty = new VuVector3Property("Collision Size", mSize))
		->setWatcher(this, &VuRigidBodyComponent::modified);
	mpSizeProperty->enable(false);

	addProperty(mpRadiusProperty = new VuFloatProperty("Collision Radius", mRadius))
		->setWatcher(this, &VuRigidBodyComponent::modified);
	mpRadiusProperty->enable(false);

	addProperty(mpHeightProperty = new VuFloatProperty("Collision Height", mHeight))
		->setWatcher(this, &VuRigidBodyComponent::modified);
	mpHeightProperty->enable(false);

	addProperty(mpSurfaceTypeProperty = new VuConstStringEnumProperty("Surface Type", mSurfaceType, VuDynamics::IF()->getSurfaceTypes()))
		->setWatcher(this, &VuRigidBodyComponent::modified);
	mpSurfaceTypeProperty->enable(false);

	addProperty(new VuBitFieldProperty("Not Corona", mExtendedFlags, EXT_COL_ENGINE_NOT_CORONA));

	addProperty(new VuVector3Property("Collision Offset", mCollisionOffset))
		->setWatcher(this, &VuRigidBodyComponent::modified);

	addProperty(new VuFloatProperty("Linear Damping", mLinearDamping)) ->setWatcher(this, &VuRigidBodyComponent::modified);
	addProperty(new VuFloatProperty("Angular Damping", mAngularDamping)) ->setWatcher(this, &VuRigidBodyComponent::modified);
	addProperty(new VuFloatProperty("Linear Sleeping Threshold", mLinearSleepingThreshold)) ->setWatcher(this, &VuRigidBodyComponent::modified);
	addProperty(new VuFloatProperty("Angular Sleeping Threshold", mAngularSleepingThreshold)) ->setWatcher(this, &VuRigidBodyComponent::modified);
	addProperty(new VuBoolProperty("Initially Active", mInitiallyActive));
}

//*****************************************************************************
VuRigidBodyComponent::~VuRigidBodyComponent()
{
	removeFromWorld();
	destroyRigidBody();
}

//*****************************************************************************
void VuRigidBodyComponent::onBake()
{
	// clear existing vertex colors
	mShadowValues.deallocate();

	if (mCollisionType == CT_TRIANGLE_MESH && mpCollisionMeshAssetProperty->getAsset())
	{
		VuCollisionMeshAsset *pMesh = mpCollisionMeshAssetProperty->getAsset();

		// allocate vertex colors
		mShadowValues.resize(pMesh->getVertCount());

		// determine model matrix
		VuTransformComponent *pTransformComponent = getOwnerEntity()->getTransformComponent();
		VuMatrix modelMat = pTransformComponent->getWorldTransform();
		modelMat.translateLocal(mCollisionOffset);
		modelMat.scaleLocal(pTransformComponent->getWorldScale());

		// gather light info
		VuLightUtil::VuLightInfo lightInfo(pMesh->getAabb(), modelMat);
		VuLightUtil::gatherLightsRecursive(getOwnerEntity()->getRootEntity(), lightInfo);
		VuLightUtil::gatherOccludersRecursive(getOwnerEntity()->getRootEntity(), VUNULL, lightInfo);

		#pragma omp parallel for
		for (int iVert = 0; iVert < pMesh->getVertCount(); iVert++)
		{
			VuVector3 position = modelMat.transform(pMesh->getVert(iVert));
			VuVector3 normal(-lightInfo.mDirLightDir);
			VuVector4 color = VuLightUtil::calculateVertexColor(position, normal, lightInfo, true);

			mShadowValues[iVert] = (VUUINT8)VuRound(color.mW*255.0f);
		}
	}
}

//*****************************************************************************
void VuRigidBodyComponent::onClearBaked()
{
	mShadowValues.deallocate();
}

//*****************************************************************************
void VuRigidBodyComponent::setCollisionFlags(VUINT32 collisionFlags)
{
	mCollisionFlags = collisionFlags;
	if ( mpRigidBody )
	{
		mpRigidBody->setCollisionFlags(mCollisionFlags);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setCollisionGroup(VUINT16 group)
{
	mCollisionGroup = group;
	if ( mpRigidBody )
	{
		mpRigidBody->setCollisionGroup(mCollisionGroup);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setCollisionMask(VUINT16 mask)
{
	mCollisionMask = mask;
	if ( mpRigidBody )
	{
		mpRigidBody->setCollisionMask(mCollisionMask);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setExtendedFlags(VUUINT32 flags)
{
	mExtendedFlags = flags;
	if ( mpRigidBody )
	{
		mpRigidBody->setExtendedFlags(mExtendedFlags);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setMass(float mass)
{
	mMass = VuMax(mass, 0.0f);

	if ( mMass == 0.0f )
		mCollisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
	else
		mCollisionFlags &= ~btCollisionObject::CF_STATIC_OBJECT;

	if ( mpRigidBody )
	{
		mpRigidBody->setMassProps(mMass, calcLocalInertia());
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setCenterOfMass(const VuVector3 &centerOfMass)
{
	mCenterOfMass = centerOfMass;
	if ( mpRigidBody )
	{
		modified();
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setMotionState(btMotionState *pMotionState)
{
	mpMotionState = pMotionState;
	if ( mpRigidBody )
	{
		mpRigidBody->setMotionState(mpMotionState);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::setContactCallback(VuRigidBodyContactCallback *pCB)
{
	mpRigidBodyContactCallback = pCB;
	if ( mpRigidBody )
	{
		mpRigidBody->setContactCallback(mpRigidBodyContactCallback);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::createRigidBody()
{
	if ( mCollisionType == CT_CONVEX_HULL || mCollisionType == CT_TRIANGLE_MESH )
	{
		mpStridingMesh = new VuStridingMesh;
		mpStridingMesh->setCollisionMeshAsset(mpCollisionMeshAssetProperty->getAsset());
		if (mpStridingMesh->getCollisionMeshAsset() == VUNULL)
			return;
	}

	VUUINT8 *pShadowValues = VUNULL;

	if ( mCollisionType != CT_NONE )
	{
		VuTransformComponent *pTransformComponent = getOwnerEntity()->getTransformComponent();

		VuVector3 scale = pTransformComponent->getWorldScale();

		if ( mCollisionType == CT_SPHERE )
		{
			mpShape = new btSphereShape(mRadius);
		}
		else if ( mCollisionType == CT_BOX )
		{
			mpShape = new btBoxShape(VuDynamicsUtil::toBtVector3(0.5f*mSize));
		}
		else if ( mCollisionType == CT_CYLINDER_X )
		{
			mpShape = new btCylinderShapeX(btVector3(0.5f*mHeight*scale.mX, mRadius*scale.mY, mRadius*scale.mZ));
			scale = VuVector3(1,1,1); // cylinder scaling broken
		}
		else if ( mCollisionType == CT_CYLINDER_Y )
		{
			mpShape = new btCylinderShape(btVector3(mRadius*scale.mX, 0.5f*mHeight*scale.mY, mRadius*scale.mZ));
			scale = VuVector3(1,1,1); // cylinder scaling broken
		}
		else if ( mCollisionType == CT_CYLINDER_Z )
		{
			mpShape = new btCylinderShapeZ(btVector3(mRadius*scale.mX, mRadius*scale.mY, 0.5f*mHeight*scale.mZ));
			scale = VuVector3(1,1,1); // cylinder scaling broken
		}
		else if ( mCollisionType == CT_CONVEX_HULL )
		{
			mpShape = new btConvexTriangleMeshShape(mpStridingMesh);
		}
		else if ( mCollisionType == CT_TRIANGLE_MESH )
		{
			VUASSERT(mMass == 0.0f, "VuRigidBodyComponent::createRigidBody() moving concave objects not supported");

			btBvhTriangleMeshShape *pBvhTriangleMeshShape = new btBvhTriangleMeshShape(mpStridingMesh, true, false);
			pBvhTriangleMeshShape->setOptimizedBvh(mpStridingMesh->getCollisionMeshAsset()->getBvh());

			mpShape = pBvhTriangleMeshShape;
			mpShape->setUserPointer(mpCollisionMeshAssetProperty->getAsset());

			if (mShadowValues.size() == mpStridingMesh->getCollisionMeshAsset()->getVertCount())
				pShadowValues = &mShadowValues[0];
		}
		else
		{
			VUASSERT(0, "Unknown collision shape!");
		}

		// use compound shape to handle offsets
		VuVector3 offset = mCollisionOffset - mCenterOfMass;
		if ( offset.magSquared() > 0.0f )
		{
			mpChildShape = mpShape;
			btCompoundShape *pCompoundShape = new btCompoundShape;
			mpShape = pCompoundShape;

			btTransform xform;
			xform.setIdentity();
			xform.setOrigin(VuDynamicsUtil::toBtVector3(offset));
			pCompoundShape->addChildShape(xform, mpChildShape);
		}

		if ( scale != VuVector3(1,1,1) )
			mpShape->setLocalScaling(VuDynamicsUtil::toBtVector3(scale));

		btVector3 localInertia = calcLocalInertia();

		btRigidBody::btRigidBodyConstructionInfo info(mMass, mpMotionState, mpShape, localInertia);
		info.m_startWorldTransform = VuDynamicsUtil::toBtTransform(pTransformComponent->getWorldTransform());
		mpRigidBody = new VuRigidBody(info, getOwnerEntity(), mCollisionGroup, mCollisionMask);
		mpRigidBody->setCollisionFlags(mCollisionFlags);
		mpRigidBody->setExtendedFlags(mExtendedFlags);
		mpRigidBody->setContactCallback(mpRigidBodyContactCallback);
		mpRigidBody->setSurfaceType(mSurfaceType.c_str());
		mpRigidBody->setGravity(VuDynamicsUtil::toBtVector3(VuDynamics::IF()->getGravity()));
		mpRigidBody->setDamping(mLinearDamping, mAngularDamping);
		mpRigidBody->setSleepingThresholds(mLinearSleepingThreshold, mAngularSleepingThreshold);
		if ( !mInitiallyActive )
			mpRigidBody->setDeactivationTime(10.0f);
		mpRigidBody->setShadowValues(pShadowValues);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::destroyRigidBody()
{
	delete mpRigidBody;
	delete mpShape;
	delete mpChildShape;
	mpRigidBody = VUNULL;
	mpShape = VUNULL;
	mpChildShape = VUNULL;

	delete mpStridingMesh;
	mpStridingMesh = VUNULL;
}

//*****************************************************************************
void VuRigidBodyComponent::addToWorld()
{
	if ( !mbAdded && mpRigidBody )
	{
		VuDynamics::IF()->addRigidBody(mpRigidBody);
		mbAdded = true;
	}
}

//*****************************************************************************
void VuRigidBodyComponent::removeFromWorld()
{
	if ( mbAdded )
	{
		VuDynamics::IF()->removeRigidBody(mpRigidBody);
		mbAdded = false;
	}
}

//*****************************************************************************
void VuRigidBodyComponent::transformModified(const VuMatrix &xform)
{
	if ( mpRigidBody )
	{
		VuMatrix matRB = xform;
		matRB.translateLocal(mCenterOfMass);

		// synchronize rigid body
		mpRigidBody->setLinearVelocity(VuDynamicsUtil::toBtVector3(VuVector3(0,0,0)));
		mpRigidBody->setAngularVelocity(VuDynamicsUtil::toBtVector3(VuVector3(0,0,0)));
		mpRigidBody->setCenterOfMassTransform(VuDynamicsUtil::toBtTransform(matRB));
	}
}

//*****************************************************************************
void VuRigidBodyComponent::scaleModified(const VuVector3 &scale)
{
	if ( mpRigidBody )
	{
		mpRigidBody->getCollisionShape()->setLocalScaling(VuDynamicsUtil::toBtVector3(scale));
		mpRigidBody->setMassProps(mMass, calcLocalInertia());

		if ( mbAdded )
			VuDynamics::IF()->getDynamicsWorld()->updateSingleAabb(mpRigidBody);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::onMotionUpdate(VuMotionComponent *pMotionComponent)
{
	if ( mpRigidBody && pMotionComponent )
	{
		mpRigidBody->setLinearVelocity(VuDynamicsUtil::toBtVector3(pMotionComponent->getWorldLinearVelocity()));
		mpRigidBody->setAngularVelocity(VuDynamicsUtil::toBtVector3(pMotionComponent->getWorldAngularVelocity()));
		mpRigidBody->setCenterOfMassTransform(VuDynamicsUtil::toBtTransform(pMotionComponent->getWorldTransform()));

		if ( mbAdded )
			VuDynamics::IF()->getDynamicsWorld()->updateSingleAabb(mpRigidBody);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::draw(const VuColor &color, const VuCamera &camera)
{
	if ( mCollisionType == CT_NONE )
		return;

	if ( VuTransformComponent *pTransformComponent = getOwnerEntity()->getTransformComponent() )
	{
		VuMatrix mat = pTransformComponent->getWorldTransform();
		mat.scaleLocal(pTransformComponent->getWorldScale());
		mat.translateLocal(mCollisionOffset);

		if ( mCollisionType == CT_SPHERE )
		{
			VuGfxUtil::IF()->drawSphereSolid(color, mRadius, 16, 16, mat, camera.getViewProjMatrix());
		}
		else if ( mCollisionType == CT_BOX )
		{
			VuGfxUtil::IF()->drawAabbSolid(color, VuAabb(-0.5f*mSize, 0.5f*mSize), mat, camera.getViewProjMatrix());
		}
		else if ( mCollisionType == CT_CYLINDER_X || mCollisionType == CT_CYLINDER_Y || mCollisionType == CT_CYLINDER_Z )
		{
			if ( mCollisionType == CT_CYLINDER_X )
				mat.rotateYLocal(VU_PIDIV2);
			if ( mCollisionType == CT_CYLINDER_Y )
				mat.rotateXLocal(VU_PIDIV2);
			VuGfxUtil::IF()->drawCylinderSolid(color, mHeight, mRadius, 16, mat, camera.getViewProjMatrix());
		}
		else if ( mCollisionType == CT_CONVEX_HULL || mCollisionType == CT_TRIANGLE_MESH )
		{
			if ( VuCollisionMeshAsset *pAsset = mpCollisionMeshAssetProperty->getAsset() )
				pAsset->drawWithColors(mat);
		}
	}
}

//*****************************************************************************
void VuRigidBodyComponent::modified()
{
	mpCollisionMeshAssetProperty->enable(false);
	mpSurfaceTypeProperty->enable(true);
	mpRadiusProperty->enable(false);
	mpSizeProperty->enable(false);
	mpHeightProperty->enable(false);

	if ( mCollisionType == CT_SPHERE )
	{
		mpRadiusProperty->enable(true);
	}
	else if ( mCollisionType == CT_BOX )
	{
		mpSizeProperty->enable(true);
	}
	else if ( mCollisionType == CT_CYLINDER_X || mCollisionType == CT_CYLINDER_Y || mCollisionType == CT_CYLINDER_Z )
	{
		mpRadiusProperty->enable(true);
		mpHeightProperty->enable(true);
	}
	else if ( mCollisionType == CT_CONVEX_HULL )
	{
		mpCollisionMeshAssetProperty->enable(true);
	}
	else if ( mCollisionType == CT_TRIANGLE_MESH )
	{
		mpCollisionMeshAssetProperty->enable(true);
		mpSurfaceTypeProperty->enable(false);
	}

	bool mbWasCreated = mpRigidBody ? true : false;
	bool mbWasAdded = mbAdded;

	removeFromWorld();
	destroyRigidBody();

	if ( mbWasCreated )
		createRigidBody();

	if ( mbWasAdded )
		addToWorld();
}

//*****************************************************************************
btVector3 VuRigidBodyComponent::calcLocalInertia()
{
	btVector3 localInertia(0,0,0);

	if ( mMass > 0.0f && mpShape )
		mpShape->calculateLocalInertia(mMass, localInertia);

	return localInertia;
}

//*****************************************************************************
void VuRigidBodyComponent::loadShadowValues(const VuJsonContainer &data)
{
	const void *pData;
	int size;
	if (data["Shadow"].getValue(pData, size))
	{
		if ( mShadowValues.size() )
			VUPRINTF("Warning:  shadow values loaded more than once for %s\n", getOwnerEntity()->getLongName().c_str());

		mShadowValues.deallocate();

		mShadowValues.resize(size);
		VU_MEMCPY(&mShadowValues[0], size, pData, size);
	}
}

//*****************************************************************************
void VuRigidBodyComponent::saveShadowValues(VuJsonContainer &data) const
{
	if (mShadowValues.size())
	{
		data["Shadow"].putValue(&mShadowValues[0], mShadowValues.size());
	}
}
