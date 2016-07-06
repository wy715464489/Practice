//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxDrawParams
// 
//*****************************************************************************

#include "VuGfxDrawParams.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
inline void CalcSphere(const VuAabb &aabb, const VuMatrix &matWorld, VuVector3 &vPos, float &fRadius)
{
	// transform aabb to world space (applies scaling too)
	VuVector3 vMinWorld = matWorld.transform(aabb.mMin);
	VuVector3 vMaxWorld = matWorld.transform(aabb.mMax);

	// calculate sphere
	vPos = 0.5f*(vMinWorld + vMaxWorld);
	fRadius = 0.5f*(vMaxWorld - vMinWorld).mag();
}

//*****************************************************************************
VuGfxDrawParams::VuGfxDrawParams(const VuCamera &camera) :
	mEyePos(camera.getEyePosition()),
	mCamera(camera),
	mRejectionScale(0),
	mbDrawReflection(false),
	mReflectionPlane(0,0,0,0),
	mbDrawSSAO(false),
	mZoneMask((VUUINT32)~0)
{
}

//*****************************************************************************
bool VuGfxDrawParams::isVisible(const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const
{
	// calculate visibility sphere (applies scaling too)
	VuVector3 vPos;
	float fRadius;
	CalcSphere(aabb, matWorld, vPos, fRadius);

	// scale rejection
	float rejectionScale = mRejectionScale*rejectionScaleModifier;
	if ( (fRadius*fRadius)/(mEyePos - vPos).magSquared() < rejectionScale*rejectionScale )
		return false;
	
	// consider reflection plane
	if ( VuMathUtil::distPointPlane(vPos, mReflectionPlane) + fRadius <= 0 )
		return false;
	
	// for now do a simple sphere test
	return mCamera.isSphereVisible(vPos, fRadius);
}

//*****************************************************************************
VuGfxDrawShadowParams::VuGfxDrawShadowParams(const VuCamera &camera, const VuShadowClip &combinedShadowClip) :
	mEyePos(camera.getEyePosition()),
	mCamera(camera),
	mCombinedShadowClip(combinedShadowClip),
	mShadowVolumeCount(0),
	mpShadowVolumes(VUNULL),
	mRejectionScale(0),
	mbDrawReflection(false),
	mReflectionPlane(0,0,0,0),
	mbDrawCollision(false),
	mZoneMask((VUUINT32)~0)
{
}

//*****************************************************************************
bool VuGfxDrawShadowParams::isVisible(const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const
{
	// calculate visibility sphere (applies scaling too)
	VuVector3 vPos;
	float fRadius;
	CalcSphere(aabb, matWorld, vPos, fRadius);

	// scale rejection
	float rejectionScale = mRejectionScale*rejectionScaleModifier;
	if ( (fRadius*fRadius)/(mEyePos - vPos).magSquared() < rejectionScale*rejectionScale )
		return false;

	// consider reflection plane
	if ( VuMathUtil::distPointPlane(vPos, mReflectionPlane) + fRadius <= 0 )
		return false;
	
	// for now do a simple sphere test
	return mCombinedShadowClip.isSphereVisible(vPos, fRadius);
}

//*****************************************************************************
bool VuGfxDrawShadowParams::isVisible(int volume, const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const
{
	// calculate visibility sphere (applies scaling too)
	VuVector3 vPos;
	float fRadius;
	CalcSphere(aabb, matWorld, vPos, fRadius);

	// scale rejection
	float rejectionScale = mRejectionScale*rejectionScaleModifier;
	if ( (fRadius*fRadius)/(mEyePos - vPos).magSquared() < rejectionScale*rejectionScale )
		return false;
	
	// consider reflection plane
	if ( VuMathUtil::distPointPlane(vPos, mReflectionPlane) + fRadius <= 0 )
		return false;
	
	// for now do a simple sphere test
	return mpShadowVolumes[volume].isSphereVisible(vPos, fRadius);
}

//*****************************************************************************
VuGfxDrawInfoParams::VuGfxDrawInfoParams(const VuCamera &pCamera):
	mFlags(0),
	mCamera(pCamera),
	mDevTextColor(255,255,255,255),
	mDevLineColor(128,128,128,128),
	mBoneSize(1.0f)
{
}
