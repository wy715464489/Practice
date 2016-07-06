//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx Settings Manager
// 
//*****************************************************************************

#include "VuGfxSettingsManager.h"
#include "VuEngine/Entities/GfxSettings/VuGfxSettingsEntity.h"


// the interface
VuGfxSettingsManager VuGfxSettingsManager::mGfxSettingsManagerInterface;

// settings math
class VuGfxSettingsData
{
public:
	void		add(const VuGfxSettings &settings, float weight);
	void		normalize();
	void		get(VuGfxSettings &settings);

	float		mWeight;

	float		mCameraFar;
	VuVector4	mClearColor;
	float		mFogStart;
	float		mFogEnd;
	VuVector4	mFogColor;
	float		mDepthFogStart;
	float		mDepthFogDist;
	VuVector4	mDepthFogColor;
	VuVector4	mContrast;
	VuVector4	mTint;
	float		mGammaMin;
	float		mGammaMax;
	float		mGammaCurve;
	VuVector4	mWaterAmbientColor;
	VuVector4	mWaterDiffuseColor;
	VuVector4	mWaterFoamAmbientColor;
	VuVector4	mWaterFoamDiffuseColor;
	float		mWaterFoamTextureSize;
	VuVector4	mWaterDecalAmbientColor;
	VuVector4	mWaterDecalDiffuseColor;
	float		mDepthFoamValue;
	float		mDepthFoamIntensity;
	VuVector4	mPfxAmbientColor;
	VuVector4	mPfxDiffuseColor;
	float		mHBAORadius;
	float		mHBAOMaxRadius;
	float		mHBAOAngleBias;
	float		mHBAOStrength;
	float		mHBAOLightFactor;
	float		mHBAOAmbientFactor;
};


//*****************************************************************************
VuGfxSettingsManager::VuGfxSettingsManager()
{
}

//*****************************************************************************
VuGfxSettingsManager::~VuGfxSettingsManager()
{
}

//*****************************************************************************
void VuGfxSettingsManager::getSettings(const VuVector3 &vPos, VuGfxSettings &settings)
{
	// start w/ empty settings
	VuGfxSettingsData data;
	memset(&data, 0, sizeof(data));

	// add weights of placed settings
	for ( VuGfxSettingsEntity **ppGSE = &mPlacedSettings.begin(); ppGSE != &mPlacedSettings.end(); ppGSE++ )
	{
		float weight = (*ppGSE)->getPositionalWeight(vPos);
		if ( weight > 0 )
		{
			weight *= (*ppGSE)->getTemporalWeight();
			if ( weight > 0 )
			{
				data.add((*ppGSE)->getSettings(), weight);
			}
		}
	}

	// add global settings if weight is < 1
	if ( data.mWeight < 1.0f )
		data.add(mGlobalSettings, 1.0f - data.mWeight);
	else
		data.normalize();

	data.get(settings);
}

//*****************************************************************************
void VuGfxSettingsData::add(const VuGfxSettings &settings, float weight)
{
	mWeight += weight;

	mCameraFar					+= weight*settings.mCameraFar;
	mClearColor					+= weight*settings.mClearColor.toVector4();
	mFogStart					+= weight*settings.mFogStart;
	mFogEnd						+= weight*settings.mFogEnd;
	mFogColor					+= weight*settings.mFogColor.toVector4();
	mDepthFogStart				+= weight*settings.mDepthFogStart;
	mDepthFogDist				+= weight*settings.mDepthFogDist;
	mDepthFogColor				+= weight*settings.mDepthFogColor.toVector4();
	mContrast					+= weight*settings.mContrast.toVector4();
	mTint						+= weight*settings.mTint.toVector4();
	mGammaMin					+= weight*settings.mGammaMin;
	mGammaMax					+= weight*settings.mGammaMax;
	mGammaCurve					+= weight*settings.mGammaCurve;
	mWaterAmbientColor			+= weight*settings.mWaterAmbientColor.toVector4();
	mWaterDiffuseColor			+= weight*settings.mWaterDiffuseColor.toVector4();
	mWaterFoamAmbientColor		+= weight*settings.mWaterFoamAmbientColor.toVector4();
	mWaterFoamDiffuseColor		+= weight*settings.mWaterFoamDiffuseColor.toVector4();
	mWaterFoamTextureSize		+= weight*settings.mWaterFoamTextureSize;
	mWaterDecalAmbientColor		+= weight*settings.mWaterDecalAmbientColor.toVector4();
	mWaterDecalDiffuseColor		+= weight*settings.mWaterDecalDiffuseColor.toVector4();
	mDepthFoamValue				+= weight*settings.mDepthFoamValue;
	mDepthFoamIntensity			+= weight*settings.mDepthFoamIntensity;
	mPfxAmbientColor			+= weight*settings.mPfxAmbientColor.toVector4();
	mPfxDiffuseColor			+= weight*settings.mPfxDiffuseColor.toVector4();
	mHBAORadius					+= weight*settings.mHBAORadius;
	mHBAOMaxRadius				+= weight*settings.mHBAOMaxRadius;
	mHBAOAngleBias				+= weight*settings.mHBAOAngleBias;
	mHBAOStrength				+= weight*settings.mHBAOStrength;
	mHBAOLightFactor			+= weight*settings.mHBAOLightFactor;
	mHBAOAmbientFactor			+= weight*settings.mHBAOAmbientFactor;
}

//*****************************************************************************
void VuGfxSettingsData::normalize()
{
	mCameraFar /= mWeight;
	mClearColor /= mWeight;
	mFogStart /= mWeight;
	mFogEnd /= mWeight;
	mFogColor /= mWeight;
	mDepthFogStart /= mWeight;
	mDepthFogDist /= mWeight;
	mDepthFogColor /= mWeight;
	mContrast /= mWeight;
	mTint /= mWeight;
	mGammaMin /= mWeight;
	mGammaMax /= mWeight;
	mGammaCurve /= mWeight;
	mWaterAmbientColor /= mWeight;
	mWaterDiffuseColor /= mWeight;
	mWaterFoamAmbientColor /= mWeight;
	mWaterFoamDiffuseColor /= mWeight;
	mWaterFoamTextureSize /= mWeight;
	mWaterDecalAmbientColor /= mWeight;
	mWaterDecalDiffuseColor /= mWeight;
	mDepthFoamValue /= mWeight;
	mDepthFoamIntensity /= mWeight;
	mPfxAmbientColor /= mWeight;
	mPfxDiffuseColor /= mWeight;
	mHBAORadius /= mWeight;
	mHBAOMaxRadius /= mWeight;
	mHBAOAngleBias /= mWeight;
	mHBAOStrength /= mWeight;
	mHBAOLightFactor /= mWeight;
	mHBAOAmbientFactor /= mWeight;

	mWeight = 1;
}

//*****************************************************************************
void VuGfxSettingsData::get(VuGfxSettings &settings)
{
	settings.mCameraFar					= mCameraFar;
	settings.mClearColor				= VuColor(mClearColor);
	settings.mFogStart					= mFogStart;
	settings.mFogEnd					= mFogEnd;
	settings.mFogColor					= VuColor(mFogColor);
	settings.mDepthFogStart				= mDepthFogStart;
	settings.mDepthFogDist				= mDepthFogDist;
	settings.mDepthFogColor				= VuColor(mDepthFogColor);
	settings.mContrast					= VuColor(mContrast);
	settings.mTint						= VuColor(mTint);
	settings.mGammaMin					= mGammaMin;
	settings.mGammaMax					= mGammaMax;
	settings.mGammaCurve				= mGammaCurve;
	settings.mWaterAmbientColor			= VuColor(mWaterAmbientColor);
	settings.mWaterDiffuseColor			= VuColor(mWaterDiffuseColor);
	settings.mWaterFoamAmbientColor		= VuColor(mWaterFoamAmbientColor);
	settings.mWaterFoamDiffuseColor		= VuColor(mWaterFoamDiffuseColor);
	settings.mWaterFoamTextureSize		= mWaterFoamTextureSize;
	settings.mWaterDecalAmbientColor	= VuColor(mWaterDecalAmbientColor);
	settings.mWaterDecalDiffuseColor	= VuColor(mWaterDecalDiffuseColor);
	settings.mDepthFoamValue			= mDepthFoamValue;
	settings.mDepthFoamIntensity		= mDepthFoamIntensity;
	settings.mPfxAmbientColor			= VuColor(mPfxAmbientColor);
	settings.mPfxDiffuseColor			= VuColor(mPfxDiffuseColor);
	settings.mHBAORadius				= mHBAORadius;
	settings.mHBAOMaxRadius				= mHBAOMaxRadius;
	settings.mHBAOAngleBias				= mHBAOAngleBias;
	settings.mHBAOStrength				= mHBAOStrength;
	settings.mHBAOLightFactor			= mHBAOLightFactor;
	settings.mHBAOAmbientFactor			= mHBAOAmbientFactor;
}
