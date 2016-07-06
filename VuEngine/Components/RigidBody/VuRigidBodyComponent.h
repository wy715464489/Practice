//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RigidBodyComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Dynamics/VuDynamics.h"

class VuConstStringEnumProperty;
class VuMotionComponent;
class VuCamera;
class VuCollisionMeshAsset;
class VuStridingMesh;


class VuRigidBodyComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(RigidBody)
	DECLARE_RTTI

public:
	VuRigidBodyComponent(VuEntity *pOwnerEntity);
	~VuRigidBodyComponent();

	virtual void	onLoad(const VuJsonContainer &data) { loadShadowValues(data); }
	virtual void	onSave(VuJsonContainer &data) const { saveShadowValues(data); }
	virtual void	onPostLoad() { modified(); }

	virtual void	onBake();
	virtual void	onClearBaked();

	void			setCollisionFlags(VUINT32 collisionFlags);
	void			setCollisionGroup(VUINT16 group);
	void			setCollisionMask(VUINT16 mask);
	void			setExtendedFlags(VUUINT32 flags);
	void			setMass(float mass);
	void			setCenterOfMass(const VuVector3 &centerOfMass);
	void			setMotionState(btMotionState *pMotionState);
	void			setContactCallback(VuRigidBodyContactCallback *pCB);

	VUINT32			getCollisionFlags() const	{ return mCollisionFlags; }
	VUINT16			getCollisionGroup() const	{ return mCollisionGroup; }
	VUINT16			getCollisionMask() const	{ return mCollisionMask; }
	VUUINT32		getExtendedFlags() const	{ return mExtendedFlags; }
	const VuVector3	&getCenterOfMass() const	{ return mCenterOfMass; }
	const VuVector3	&getCollisionOffset() const	{ return mCollisionOffset; }

	void			createRigidBody();
	void			destroyRigidBody();
	void			addToWorld();
	void			removeFromWorld();

	VuRigidBody		*getRigidBody()	{ return mpRigidBody; }

	void			transformModified(const VuMatrix &xform);
	void			scaleModified(const VuVector3 &scale);
	void			onMotionUpdate(VuMotionComponent *pMotionComponent);

	void			draw(const VuColor &color, const VuCamera &camera);

	VUUINT8			getShadowValues(int index) { return mShadowValues[index]; }

private:
	void			modified();
	btVector3		calcLocalInertia();

	void			loadShadowValues(const VuJsonContainer &data);
	void			saveShadowValues(VuJsonContainer &data) const;

	enum eCollisionType { CT_NONE, CT_SPHERE, CT_BOX, CT_CYLINDER_X, CT_CYLINDER_Y, CT_CYLINDER_Z, CT_CONVEX_HULL, CT_TRIANGLE_MESH };
	typedef VuAssetProperty<VuCollisionMeshAsset> ColAssetProperty;

	// properties
	int							mCollisionType;
	std::string					mCollisionMeshAssetName;
	float						mRadius;
	VuVector3					mSize;
	float						mHeight;
	std::string					mSurfaceType;
	VuVector3					mCollisionOffset;
	float						mLinearDamping;
	float						mAngularDamping;
	float						mLinearSleepingThreshold;
	float						mAngularSleepingThreshold;
	bool						mInitiallyActive;

	// property references
	ColAssetProperty			*mpCollisionMeshAssetProperty;
	VuProperty					*mpRadiusProperty;
	VuProperty					*mpSizeProperty;
	VuProperty					*mpHeightProperty;
	VuConstStringEnumProperty	*mpSurfaceTypeProperty;

	VuStridingMesh				*mpStridingMesh;
	VUINT32						mCollisionFlags;
	VUINT16						mCollisionGroup;
	VUINT16						mCollisionMask;
	VUUINT32					mExtendedFlags;
	VuVector3					mCenterOfMass;
	float						mMass;
	btMotionState				*mpMotionState;
	VuRigidBodyContactCallback	*mpRigidBodyContactCallback;
	btCollisionShape			*mpShape;
	btCollisionShape			*mpChildShape;
	VuRigidBody					*mpRigidBody;
	bool						mbAdded;

	// vertex colors
	typedef VuArray<VUUINT8> ShadowValues;
	ShadowValues				mShadowValues;
};
