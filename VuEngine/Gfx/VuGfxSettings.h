//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx Settings
// 
//*****************************************************************************

#pragma once

#include <float.h>
#include "VuEngine/Util/VuColor.h"


class VuGfxSettings
{
public:
	VuGfxSettings():
		mCameraFar(500.0f),
		mClearColor(0,0,0),
		mFogStart(FLT_MAX),
		mFogEnd(FLT_MAX),
		mFogColor(0,0,0),
		mDepthFogStart(2),
		mDepthFogDist(10),
		mDepthFogColor(0,0,0),
		mContrast(0,0,0),
		mTint(255,255,255),
		mGammaMin(0.0f),
		mGammaMax(1.0f),
		mGammaCurve(1.0f),
		mWaterAmbientColor(45,60,66,192),
		mWaterDiffuseColor(60,80,90,192),
		mWaterFoamAmbientColor(128,128,128,255),
		mWaterFoamDiffuseColor(255,255,255,255),
		mWaterFoamTextureSize(14.0f),
		mWaterDecalAmbientColor(128,128,128,255),
		mWaterDecalDiffuseColor(255,255,255,255),
		mDepthFoamValue(0.5f),
		mDepthFoamIntensity(0.5f),
		mPfxAmbientColor(192,192,192,255),
		mPfxDiffuseColor(255,255,255,255),
		mHBAORadius(2.1f),
		mHBAOMaxRadius(0.2f),
		mHBAOAngleBias(25.0f),
		mHBAOStrength(8.0f),
		mHBAOLightFactor(0.75f),
		mHBAOAmbientFactor(0.55f)
	{}

	float		mCameraFar;
	VuColor		mClearColor;
	float		mFogStart;
	float		mFogEnd;
	VuColor		mFogColor;
	float		mDepthFogStart;
	float		mDepthFogDist;
	VuColor		mDepthFogColor;
	VuColor		mContrast;
	VuColor		mTint;
	float		mGammaMin;
	float		mGammaMax;
	float		mGammaCurve;
	VuColor		mWaterAmbientColor;
	VuColor		mWaterDiffuseColor;
	VuColor		mWaterFoamAmbientColor;
	VuColor		mWaterFoamDiffuseColor;
	float		mWaterFoamTextureSize;
	VuColor		mWaterDecalAmbientColor;
	VuColor		mWaterDecalDiffuseColor;
	float		mDepthFoamValue;
	float		mDepthFoamIntensity;
	VuColor		mPfxAmbientColor;
	VuColor		mPfxDiffuseColor;
	float		mHBAORadius;		// Radius around each pixel with in which to select sample for AO calculation
	float		mHBAOMaxRadius;		// Maximum Radius for selecting samples
	float		mHBAOAngleBias;		// Change Bias for AO calculation
	float		mHBAOStrength;		// Change scene's AO strength
	float		mHBAOLightFactor;	// Factor applied to non-ambient lighting
	float		mHBAOAmbientFactor;	// Factor applied to ambient lighting

	bool hasDefaultColorCorrection() const
	{
		if ( mContrast.mR != 0 || mContrast.mG != 0 || mContrast.mB != 0 )
			return false;

		if ( mTint.mR != 255 || mTint.mG != 255 || mTint.mB != 255 )
			return false;

		if ( mGammaMin != 0.0f || mGammaMax  != 1.0f || mGammaCurve != 1.0f )
			return false;

		return true;
	}
};
