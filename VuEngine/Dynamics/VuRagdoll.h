//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ragdoll class
// 
//*****************************************************************************

#pragma once

#include "btBulletDynamicsCommon.h"
#include "VuEngine/Dynamics/VuDynamicsCallbacks.h"
#include "VuEngine/Dynamics/VuCollisionTypes.h"
#include "VuEngine/Math/VuMatrix.h"

class VuSkeleton;
class VuJsonContainer;
class VuAnimationTransform;
class VuRigidBody;
class VuCamera;
class VuColor;
class VuEntity;


class VuRagdoll: public VuDynamicsStepCallback
{
public:
	struct Body;

	VuRagdoll(btDynamicsWorld *pDynamicsWorld = VUNULL);
	~VuRagdoll();

	struct Params
	{
		Params() : mCollisionMask(COL_EVERYTHING), mWaterSimulation(false) {}
		VUUINT16	mCollisionMask;
		bool		mWaterSimulation;
	};

	bool				configure(VuSkeleton *pSkeleton, const VuJsonContainer &data, VuEntity *pEntity, const Params &params);
	void				clear();

	void				attachTo(VuRigidBody *pRigidBody, const VuVector3 &offset, const char *bodyName);

	void				startSimulation(const VuMatrix &modelMat, const VuAnimationTransform *pLocalPose, const VuVector3 &linVel, const VuVector3 &angVel);
	void				stopSimulation();

	bool				isActive() const { return mActive; }
	const VuSkeleton	*getSkeleton() const { return mpSkeleton; }
	void				updateModelMatrices(const VuMatrix &modelMat, VuMatrix *pModelMatrices) const;

	int					getBodyIndex(const char *bodyName) const;
	int					getBodyCount() const { return (int)mBodies.size(); }
	const Body			&getBody(int index) const { return mBodies[index]; }

	int					getConstraintCount() const { return (int)mConstraints.size(); }
	btTypedConstraint	*getConstraint(int index) const { return mConstraints[index]; }
	int					getTypedConstraintIndex(int index) const;
	int					getConstraintIndex(btTypedConstraintType type, int typedIndex) const;

	void				drawDebugBodies(const VuCamera &camera, const VuColor &color) const;

	struct Body : public btMotionState
	{
		std::string		mName;
		int				mBoneIndex;
		float			mRadius;
		float			mHeight;
		VuMatrix		mTransform;
		VuMatrix		mInvTransform;
		VuRigidBody		*mpRigidBody;
		float			mFluidsDensity;
		float			mSubmergedRatio;
		VuMatrix		mWorldTransform;

		virtual void	getWorldTransform(btTransform& worldTrans) const;
		virtual void	setWorldTransform(const btTransform& worldTrans);
	};

private:
	// VuDynamicsStepCallback
	virtual void		onDynamicsApplyForces(float fdt);

	static void			getTransformData(float scale, const VuJsonContainer &posData, const VuJsonContainer &rotData, VuMatrix &xform);

	struct FixedBone
	{
		int			mIndex;
		VuMatrix	mLocalMatrix;
	};
	typedef std::vector<Body> Bodies;
	typedef std::vector<btTypedConstraint *> Constraints;
	typedef std::vector<FixedBone> FixedBones;

	Params			mParams;
	btDynamicsWorld	*mpDynamicsWorld;
	VuSkeleton		*mpSkeleton;
	Bodies			mBodies;
	Constraints		mConstraints;
	FixedBones		mFixedBones;
	float			mFluidsDensityModifier;
	float			mFluidsLiftCoeff;
	bool			mActive;
	float			mScale;
};
