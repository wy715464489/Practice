//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Camera class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuFrustum.h"

class VuAabb;


class VuCamera
{
public:
	VuCamera();

	void				setProjMatrixHorz(float fFovHorz, float fAspectRatio, float fNear, float fFar, bool bUpdateFrustum = true);
	void				setProjMatrixVert(float fFovVert, float fAspectRatio, float fNear, float fFar, bool bUpdateFrustum = true);
	void				setViewMatrix(const VuVector3 &vEye, const VuVector3 &vTarget, const VuVector3 &vUp);
	void				setListenerVelocity(const VuVector3 &vVel);

	const VuVector3		&getEyePosition() const		{ return mEyePosition; }
	const VuVector3		&getTargetPosition() const	{ return mTargetPosition; }
	const VuVector3		&getListenerVelocity() const{ return mListenerVelocity; }
	const VuMatrix		&getProjMatrix() const		{ return mProjMatrix; }
	const VuMatrix		&getViewMatrix() const		{ return mViewMatrix; }
	const VuMatrix		&getViewProjMatrix() const	{ return mViewProjMatrix; }
	float				getFovHorz() const			{ return mFovHorz; }
	float				getFovVert() const			{ return mFovVert; }
	float				getAspectRatio() const		{ return mAspectRatio; }
	float				getNearPlane() const		{ return mNearPlane; }
	float				getFarPlane() const			{ return mFarPlane; }

	const VuMatrix		&getTransform() const		{ return mTransform; }

	const VuFrustum		&getFrustum() const			{ return mFrustum; }
	const VuVector4		*getFrustumPlanes() const	{ return mFrustumPlanes; }	// near/far/left/right/top/bottom

	VuVector3			worldToScreen(const VuVector3 &world) const;
	VuVector3			screenToWorld(const VuVector3 &screen) const;

	bool				isSphereVisible(const VuVector3 &vPosWorld, float fRadius) const;
	bool				isAabbVisible(const VuAabb &aabb, const VuMatrix &matWorld) const;

	void				getMinEnclosingSphere(VuVector3 &vPos, float &fRadius, float fNear = 0, float fFar = 1) const;

	void				screenShotShear(int ix, int iy, int nx, int ny);
	float				getScreenShotScale() const { return mScreenShotScale; }

private:
	void		updateFrustum();

	VuVector3	mEyePosition;
	VuVector3	mTargetPosition;
	VuVector3	mListenerVelocity;
	VuMatrix	mProjMatrix;
	VuMatrix	mViewMatrix;
	VuMatrix	mViewProjMatrix;
	VuMatrix	mInverseViewMatrix;
	VuMatrix	mTransform;
	float		mFovHorz;
	float		mFovVert;
	float		mAspectRatio;
	float		mNearPlane;
	float		mFarPlane;
	float		mTanHalfFovHorz;
	float		mTanHalfFovVert;
	VuFrustum	mFrustum;
	VuVector4	mFrustumPlanes[6]; // near/far/left/right/top/bottom
	float		mScreenShotScale;
};