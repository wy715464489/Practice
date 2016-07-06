//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSettings entity
// 
//*****************************************************************************

#include "VuGfxSettingsEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"


IMPLEMENT_RTTI(VuGfxSettingsEntity, VuEntity);


//*****************************************************************************
VuGfxSettingsEntity::VuGfxSettingsEntity():
	mbAlwaysActive(true),
	mRampUpTime(1.0f),
	mDuration(1.0f),
	mRampDownTime(1.0f),
	mTimer(0.0f)
{
	// default 'nice' settings
	mSettings.mFogStart = 500;
	mSettings.mFogEnd = 1000;
	mSettings.mDepthFogStart = 2;
	mSettings.mDepthFogDist = 10;

	// properties
	addProperty(new VuBoolProperty("Always Active", mbAlwaysActive));
	addProperty(new VuFloatProperty("Ramp Up Time", mRampUpTime));
	addProperty(new VuFloatProperty("Duration", mDuration));
	addProperty(new VuFloatProperty("Ramp Down Time", mRampDownTime));

	addProperty(new VuFloatProperty("Camera Far Plane", mSettings.mCameraFar));
	addProperty(new VuColorProperty("Clear Color", mSettings.mClearColor));

	addProperty(new VuFloatProperty("Fog Start", mSettings.mFogStart));
	addProperty(new VuFloatProperty("Fog End", mSettings.mFogEnd));
	addProperty(new VuColorProperty("Fog Color", mSettings.mFogColor));
	addProperty(new VuFloatProperty("Depth Fog Start", mSettings.mDepthFogStart));
	addProperty(new VuFloatProperty("Depth Fog Distance", mSettings.mDepthFogDist));
	addProperty(new VuColorProperty("Depth Fog Color", mSettings.mDepthFogColor));

	addProperty(new VuColorProperty("Contrast", mSettings.mContrast));
	addProperty(new VuColorProperty("Tint", mSettings.mTint));

	addProperty(new VuFloatProperty("Gamma Min", mSettings.mGammaMin));
	addProperty(new VuFloatProperty("Gamma Max", mSettings.mGammaMax));
	addProperty(new VuFloatProperty("Gamma Curve", mSettings.mGammaCurve));

	addProperty(new VuColorProperty("Water Ambient Color", mSettings.mWaterAmbientColor));
	addProperty(new VuColorProperty("Water Diffuse Color", mSettings.mWaterDiffuseColor));
	addProperty(new VuColorProperty("Water Foam Ambient Color", mSettings.mWaterFoamAmbientColor));
	addProperty(new VuColorProperty("Water Foam Diffuse Color", mSettings.mWaterFoamDiffuseColor));
	addProperty(new VuFloatProperty("Water Foam Texture Size", mSettings.mWaterFoamTextureSize));
	addProperty(new VuColorProperty("Water Decal Ambient Color", mSettings.mWaterDecalAmbientColor));
	addProperty(new VuColorProperty("Water Decal Diffuse Color", mSettings.mWaterDecalDiffuseColor));
	addProperty(new VuFloatProperty("Water Depth Foam Value", mSettings.mDepthFoamValue));
	addProperty(new VuFloatProperty("Water Depth Foam Intensity", mSettings.mDepthFoamIntensity));

	addProperty(new VuColorProperty("Pfx Ambient Color", mSettings.mPfxAmbientColor));
	addProperty(new VuColorProperty("Pfx Diffuse Color", mSettings.mPfxDiffuseColor));

	addProperty(new VuFloatProperty("HBAO Radius", mSettings.mHBAORadius));
	addProperty(new VuFloatProperty("HBAO Max Radius", mSettings.mHBAOMaxRadius));
	addProperty(new VuFloatProperty("HBAO Angle Bias", mSettings.mHBAOAngleBias));
	addProperty(new VuFloatProperty("HBAO Strength", mSettings.mHBAOStrength));
	addProperty(new VuFloatProperty("HBAO Light Factor", mSettings.mHBAOLightFactor));
	addProperty(new VuFloatProperty("HBAO Ambient Factor", mSettings.mHBAOAmbientFactor));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuGfxSettingsEntity, Trigger);
}

//*****************************************************************************
void VuGfxSettingsEntity::onGameInitialize()
{
	if ( !mbAlwaysActive )
		VuTickManager::IF()->registerHandler(this, &VuGfxSettingsEntity::tickBuild, "Build");
}

//*****************************************************************************
void VuGfxSettingsEntity::onGameRelease()
{
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
float VuGfxSettingsEntity::getTemporalWeight()
{
	if ( mbAlwaysActive )
		return 1.0f;

	if ( mTimer <= 0 )
		return 0.0f;

	if ( mTimer <= mRampDownTime )
		return mTimer/mRampDownTime;

	if ( mTimer <= mRampDownTime + mDuration )
		return 1.0f;

	return 1.0f - (mTimer - mDuration - mRampDownTime)/mRampUpTime;
}

//*****************************************************************************
void VuGfxSettingsEntity::tickBuild(float fdt)
{
	mTimer -= fdt;
}

//*****************************************************************************
VuRetVal VuGfxSettingsEntity::Trigger(const VuParams &params)
{
	if ( mTimer <= 0 )
	{
		mTimer = mRampUpTime + mDuration + mRampDownTime;
	}

	return VuRetVal();
}