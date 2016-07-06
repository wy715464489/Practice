//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicLight class.
// 
//*****************************************************************************

#include "VuDynamicLight.h"
#include "VuLightManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
VuDynamicLight::VuDynamicLight():
	mPosition(0,0,0),
	mDirection(0,0,-1),
	mDiffuseColor(255,255,255),
	mSpecularColor(255,255,255),
	mFactor(1.0f),
	mFalloffRangeMin(30.0f),
	mFalloffRangeMax(50.0f),
	mConeAngle(VU_PIDIV2),
	mPenumbraAngle(0.0f),
	mDrawDistance(FLT_MAX),
	mbReflecting(false),
	mViewportMask(0xf),
	mGroup(1),
	mbRegistered(false),
	mpDbvtNode(VUNULL)
{
	update();
}

//*****************************************************************************
VuDynamicLight::~VuDynamicLight()
{
	turnOff();
}

//*****************************************************************************
void VuDynamicLight::turnOn()
{
	if ( !mbRegistered )
	{
		VuLightManager::IF()->addDynamicLight(this);
		mbRegistered = true;
	}
}

//*****************************************************************************
void VuDynamicLight::turnOff()
{
	if ( mbRegistered )
	{
		VuLightManager::IF()->removeDynamicLight(this);
		mbRegistered = false;
	}
}

//*****************************************************************************
void VuDynamicLight::update()
{
	// recalc aabb
	VuVector3 vRange(mFalloffRangeMax, mFalloffRangeMax, mFalloffRangeMax);
	mAabb.mMin = mPosition - vRange;
	mAabb.mMax = mPosition + vRange;

	// update scene graph
	if ( mbRegistered )
		VuLightManager::IF()->updateDynamicLight(this);

	// update render light
	{
		float minAngle = VuMin(0.5f*mConeAngle, 0.5f*mConeAngle + mPenumbraAngle);
		float maxAngle = VuMax(0.5f*mConeAngle, 0.5f*mConeAngle + mPenumbraAngle);
		float minDist = VuMin(mFalloffRangeMin, mFalloffRangeMax);
		float maxDist = VuMax(mFalloffRangeMin, mFalloffRangeMax);

		mRenderLight.mPosition.mX = mPosition.mX;
		mRenderLight.mPosition.mY = mPosition.mY;
		mRenderLight.mPosition.mZ = mPosition.mZ;
		mRenderLight.mPosition.mW = 1.0f;
		mRenderLight.mDirection.mX = mDirection.mX;
		mRenderLight.mDirection.mY = mDirection.mY;
		mRenderLight.mDirection.mZ = mDirection.mZ;
		mRenderLight.mDirection.mW = 0.0f;
		mDiffuseColor.toVector4(mRenderLight.mDiffuseColor);
		mRenderLight.mDiffuseColor *= mFactor;
		mSpecularColor.toVector4(mRenderLight.mSpecularColor);
		mRenderLight.mSpecularColor *= mFactor;
		mRenderLight.mRange.mX = minDist;
		mRenderLight.mRange.mY = maxDist + FLT_EPSILON;
		mRenderLight.mRange.mZ = VuCos(minAngle) + FLT_EPSILON;
		mRenderLight.mRange.mW = VuCos(maxAngle);
		mRenderLight.mGroup = mGroup;
	}
}

//*****************************************************************************
void VuRenderLight::debugDraw(const VuMatrix &viewProjMatrix) const
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuMatrix transform;
	VuMathUtil::buildOrientationMatrix(VuVector3(mDirection.mX, mDirection.mY, mDirection.mZ), VuVector3(0,0,1), transform);
	transform.setTrans(VuVector3(mPosition.mX, mPosition.mY, mPosition.mZ));

	float radius = mRange.mY;
	VuColor color(128, 255, 128);

	// draw center arrow
	{
		float head = 0.25f*radius;

		VuMatrix mat = transform;

		pGfxUtil->drawArrowLines(color, radius, head, head, mat*viewProjMatrix);

		mat.rotateYLocal(VuDegreesToRadians(-90.0f));
		pGfxUtil->drawArrowLines(color, radius, head, head, mat*viewProjMatrix);
	}

	// draw penumbra
	{
		VuMatrix mat = transform;
		mat.scaleLocal(VuVector3(radius, radius, radius));

		float cosMinAngle = mRange.mZ;
		float cosMaxAngle = mRange.mW;
		float sinMinAngle = VuSqrt(1.0f - VuSquare(cosMinAngle));
		float sinMaxAngle = VuSqrt(1.0f - VuSquare(cosMaxAngle));

		pGfxUtil->drawArcLines(color, VuVector3(0,cosMinAngle,0), VuVector3(0,1,0), VuVector3(1,0,0), 0.0f, VU_2PI, sinMinAngle, 16, false, mat*viewProjMatrix);
		pGfxUtil->drawArcLines(color, VuVector3(0,cosMaxAngle,0), VuVector3(0,1,0), VuVector3(1,0,0), 0.0f, VU_2PI, sinMaxAngle, 16, false, mat*viewProjMatrix);
	}
}