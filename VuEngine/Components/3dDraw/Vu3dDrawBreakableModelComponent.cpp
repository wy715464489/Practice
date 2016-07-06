//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawBreakableModelComponent class
// 
//*****************************************************************************

#include "Vu3dDrawBreakableModelComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"


IMPLEMENT_RTTI(Vu3dDrawBreakableModelComponent, Vu3dDrawComponent);


//*****************************************************************************
Vu3dDrawBreakableModelComponent::Vu3dDrawBreakableModelComponent(VuEntity *pOwnerEntity):
	Vu3dDrawComponent(pOwnerEntity),
	mDrawDist(FLT_MAX),
	mRejectionScaleModifier(1.0f),
	mFadeDelay(0.0f),
	mFadeTime(2.0f),
	mState(STATE_WHOLE),
	mTimer(0.0f),
	mColor(255,255,255)
{
	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "Model Asset", mModelAssetName))
		->setWatcher(this, &Vu3dDrawBreakableModelComponent::modified);

	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));
	addProperty(new VuFloatProperty("Rejection Scale Modifier", mRejectionScaleModifier));
	addProperty(new VuFloatProperty("Fade Delay", mFadeDelay));
	addProperty(new VuFloatProperty("Fade Time", mFadeTime));
	addProperty(new VuVector3Property("Min Piece Lin Vel", mBreakableModelInstance.mMinPieceLinVel));
	addProperty(new VuVector3Property("Max Piece Lin Vel", mBreakableModelInstance.mMaxPieceLinVel));
	addProperty(new VuRotation3dProperty("Min Piece Ang Vel", mBreakableModelInstance.mMinPieceAngVel));
	addProperty(new VuRotation3dProperty("Max Piece Ang Vel", mBreakableModelInstance.mMaxPieceAngVel));
	addProperty(new VuFloatProperty("Min Velocity Damping", mBreakableModelInstance.mMinVelocityDamping));
	addProperty(new VuFloatProperty("Max Velocity Damping", mBreakableModelInstance.mMaxVelocityDamping));
	addProperty(new VuFloatProperty("Gravity", mBreakableModelInstance.mGravity));

	setDrawMethod(this, &Vu3dDrawBreakableModelComponent::draw);
}

//*****************************************************************************
Vu3dDrawBreakableModelComponent::~Vu3dDrawBreakableModelComponent()
{
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::onGameInitialize()
{
	mState = STATE_WHOLE;
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::onGameRelease()
{
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::startBreak(const VuMatrix &mat, const VuVector3 &vel, const VuColor &color)
{
	mBreakableModelInstance.initializePieces(mat, vel);
	mTimer = mFadeDelay + mFadeTime;
	mState = STATE_BREAKING;
	mColor = color;

	show();
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::tickDecision(float fdt)
{
	if ( mState == STATE_BREAKING )
	{
		mTimer -= fdt;
		if ( mTimer < 0 )
		{
			hide();
			mTimer = 0.0f;
			mState = STATE_DONE;
		}
	}
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::tickBuild(float fdt)
{
	if ( mState == STATE_BREAKING )
	{
		VuAabb aabb;
		mBreakableModelInstance.updatePieces(fdt, aabb);
		if ( aabb.isValid() )
			updateVisibility(aabb);
	}
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::draw(const VuGfxDrawParams &params)
{
	float distSquared = (getAabb().getCenter() - params.mEyePos).magSquared();
	if ( distSquared < mDrawDist*mDrawDist )
	{
		float fAlpha = VuMin(mTimer/mFadeTime, 1.0f);
		mColor.mA = (VUUINT8)VuRound(fAlpha*255.0f);

		mBreakableModelInstance.setColor(mColor);
		mBreakableModelInstance.setRejectionScaleModifier(mRejectionScaleModifier);
		mBreakableModelInstance.drawPieces(params);
	}
}

//*****************************************************************************
void Vu3dDrawBreakableModelComponent::modified()
{
	mBreakableModelInstance.setModelAsset(mModelAssetName);
}
