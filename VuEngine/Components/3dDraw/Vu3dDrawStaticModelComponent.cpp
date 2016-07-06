//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawStaticModelComponent class
// 
//*****************************************************************************

#include "Vu3dDrawStaticModelComponent.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"


IMPLEMENT_RTTI(Vu3dDrawStaticModelComponent, Vu3dDrawComponent);


//*****************************************************************************
Vu3dDrawStaticModelComponent::Vu3dDrawStaticModelComponent(VuEntity *pOwnerEntity):
	Vu3dDrawComponent(pOwnerEntity),
	mColor(255,255,255),
	mAmbientColor(160,160,160),
	mDrawDist(FLT_MAX),
	mLod0DrawDist(FLT_MAX),
	mLod1DrawDist(FLT_MAX),
	mRejectionScaleModifier(1.0f),
	mUseLod1LowSpec(true),
	mCastBakedShadow(true),
	mMatrix(VuMatrix::identity()),
	mShadowValue(1.0f)
{
	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "Model Asset", mModelAssetName))
		->setWatcher(this, &Vu3dDrawStaticModelComponent::modified);

	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "LOD 1 Model Asset", mLod1ModelAssetName))
		->setWatcher(this, &Vu3dDrawStaticModelComponent::modified);

	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "LOD 2 Model Asset", mLod2ModelAssetName))
		->setWatcher(this, &Vu3dDrawStaticModelComponent::modified);

	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "Reflection Model Asset", mReflectionModelAssetName))
		->setWatcher(this, &Vu3dDrawStaticModelComponent::modified);

	addProperty(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "Ultra Model Asset", mUltraModelAssetName))
		->setWatcher(this, &Vu3dDrawStaticModelComponent::modified);

	addProperty(new VuColorProperty("Color", mColor));
	addProperty(new VuColorProperty("Ambient Color", mAmbientColor));

	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));
	addProperty(new VuFloatProperty("LOD 0 Draw Distance", mLod0DrawDist));
	addProperty(new VuFloatProperty("LOD 1 Draw Distance", mLod1DrawDist));
	addProperty(new VuFloatProperty("Rejection Scale Modifier", mRejectionScaleModifier));
	addProperty(new VuBoolProperty("Use LOD 1 Low Spec", mUseLod1LowSpec));
	addProperty(new VuBoolProperty("Cast Baked Shadow", mCastBakedShadow));

	setDrawMethod(this, &Vu3dDrawStaticModelComponent::draw);
	setDrawShadowMethod(this, &Vu3dDrawStaticModelComponent::drawShadow);
	setDrawPrefetchMethod(this, &Vu3dDrawStaticModelComponent::drawPrefetch);
}

//*****************************************************************************
Vu3dDrawStaticModelComponent::~Vu3dDrawStaticModelComponent()
{
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::updateVisibility(const VuMatrix &mat)
{
	mMatrix = mat;
	Vu3dDrawComponent::updateVisibility(getModelAabb(), mMatrix);
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::drawLayout(const Vu3dLayoutDrawParams &params)
{
	VuStaticModelInstance *pModelInstance = &mModelInstance;
	if ( !params.mbForceHighLOD )
		pModelInstance = chooseModelToDraw(params.mCamera.getEyePosition(), false);

	if ( pModelInstance )
	{
		pModelInstance->setRejectionScaleModifier(mRejectionScaleModifier);
		pModelInstance->setColor(mColor);
		pModelInstance->draw(mMatrix, VuGfxDrawParams(params.mCamera));
	}
}

//*****************************************************************************
bool Vu3dDrawStaticModelComponent::collideLayout(const VuVector3 &v0, VuVector3 &v1)
{
	return mModelInstance.collideRay(mMatrix, v0, v1);
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::draw(const VuGfxDrawParams &params)
{
	if ( VuStaticModelInstance *pModelInstance = chooseModelToDraw(params.mEyePos, params.mbDrawReflection) )
	{
		VuColor color = getColor();

		pModelInstance->setRejectionScaleModifier(mRejectionScaleModifier);
		pModelInstance->setColor(color);
		pModelInstance->draw(mMatrix, params);
	}
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::drawShadow(const VuGfxDrawShadowParams &params)
{
	if ( VuStaticModelInstance *pModelInstance = chooseModelToDraw(params.mEyePos, params.mbDrawReflection) )
	{
		pModelInstance->setRejectionScaleModifier(mRejectionScaleModifier);
		pModelInstance->drawShadow(mMatrix, params);
	}
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::drawPrefetch()
{
	mModelInstance.drawPrefetch();
	mLod1ModelInstance.drawPrefetch();
	mLod2ModelInstance.drawPrefetch();
	mReflectionModelInstance.drawPrefetch();
	mUltraModelInstance.drawPrefetch();
}

//*****************************************************************************
VuStaticModelInstance *Vu3dDrawStaticModelComponent::chooseModelToDraw(const VuVector3 &eyePos, bool bDrawReflection)
{
	float distSquared = (getAabb().getCenter() - eyePos).magSquared();
	if ( distSquared >= mDrawDist*mDrawDist )
		return VUNULL;

	if ( bDrawReflection )
		return &mReflectionModelInstance;

	if ( mUltraModelInstance.getGfxStaticScene() )
	{
		if ( (distSquared > mLod1DrawDist*mLod1DrawDist) && mLod1ModelInstance.getGfxStaticScene() )
			return &mLod1ModelInstance;

		if ( (distSquared > mLod0DrawDist*mLod0DrawDist) && mModelInstance.getGfxStaticScene() )
			return &mModelInstance;

		return &mUltraModelInstance;
	}

	if ( (distSquared > mLod1DrawDist*mLod1DrawDist) && mLod2ModelInstance.getGfxStaticScene() )
		return &mLod2ModelInstance;

	if ( ((distSquared > mLod0DrawDist*mLod0DrawDist) && mLod1ModelInstance.getGfxStaticScene()) || (mModelInstance.getGfxStaticScene() == VUNULL) )
		return &mLod1ModelInstance;

	return &mModelInstance;
}

//*****************************************************************************
void Vu3dDrawStaticModelComponent::modified()
{
	if ( VuGfxUtil::IF()->getLowModelLOD() && mUseLod1LowSpec && mLod1ModelAssetName.length() )
		mModelInstance.reset();
	else
		mModelInstance.setModelAsset(mModelAssetName);

	mLod1ModelInstance.setModelAsset(mLod1ModelAssetName);
	mLod2ModelInstance.setModelAsset(mLod2ModelAssetName);
	mReflectionModelInstance.setModelAsset(mReflectionModelAssetName);

	if ( VuGfxUtil::IF()->getUltraModelLOD() || VuEngine::IF()->editorMode() )
		mUltraModelInstance.setModelAsset(mUltraModelAssetName);

	mModelAabb = mModelInstance.getGfxStaticScene() ? mModelInstance.getAabb() : mLod1ModelInstance.getAabb();

	updateVisibility(mMatrix);

	if ( Vu3dLayoutComponent *p3dLayoutComponent = getOwnerEntity()->getComponent<Vu3dLayoutComponent>() )
		p3dLayoutComponent->setLocalBounds(mModelAabb);
}
