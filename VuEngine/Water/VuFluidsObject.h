//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  FluidsObject class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuArray.h"

class VuFluidsMeshAsset;
class btRigidBody;
class VuMatrix;
class VuAabb;
class VuWaterWave;


class VuFluidsObject
{
public:
	struct TransformedVert
	{
		VuVector3	mPos;
		VuVector3	mWaterVel;
		float		mWaterHeight;
		VUUINT32	mSubmerged;
	};
	struct TransformedEdge
	{
		VuVector3	mIntersectionPos;
		VuVector3	mIntersectionWaterVel;
		float		mIntersectionWaterHeight;
		VUUINT32	mPartiallySubmerged;
	};

	VuFluidsObject();
	~VuFluidsObject();

	void	setAsset(const std::string &assetName);
	void	setHydrodynamicCenter(const VuVector3 &vHydrodynamicCenter)	{ mHydrodynamicCenter = vHydrodynamicCenter; }
	void	setWaterDragDensity(float fWaterDragDensity)				{ mWaterDragDensity = fWaterDragDensity; }
	void	setWaterBuoyancyDensity(float fWaterBuoyancyDensity)		{ mWaterBuoyancyDensity = fWaterBuoyancyDensity; }
	void	setLinearVelocityFactor(float fLinVelFactor)				{ mLinVelFactor = fLinVelFactor; }
	void	setSkinFrictionCoeff(const VuVector3 &vSkinFrictionCoeff)	{ mSkinFrictionCoeff = vSkinFrictionCoeff; }
	void	setIgnoreWaveCount(int count);
	void	setIgnoreWave(int index, const VuWaterWave *pWave)			{ mppIgnoreWaves[index] = pWave; }
	void	setKillDownwardPressure(bool bKill)							{ mMinPressureForce = bKill ? 0.0f : -FLT_MAX; }
	void	setTransform(const VuMatrix &matModel);
	void	updateForces(float fdt, btRigidBody &rb);
	bool	isSubmerged() const			{ return mSubmerged; }
	float	getSubmergedVolume() const	{ return mSubmergedVolume; }
	float	getSubmergedArea() const	{ return mSubmergedArea; }
	float	getMinWaterHeight() const	{ return mMinWaterHeight; }
	void	getAabb(VuAabb &aabb) const;

	const VuVector3	&getTotalForce() const		{ return mTotalForce; }
	const VuVector3	&getTotalTorque() const		{ return mTotalTorque; }
	const VuVector3	&getAvgWaterNormal() const	{ return mAvgWaterNormal; }
	const VuVector3	&getAvgWaterVel() const		{ return mAvgWaterVel; }

	const VuFluidsMeshAsset	*getFluidsMeshAsset() const		{ return mpFluidsMeshAsset; }
	const TransformedVert	*getTransformedVerts() const	{ return &mTransformedVerts[0]; }
	const TransformedEdge	*getTransformedEdges() const	{ return &mTransformedEdges[0]; }

private:

	typedef VuArray<TransformedVert> TransformedVerts;
	typedef VuArray<TransformedEdge> TransformedEdges;

	VuFluidsMeshAsset	*mpFluidsMeshAsset;
	VuVector3			mHydrodynamicCenter;
	float				mWaterDragDensity;
	float				mWaterBuoyancyDensity;
	float				mLinVelFactor;
	VuVector3			mSkinFrictionCoeff;
	int					mIgnoreWaveCount;
	const VuWaterWave	**mppIgnoreWaves;
	TransformedVerts	mTransformedVerts;
	TransformedEdges	mTransformedEdges;
	bool				mSubmerged;
	float				mSubmergedVolume;
	float				mSubmergedArea;
	float				mMinWaterHeight;
	VuVector3			mTotalForce;
	VuVector3			mTotalTorque;
	VuVector3			mBuoyancyForce;
	VuVector3			mBuoyancyTorque;
	VuVector3			mDragForce;
	VuVector3			mDragTorque;
	VuVector3			mDampingForce;
	VuVector3			mDampingTorque;
	VuVector3			mAvgWaterNormal;
	VuVector3			mAvgWaterVel;
	float				mMinPressureForce; // either 0 or -FLT_MAX, specified by killDownwardPressure()
};
