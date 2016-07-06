//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawAnimatedModelComponent class
// 
//*****************************************************************************

#include "Vu3dDrawAnimatedModelComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAnimatedModelAsset.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Animation/VuAnimatedSkeleton.h"


IMPLEMENT_RTTI(Vu3dDrawAnimatedModelComponent, Vu3dDrawComponent);


//*****************************************************************************
Vu3dDrawAnimatedModelComponent::Vu3dDrawAnimatedModelComponent(VuEntity *pOwnerEntity):
	Vu3dDrawComponent(pOwnerEntity),
	mColor(255,255,255),
	mAlpha(1.0f),
	mAdditiveAlpha(1.0f),
	mDrawDist(FLT_MAX),
	mpAnimatedSkeleton(VUNULL),
	mMatrix(VuMatrix::identity())
{
	addProperty(new VuAssetNameProperty(VuAnimatedModelAsset::msRTTI.mstrType, "Model Asset", mModelAssetName))
		->setWatcher(this, &Vu3dDrawAnimatedModelComponent::modified);

	addProperty(new VuColorProperty("Color", mColor));
	addProperty(new VuFloatProperty("Alpha", mAlpha));
	addProperty(new VuFloatProperty("Additive Alpha", mAdditiveAlpha));

	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));

	setDrawMethod(this, &Vu3dDrawAnimatedModelComponent::draw);
	setDrawShadowMethod(this, &Vu3dDrawAnimatedModelComponent::drawShadow);
	setDrawPrefetchMethod(this, &Vu3dDrawAnimatedModelComponent::drawPrefetch);
}

//*****************************************************************************
Vu3dDrawAnimatedModelComponent::~Vu3dDrawAnimatedModelComponent()
{
	if ( mpAnimatedSkeleton )
		mpAnimatedSkeleton->removeRef();
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::onGameInitialize()
{
	if ( mModelInstance.getSkeleton() )
		mpAnimatedSkeleton = new VuAnimatedSkeleton(mModelInstance.getSkeleton());
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::onGameRelease()
{
	if ( mpAnimatedSkeleton )
	{
		mpAnimatedSkeleton->removeRef();
		mpAnimatedSkeleton = VUNULL;
	}
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::updateVisibility(const VuMatrix &mat)
{
	mMatrix = mat;
	Vu3dDrawComponent::updateVisibility(mModelInstance.getLocalAabb(), mModelInstance.getRootTransform()*mMatrix);
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::drawLayout(const Vu3dLayoutDrawParams &params)
{
	VuColor color = mColor;
	color.mR = (VUUINT8)VuRound(color.mR*mAdditiveAlpha);
	color.mG = (VUUINT8)VuRound(color.mG*mAdditiveAlpha);
	color.mB = (VUUINT8)VuRound(color.mB*mAdditiveAlpha);
	color.mA = (VUUINT8)VuRound(color.mA*mAlpha);
	mModelInstance.setColor(color);
	mModelInstance.draw(mMatrix, VuGfxDrawParams(params.mCamera));
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::draw(const VuGfxDrawParams &params)
{
	float distSquared = (getAabb().getCenter() - params.mEyePos).magSquared();
	if ( distSquared < mDrawDist*mDrawDist )
	{
		VuColor color = mColor;
		color.mR = (VUUINT8)VuRound(color.mR*mAdditiveAlpha);
		color.mG = (VUUINT8)VuRound(color.mG*mAdditiveAlpha);
		color.mB = (VUUINT8)VuRound(color.mB*mAdditiveAlpha);
		color.mA = (VUUINT8)VuRound(color.mA*mAlpha);
		mModelInstance.setColor(color);
		mModelInstance.draw(mMatrix, params);
	}
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::drawShadow(const VuGfxDrawShadowParams &params)
{
	float distSquared = (getAabb().getCenter() - params.mEyePos).magSquared();
	if ( distSquared < mDrawDist*mDrawDist )
	{
		mModelInstance.drawShadow(mMatrix, params);
	}
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::drawPrefetch()
{
	mModelInstance.drawPrefetch();
}

//*****************************************************************************
void Vu3dDrawAnimatedModelComponent::modified()
{
	if ( mpAnimatedSkeleton )
	{
		mpAnimatedSkeleton->removeRef();
		mpAnimatedSkeleton = VUNULL;
	}

	mModelInstance.setModelAsset(mModelAssetName);

	updateVisibility(mMatrix);

	if ( Vu3dLayoutComponent *p3dLayoutComponent = getOwnerEntity()->getComponent<Vu3dLayoutComponent>() )
		p3dLayoutComponent->setLocalBounds(mModelInstance.getLocalAabb());

	if ( getOwnerEntity()->isGameInitialized() && mModelInstance.getSkeleton() )
		mpAnimatedSkeleton = new VuAnimatedSkeleton(mModelInstance.getSkeleton());
}
