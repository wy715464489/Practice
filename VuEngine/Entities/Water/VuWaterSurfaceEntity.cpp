//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water surface entity
// 
//*****************************************************************************

#include "VuWaterSurfaceEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCubeTextureAsset.h"
#include "VuEngine/Assets/VuWaterMapAsset.h"
#include "VuEngine/Assets/VuLightMapAsset.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Dynamics/VuCollisionTypes.h"


IMPLEMENT_RTTI(VuWaterSurfaceEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuWaterSurfaceEntity);


// choices for size
static VuStaticIntEnumProperty::Choice sSizeChoices[] =
{
	{ "1", 1},
	{ "2", 2},
	{ "4", 4},
	{ "8", 8},
	{ "16", 16},
	{ "32", 32},
	{ "64", 64},
	{ "128", 128},
	{ "256", 256},
	{ "512", 512},
	{ "1024", 1024},
	{ "2048", 2048},
	{ "4096", 4096},
	{ "8192", 8192},
	{ VUNULL }
};

//*****************************************************************************
VuWaterSurfaceEntity::VuWaterSurfaceEntity():
	mSizeX(256),
	mSizeY(256),
	mMaxWaveDepth(5),
	mMaxWaveHeight(5),
	mReflectionHeight(5),
	mReflectionOffset(0),
	mMinRecursionDepth(0),
	mDrawDist(FLT_MAX),
	mpSurface(VUNULL),
	mpShader(VUNULL)
{
	// properties
	addProperty(new VuStaticIntEnumProperty("X Size", mSizeX, sSizeChoices))	->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuStaticIntEnumProperty("Y Size", mSizeY, sSizeChoices))	->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuFloatProperty("Max Wave Depth", mMaxWaveDepth))		->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuFloatProperty("Max Wave Height", mMaxWaveHeight))		->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuFloatProperty("Reflection Height", mReflectionHeight))->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuFloatProperty("Reflection Offset", mReflectionOffset))->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuIntProperty("Min Recursion Depth", mMinRecursionDepth))->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));
	addProperty(mpWaterMapAssetProperty = new VuAssetProperty<VuWaterMapAsset>("WaterMap", mWaterMapAssetName));
	addProperty(mpLightMapAssetProperty = new VuAssetProperty<VuLightMapAsset>("LightMap", mLightMapAssetName));

	addProperty(new VuBoolProperty("OverrideGlobalSettings", mShaderDesc.mOverrideGlobalSettings))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("AmbientColor", mShaderDesc.mAmbientColor))				->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("DiffuseColor", mShaderDesc.mDiffuseColor))				->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("FoamAmbientColor", mShaderDesc.mFoamAmbientColor))		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("FoamDiffuseColor", mShaderDesc.mFoamDiffuseColor))		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuBoolProperty("FogEnabled", mShaderDesc.mFogEnabled))					->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuFloatProperty("FoamTextureSize", mShaderDesc.mFoamTextureSize))		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuAssetNameProperty(VuTextureAsset::msRTTI.mstrType, "FoamTextureAsset", mShaderDesc.mFoamTextureAsset))
		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuBoolProperty("ProceduralReflection", mShaderDesc.mProceduralReflection))	->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuBoolProperty("NormalMapEnabled", mShaderDesc.mNormalMapEnabled))			->	setWatcher(this, &VuWaterSurfaceEntity::surfaceModified);
	addProperty(new VuAssetNameProperty(VuCubeTextureAsset::msRTTI.mstrType, "ReflectionCubeTextureAsset", mShaderDesc.mReflectionCubeTextureAsset))
		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("DecalAmbientColor", mShaderDesc.mDecalAmbientColor))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuColorProperty("DecalDiffuseColor", mShaderDesc.mDecalDiffuseColor))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuAssetNameProperty(VuTextureAsset::msRTTI.mstrType, "DecalTextureAsset", mShaderDesc.mDecalTextureAsset))
		->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuBoolProperty("ReceiveShadows", mShaderDesc.mReceiveShadows))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuFloatProperty("FresnelFactor", mShaderDesc.mFresnelFactor))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuFloatProperty("FresnelMin", mShaderDesc.mFresnelMin))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);
	addProperty(new VuFloatProperty("FresnelMax", mShaderDesc.mFresnelMax))	->	setWatcher(this, &VuWaterSurfaceEntity::shaderModified);

	// components
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	mp3dDrawComponent->setDrawMethod(this, &VuWaterSurfaceEntity::draw);
	mp3dLayoutComponent->setDrawMethod(this, &VuWaterSurfaceEntity::drawLayout);
	mp3dLayoutComponent->setCollideMethod(this, &VuWaterSurfaceEntity::collideLayout);

	// want to know when Transform is changed
	mpTransformComponent->setWatcher(&VuWaterSurfaceEntity::surfaceModified);

	// limit manipulation to translation and rotation about Z
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z);

	surfaceModified();

	// register config
	if ( VuWater::IF() )
	{
		VuConfigManager::IF()->registerBoolHandler("Water/Reflection", this, &VuWaterSurfaceEntity::configReflection);
		VuConfigManager::IF()->registerBoolHandler("Water/NormalMap", this, &VuWaterSurfaceEntity::configNormalMap);
		VuConfigManager::IF()->registerIntHandler("Gfx/ShaderLOD", this, &VuWaterSurfaceEntity::configShaderLOD);
	}
}

//*****************************************************************************
VuWaterSurfaceEntity::~VuWaterSurfaceEntity()
{
	// unregister config
	if ( VuWater::IF() )
	{
		VuConfigManager::IF()->unregisterBoolHandler("Water/Reflection", this);
		VuConfigManager::IF()->unregisterBoolHandler("Water/NormalMap", this);
		VuConfigManager::IF()->unregisterIntHandler("Gfx/ShaderLOD", this);
	}

	// destroy shader
	if ( mpShader )
	{
		mpShader->removeRef();
		mpShader = VUNULL;
	}
}

//*****************************************************************************
void VuWaterSurfaceEntity::onPostLoad()
{
	surfaceModified();
	shaderModified();
}

//*****************************************************************************
void VuWaterSurfaceEntity::onGameInitialize()
{
	// create surface
	VuWaterSurfaceDesc surfaceDesc;
	createSurfaceDesc(surfaceDesc);
	mpSurface = VuWater::IF()->createSurface(surfaceDesc, this);

	// start rendering
	mp3dDrawComponent->show();
}

//*****************************************************************************
void VuWaterSurfaceEntity::onGameRelease()
{
	// stop rendering
	mp3dDrawComponent->hide();

	// destroy surface
	mpSurface->removeRef();
	mpSurface = VUNULL;
}

//*****************************************************************************
VuWaterMapAsset *VuWaterSurfaceEntity::getWaterMapAsset() const
{
	return mpWaterMapAssetProperty->getAsset();
}

//*****************************************************************************
VuLightMapAsset *VuWaterSurfaceEntity::getLightMapAsset() const
{
	return mpLightMapAssetProperty->getAsset();
}

//*****************************************************************************
void VuWaterSurfaceEntity::surfaceModified()
{
	// modify surface
	VuWaterSurfaceDesc desc;
	createSurfaceDesc(desc);
	if ( mpSurface )
		mpSurface->modify(desc);

	// update bounds
	VuAabb aabb;
	aabb.mMin = VuVector3(-0.5f*mSizeX, -0.5f*mSizeY, -mMaxWaveDepth);
	aabb.mMax = VuVector3( 0.5f*mSizeX,  0.5f*mSizeY,  mMaxWaveHeight);
	mp3dDrawComponent->updateVisibility(aabb, mpTransformComponent->getWorldTransform());

	aabb.mMax.mZ = VuMax(aabb.mMax.mZ, mReflectionHeight);
	mp3dLayoutComponent->setLocalBounds(aabb);
}

//*****************************************************************************
void VuWaterSurfaceEntity::shaderModified()
{
	VuGfxSort::IF()->flush();

	if ( mpShader )
	{
		mpShader->removeRef();
		mpShader = VUNULL;
	}

	if ( VuWater::IF() )
	{
		VuWaterShaderDesc shaderDesc;
		createShaderDesc(shaderDesc);
		mpShader = VuWater::IF()->createShader(shaderDesc);
	}

	getProperties().get("AmbientColor")->enable(mShaderDesc.mOverrideGlobalSettings);
	getProperties().get("DiffuseColor")->enable(mShaderDesc.mOverrideGlobalSettings);
	getProperties().get("FoamAmbientColor")->enable(mShaderDesc.mOverrideGlobalSettings);
	getProperties().get("FoamDiffuseColor")->enable(mShaderDesc.mOverrideGlobalSettings);
	getProperties().get("FoamTextureSize")->enable(mShaderDesc.mOverrideGlobalSettings);
}

//*****************************************************************************
void VuWaterSurfaceEntity::createSurfaceDesc(VuWaterSurfaceDesc &desc)
{
	memset(&desc, 0, sizeof(desc));
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mPowSizeX = VuRound(VuLog((float)mSizeX)/VuLog(2.0f));
	desc.mPowSizeY = VuRound(VuLog((float)mSizeY)/VuLog(2.0f));
	desc.mMaxWaveDepth = mMaxWaveDepth;
	desc.mMaxWaveHeight = mMaxWaveHeight;
	desc.mReflectionHeight = mReflectionHeight;
	desc.mReflectionOffset = mReflectionOffset;
	desc.mMinRecursionDepth = mMinRecursionDepth;
	desc.mpWaterMapAsset = mpWaterMapAssetProperty->getAsset();
	desc.mpLightMapAsset = mpLightMapAssetProperty->getAsset();
	desc.mProceduralReflection = mShaderDesc.mProceduralReflection;
	desc.mFlags = WATER_SURFACE_WATER;
}

//*****************************************************************************
void VuWaterSurfaceEntity::createShaderDesc(VuWaterShaderDesc &desc)
{
	desc = mShaderDesc;
	if ( !VuWater::IF()->getProdecuralReflectionsEnabled() )
		desc.mProceduralReflection = false;
	if ( !VuWater::IF()->getNormalMapEnabled() )
		desc.mNormalMapEnabled = false;
	if ( VuGfxUtil::IF()->getShaderLOD() != 0 )
		desc.mReceiveShadows = false;
}

//*****************************************************************************
void VuWaterSurfaceEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	float x0 = -0.5f*mSizeX;
	float y0 = -0.5f*mSizeY;
	float x1 = 0.5f*mSizeX;
	float y1 = 0.5f*mSizeY;

	// draw surface
	{
		VuVector3 verts[4];
		verts[0] = VuVector3(x0, y0, 0.0f);
		verts[1] = VuVector3(x1, y0, 0.0f);
		verts[2] = VuVector3(x0, y1, 0.0f);
		verts[3] = VuVector3(x1, y1, 0.0f);

		VuGfxUtil::IF()->drawTriangleStrip(mShaderDesc.mDiffuseColor, verts, 4, mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix());
	}

	// draw line rect at min(mMaxWaveHeight,mReflectionHeight)
	if ( mMaxWaveHeight != mReflectionHeight )
	{
		VuVector3 verts[5];
		verts[0] = VuVector3(x0, y0, 0.0f);
		verts[1] = VuVector3(x1, y0, 0.0f);
		verts[2] = VuVector3(x1, y1, 0.0f);
		verts[3] = VuVector3(x0, y1, 0.0f);
		verts[4] = VuVector3(x0, y0, 0.0f);

		VuColor color = params.mbSelected ? VuColor(255, 255, 0) : VuColor(128, 128, 128);
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.translateLocal(VuVector3(0, 0, VuMin(mMaxWaveHeight, mReflectionHeight)));
		VuGfxUtil::IF()->drawLines3d(VUGFX_PT_LINESTRIP, color, verts, 5, mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
bool VuWaterSurfaceEntity::collideLayout(const VuVector3 &v0, VuVector3 &v1)
{
	float x0 = -0.5f*mSizeX;
	float y0 = -0.5f*mSizeY;
	float x1 = 0.5f*mSizeX;
	float y1 = 0.5f*mSizeY;

	VuVector3 verts[4];
	verts[0] = VuVector3(x0, y0, 0.0f);
	verts[1] = VuVector3(x1, y0, 0.0f);
	verts[2] = VuVector3(x1, y1, 0.0f);
	verts[3] = VuVector3(x0, y1, 0.0f);
	for ( int i = 0; i < 4; i++ )
		verts[i] = mpTransformComponent->getWorldTransform().transform(verts[i]);

	bool hit = false;
	hit |= VuMathUtil::triangleLineSegIntersection(verts[0], verts[1], verts[2], v0, v1, v1);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[2], verts[3], verts[0], v0, v1, v1);

	return hit;
}

//*****************************************************************************
void VuWaterSurfaceEntity::draw(const VuGfxDrawParams &params)
{
	if ( params.mbDrawReflection )
		return;

	float distSquared = (mp3dDrawComponent->getAabb().getCenter() - params.mEyePos).magSquared();
	if ( distSquared < mDrawDist*mDrawDist )
	{
		VuWaterRendererParams waterParams(mpSurface, mpShader, &params.mCamera);
		VuWater::IF()->renderer()->submit(waterParams);
	}
}

//*****************************************************************************
void VuWaterSurfaceEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}
