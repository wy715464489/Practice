//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Foliage entity
// 
//*****************************************************************************

#include "VuFoliageEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Gfx/Light/VuLightUtil.h"
#include "VuEngine/Managers/VuFoliageManager.h"


IMPLEMENT_RTTI(VuFoliageEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuFoliageEntity);


//*****************************************************************************
VuFoliageEntity::VuFoliageEntity():
	mFogEnabled(false),
	mManualColor(false),
	mReceiveShadows(true),
	mDirectionalLighting(false),
	mColor(255, 255, 255),
	mUV0(0.0f, 0.0f),
	mUV1(1.0f, 1.0f),
	mDrawDist(FLT_MAX),
	mBakedColor(0,0,0,1),
	mpBucket(VUNULL)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));

	mpTransformComponent->setWatcher(&VuFoliageEntity::transformModified);
	mpTransformComponent->setMask((VUUINT32)~VuTransformComponent::SCALE_Y);
	mp3dLayoutComponent->setDrawMethod(this, &VuFoliageEntity::drawLayout);
	mp3dDrawComponent->setDrawMethod(this, &VuFoliageEntity::draw);

	// properties
	addProperty(mpTextureAssetProperty = new VuAssetProperty<VuTextureAsset>("Texture Asset", mTextureAssetName)) -> setWatcher(this, &VuFoliageEntity::textureModified);

	addProperty(new VuBoolProperty("Fog Enabled", mFogEnabled));
	addProperty(new VuBoolProperty("Manual Color", mManualColor));
	addProperty(new VuBoolProperty("Receive Shadows", mReceiveShadows));
	addProperty(new VuBoolProperty("Directional Lighting", mDirectionalLighting));
	addProperty(new VuColorProperty("Color", mColor));
	addProperty(new VuFloatProperty("U0", mUV0.mX));
	addProperty(new VuFloatProperty("V0", mUV0.mY));
	addProperty(new VuFloatProperty("U1", mUV1.mX));
	addProperty(new VuFloatProperty("V1", mUV1.mY));
	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));
}

//*****************************************************************************
void VuFoliageEntity::onBake()
{
	// cast ray from top to bottom of foliage object to find lighting position
	VuVector3 pos = mpTransformComponent->getWorldPosition();
	VuVector3 center = pos + VuVector3(0, 0, mpTransformComponent->getWorldScale().mZ);
	VuVector3 top = center + VuVector3(0, 0, mpTransformComponent->getWorldScale().mZ);

	VuVector3 lightPos = center;
	collideRayRecursive(getRootEntity(), top, lightPos);

	// gather light info
	VuLightUtil::VuLightInfo lightInfo(VuAabb::zero(), VuMatrix::translation(lightPos));
	VuLightUtil::gatherLightsRecursive(getRootEntity(), lightInfo);
	VuLightUtil::gatherOccludersRecursive(getRootEntity(), VUNULL, lightInfo);

	// calculate color
	mBakedColor = VuLightUtil::calculateFoliageColor(lightPos, lightInfo, mReceiveShadows);
}

//*****************************************************************************
void VuFoliageEntity::onClearBaked()
{
	mBakedColor.set(0,0,0,1);
}

//*****************************************************************************
void VuFoliageEntity::onGameInitialize()
{
	mp3dDrawComponent->show();

	createBucket();
}

//*****************************************************************************
void VuFoliageEntity::onGameRelease()
{
	mp3dDrawComponent->hide();

	releaseBucket();
}

//*****************************************************************************
void VuFoliageEntity::loadInternal(const VuJsonContainer &data)
{
	transformModified();
	textureModified();

	VuDataUtil::getValue(data["BakedColor"], mBakedColor);
}

//*****************************************************************************
void VuFoliageEntity::saveInternal(VuJsonContainer &data) const
{
	if ( mBakedColor != VuVector4(0,0,0,1) )
		VuDataUtil::putValue(data["BakedColor"], mBakedColor);
}

//*****************************************************************************
void VuFoliageEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	// check draw dist
	VuVector3 vPos = mpTransformComponent->getWorldPosition();
	float distSquared = (vPos - params.mCamera.getEyePosition()).magSquared();
	if ( distSquared > mDrawDist*mDrawDist )
		return;

	if ( mpTextureAssetProperty->getAsset() )
	{
		VuFoliageManager::DrawParams foliageParams;
		foliageParams.mPos = vPos;
		foliageParams.mScaleX = mpTransformComponent->getWorldScale().mX;
		foliageParams.mScaleZ = mpTransformComponent->getWorldScale().mZ;
		calculateDrawColor(foliageParams.mColor);
		foliageParams.mUV0 = mUV0;
		foliageParams.mUV1 = mUV1;

		VuFoliageManager::IF()->drawLayout(mpTextureAssetProperty->getAsset(), mFogEnabled, foliageParams, params.mCamera);
	}
}

//*****************************************************************************
void VuFoliageEntity::draw(const VuGfxDrawParams &params)
{
	// check draw dist
	VuVector3 vPos = mpTransformComponent->getWorldPosition();
	float distSquared = (vPos - params.mEyePos).magSquared();
	if ( distSquared > mDrawDist*mDrawDist )
		return;

	if ( mpBucket )
	{
		VuFoliageManager::DrawParams foliageParams;
		foliageParams.mPos = vPos;
		foliageParams.mScaleX = mpTransformComponent->getWorldScale().mX;
		foliageParams.mScaleZ = mpTransformComponent->getWorldScale().mZ;
		calculateDrawColor(foliageParams.mColor);
		foliageParams.mUV0 = mUV0;
		foliageParams.mUV1 = mUV1;

		VuFoliageManager::IF()->draw(mpBucket, foliageParams, params.mCamera);
	}
}

//*****************************************************************************
void VuFoliageEntity::transformModified()
{
	const VuVector3 &pos = mpTransformComponent->getWorldPosition();
	const VuVector3 &scale = mpTransformComponent->getWorldScale();

	VuAabb aabb;
	aabb.mMin = VuVector3(pos.mX - scale.mX, pos.mY - scale.mX, pos.mZ);
	aabb.mMax = VuVector3(pos.mX + scale.mX, pos.mY + scale.mX, pos.mZ + 2*scale.mZ);

	mp3dDrawComponent->updateVisibility(aabb);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1, -scale.mX, 0), VuVector3(1, scale.mX, 2)));
}

//*****************************************************************************
void VuFoliageEntity::textureModified()
{
	if ( isGameInitialized() )
	{
		releaseBucket();
		createBucket();
	}
}

//*****************************************************************************
void VuFoliageEntity::createBucket()
{
	if ( mpTextureAssetProperty->getAsset() )
		mpBucket = VuFoliageManager::IF()->createBucket(mpTextureAssetProperty->getAsset(), mFogEnabled);
}

//*****************************************************************************
void VuFoliageEntity::releaseBucket()
{
	if ( mpBucket )
	{
		VuFoliageManager::IF()->releaseBucket(mpBucket);
		mpBucket = VUNULL;
	}
}

//*****************************************************************************
void VuFoliageEntity::calculateDrawColor(VuColor &drawColor)
{
	if ( mManualColor )
	{
		drawColor = mColor;
		return;
	}

	float shadowValue = mBakedColor.mW;

	// ambient lighting
	VuVector3 color;
	VuLightManager::IF()->ambientLight().mFoliageColor.toVector3(color);

	// directional lighting
	const VuDirectionalLight &dirLight = VuLightManager::IF()->directionalLight();

	float dirLightAmt = 1.0f;
	if ( mDirectionalLighting )
	{
		// front contribution
		float lightDot = VuDot(mpTransformComponent->getWorldTransform().getAxisY(), dirLight.mDirection);
		dirLightAmt = VuMax(-lightDot, 0.0f);
	}

	color += (shadowValue*dirLightAmt)*dirLight.mFoliageColor.toVector3();

	// scene lighting
	color.mX += mBakedColor.mX;
	color.mY += mBakedColor.mY;
	color.mZ += mBakedColor.mZ;

	color.mX = VuMin(color.mX, 1.0f);
	color.mY = VuMin(color.mY, 1.0f);
	color.mZ = VuMin(color.mZ, 1.0f);

	drawColor.fromVector3(color);
}

//*****************************************************************************
void VuFoliageEntity::collideRayRecursive(const VuEntity *pEntity, const VuVector3 &v0, VuVector3 &v1)
{
	// collide
	if ( Vu3dLayoutComponent *p3dLayoutComponent = pEntity->getComponent<Vu3dLayoutComponent>() )
		p3dLayoutComponent->collideRay(v0, v1);

	// recurse
	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
		collideRayRecursive(pEntity->getChildEntity(i), v0, v1);
}
