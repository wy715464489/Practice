//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Camera class
// 
//*****************************************************************************

#include <math.h>
#include "VuCamera.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
VuCamera::VuCamera():
	mEyePosition(0,0,0),
	mTargetPosition(0,0,0),
	mListenerVelocity(0,0,0),
	mFovHorz(0),
	mFovVert(0),
	mAspectRatio(1),
	mNearPlane(1),
	mFarPlane(2),
	mTanHalfFovHorz(0),
	mTanHalfFovVert(0),
	mScreenShotScale(1.0f)
{
	mProjMatrix.loadIdentity();
	mViewMatrix.loadIdentity();
	mViewProjMatrix.loadIdentity();
	mInverseViewMatrix.loadIdentity();

	mTransform = VuMatrix(
		VuVector4( 1,  0,  0,  0),
		VuVector4( 0,  0, -1,  0),
		VuVector4( 0,  1,  0,  0),
		VuVector4( 0,  0,  0,  1)
	);
}

//*****************************************************************************
void VuCamera::setProjMatrixHorz(float fFovHorz, float fAspectRatio, float fNear, float fFar, bool bUpdateFrustum)
{
	mFovHorz = fFovHorz;
	mAspectRatio = fAspectRatio;
	mNearPlane = fNear;
	mFarPlane = fFar;

	mTanHalfFovHorz = VuTan(0.5f*fFovHorz);
	mTanHalfFovVert = mTanHalfFovHorz/fAspectRatio;

	mFovVert = 2.0f*VuATan(mTanHalfFovVert);

	float fScaleX = 1.0f/mTanHalfFovHorz;
	float fScaleY = 1.0f/mTanHalfFovVert;
	float fQ = fFar/(fNear - fFar);

	mProjMatrix = VuMatrix(
		VuVector4(fScaleX,       0,        0,        0),
		VuVector4(      0, fScaleY,        0,        0),
		VuVector4(      0,       0,       fQ,       -1),
		VuVector4(      0,       0, fNear*fQ,        0)
	);

	mViewProjMatrix = mViewMatrix*mProjMatrix;

	if ( bUpdateFrustum )
	{
		updateFrustum();
	}

	mScreenShotScale = 1.0f;
}

//*****************************************************************************
void VuCamera::setProjMatrixVert(float fFovVert, float fAspectRatio, float fNear, float fFar, bool bUpdateFrustum)
{
	mFovVert = fFovVert;
	mAspectRatio = fAspectRatio;
	mNearPlane = fNear;
	mFarPlane = fFar;

	mTanHalfFovVert = VuTan(0.5f*fFovVert);
	mTanHalfFovHorz = mTanHalfFovVert*fAspectRatio;

	mFovHorz = 2.0f*VuATan(mTanHalfFovHorz);

	float fScaleX = 1.0f/mTanHalfFovHorz;
	float fScaleY = 1.0f/mTanHalfFovVert;
	float fQ = fFar/(fNear - fFar);

	mProjMatrix = VuMatrix(
		VuVector4(fScaleX,       0,        0,        0),
		VuVector4(      0, fScaleY,        0,        0),
		VuVector4(      0,       0,       fQ,       -1),
		VuVector4(      0,       0, fNear*fQ,        0)
	);

	mViewProjMatrix = mViewMatrix*mProjMatrix;

	if ( bUpdateFrustum )
	{
		updateFrustum();
	}

	mScreenShotScale = 1.0f;
}

//*****************************************************************************
void VuCamera::setViewMatrix(const VuVector3 &vEye, const VuVector3 &vTarget, const VuVector3 &vUp)
{
	VuVector3 vAxisForward = (vTarget - vEye);
	VuVector3 vAxisRight = VuCross(vAxisForward, vUp);
	VuVector3 vAxisUp = VuCross(vAxisRight, vAxisForward);

	if ( vAxisForward.magSquared() < FLT_EPSILON || vAxisRight.magSquared() < FLT_EPSILON || vAxisUp.magSquared() < FLT_EPSILON )
	{
		vAxisRight = VuVector3(1,0,0);
		vAxisForward = VuVector3(0,1,0);
		vAxisUp = VuVector3(0,0,1);
	}

	vAxisRight = vAxisRight.normal();
	vAxisForward = vAxisForward.normal();
	vAxisUp = vAxisUp.normal();

	mInverseViewMatrix.loadIdentity();
	mInverseViewMatrix.setAxisX(vAxisRight);
	mInverseViewMatrix.setAxisY(vAxisUp);
	mInverseViewMatrix.setAxisZ(-vAxisForward);
	mInverseViewMatrix.setTrans(vEye);

	mViewMatrix = mInverseViewMatrix;
	mViewMatrix.invert();

	mViewProjMatrix = mViewMatrix*mProjMatrix;
	mEyePosition = vEye;
	mTargetPosition = vTarget;

	mTransform.loadIdentity();
	mTransform.setAxisX(vAxisRight);
	mTransform.setAxisY(vAxisForward);
	mTransform.setAxisZ(vAxisUp);
	mTransform.setTrans(vEye);

	updateFrustum();
}

//*****************************************************************************
void VuCamera::setListenerVelocity(const VuVector3 &vVel)
{
	mListenerVelocity = vVel;
}

//*****************************************************************************
VuVector3 VuCamera::worldToScreen(const VuVector3 &world) const
{
	float fScaleX = mProjMatrix.getAxisX().mX;
	float fScaleY = mProjMatrix.getAxisY().mY;

	// transform
	VuVector3 screen = mViewMatrix.transform(world);

	// project
	screen.mX = -screen.mX*fScaleX / screen.mZ * 0.5f + 0.5f;
	screen.mY =  screen.mY*fScaleY / screen.mZ * 0.5f + 0.5f;
	screen.mZ = ((-screen.mZ) - mNearPlane)/(mFarPlane - mNearPlane);

	return screen;
}

//*****************************************************************************
VuVector3 VuCamera::screenToWorld(const VuVector3 &screen) const
{
	float fScaleX = mProjMatrix.getAxisX().mX;
	float fScaleY = mProjMatrix.getAxisY().mY;

	VuVector3 world;

	// unproject
	world.mZ = -(mNearPlane + screen.mZ*(mFarPlane - mNearPlane));
	world.mY =  (2.0f*screen.mY - 1.0f)*world.mZ/fScaleY;
	world.mX = -(2.0f*screen.mX - 1.0f)*world.mZ/fScaleX;

	// transform
	world = mInverseViewMatrix.transform(world);

	return world;
}

//*****************************************************************************
bool VuCamera::isSphereVisible(const VuVector3 &vPosWorld, float fRadius) const
{
	VUUINT32 fail = 0x0;

	for ( int i = 0; i < 6; i++ )
	{
		float dist = VuMathUtil::distPointPlane(vPosWorld, mFrustumPlanes[i]);
		dist += fRadius;
		fail |= (int&)(dist)&0x80000000;
	}

	return !fail;
}

//*****************************************************************************
bool VuCamera::isAabbVisible(const VuAabb &aabb, const VuMatrix &matWorld) const
{
	// transform aabb to world space (applies scaling too)
	VuVector3 vMinWorld = matWorld.transform(aabb.mMin);
	VuVector3 vMaxWorld = matWorld.transform(aabb.mMax);
	
	// for now do a simple sphere test
	return isSphereVisible(0.5f*(vMinWorld + vMaxWorld), 0.5f*(vMaxWorld - vMinWorld).mag());
}

//*****************************************************************************
void VuCamera::getMinEnclosingSphere(VuVector3 &vPos, float &fRadius, float fNear, float fFar) const
{
	VuVector3 vNear = screenToWorld(VuVector3(0.5, 0.5, fNear));
	VuVector3 vFar = screenToWorld(VuVector3(0.5, 0.5, fFar));

	float A = (screenToWorld(VuVector3(0, 0, fNear)) - vNear).magSquared();
	float B = (screenToWorld(VuVector3(0, 0, fFar)) - vFar).magSquared();
	float C = (vFar - vNear).magSquared();

	if ( A + C < B )
	{
		vPos = vFar;
		fRadius = VuSqrt(B);
	}
	else
	{
		float x = (C + B - A)/(2*VuSqrt(C));

		vPos = vNear + x*(vFar - vNear).normal();
		fRadius = VuSqrt(x*x + A);
	}
}

//*****************************************************************************
void VuCamera::screenShotShear(int ix, int iy, int nx, int ny)
{
	float nearWidth = 2.0f*mFrustum.mRBound;
	float nearHeight = 2.0f*mFrustum.mUBound;

	VuMatrix shearMat;
	shearMat.loadIdentity();
	shearMat.set(2, 0, (ix - (nx - 1) * 0.5f) * nearWidth / mNearPlane);
	shearMat.set(2, 1, -(iy - (ny - 1) * 0.5f) * nearHeight / mNearPlane);

	VuMatrix scaleMat;
	scaleMat.loadIdentity();
	scaleMat.set(0,0,(float)nx);
	scaleMat.set(1,1,(float)ny);

	mProjMatrix = scaleMat*shearMat*mProjMatrix;
	mViewProjMatrix = mViewMatrix*mProjMatrix;

	mScreenShotScale = float(nx*ny);
}

//*****************************************************************************
void VuCamera::updateFrustum()
{
	mFrustum.mO = mTransform.getTrans();
	mFrustum.mD = mTransform.getAxisY();
	mFrustum.mU = mTransform.getAxisZ();
	mFrustum.mR = mTransform.getAxisX();

	mFrustum.mDMin = mNearPlane;
	mFrustum.mDMax = mFarPlane;
	mFrustum.mUBound = mNearPlane*mTanHalfFovVert;
	mFrustum.mRBound = mNearPlane*mTanHalfFovHorz;

	mFrustum.update();

	mFrustum.getPlanes(mFrustumPlanes);
}
