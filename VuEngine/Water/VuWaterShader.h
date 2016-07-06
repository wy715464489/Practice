//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water shader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuPackedVector.h"

class VuCompiledShaderAsset;
class VuVertexDeclaration;
class VuTextureAsset;
class VuCubeTextureAsset;
class VuCubeTexture;
class VuTexture;
class VuMatrix;
class VuAabb;
class VuGfxSortMaterial;


//*****************************************************************************
// Description used for shader creation
//*****************************************************************************
class VuWaterShaderDesc
{
public:
	VuWaterShaderDesc() :
		mOverrideGlobalSettings(false),
		mAmbientColor(45,60,66,192),
		mDiffuseColor(60,80,90,192),
		mFoamAmbientColor(128,128,128,255),
		mFoamDiffuseColor(255,255,255,255),
		mFogEnabled(false),
		mFoamTextureSize(14.0f),
		mFoamTextureAsset("Water/WakeFroth"),
		mProceduralReflection(true),
		mNormalMapEnabled(true),
		mReflectionCubeTextureAsset("Proxy_cube"),
		mDecalAmbientColor(128,128,128,255),
		mDecalDiffuseColor(255,255,255,255),
		mDecalTextureAsset(""),
		mReceiveShadows(true),
		mFresnelFactor(1.0f),
		mFresnelMin(0.0f),
		mFresnelMax(1.0f)
	{
	}

	inline bool operator == (const VuWaterShaderDesc &rhs) const
	{
		if ( 
			mShaderNameOverride			!= rhs.mShaderNameOverride ||
			mOverrideGlobalSettings		!= rhs.mOverrideGlobalSettings ||
			mFogEnabled					!= rhs.mFogEnabled ||
			mFoamTextureAsset			!= rhs.mFoamTextureAsset ||
			mProceduralReflection		!= rhs.mProceduralReflection ||
			mNormalMapEnabled			!= rhs.mNormalMapEnabled ||
			mReflectionCubeTextureAsset	!= rhs.mReflectionCubeTextureAsset ||
			mDecalTextureAsset			!= rhs.mDecalTextureAsset ||
			mReceiveShadows				!= rhs.mReceiveShadows ||
			mFresnelFactor				!= rhs.mFresnelFactor ||
			mFresnelMin					!= rhs.mFresnelMin ||
			mFresnelMax					!= rhs.mFresnelMax )
		{
			return false;
		}

		if ( !mOverrideGlobalSettings )
			return true;

		return
			mAmbientColor		== rhs.mAmbientColor &&
			mDiffuseColor		== rhs.mDiffuseColor &&
			mFoamAmbientColor	== rhs.mFoamAmbientColor &&
			mFoamDiffuseColor	== rhs.mFoamDiffuseColor &&
			mFoamTextureSize	== rhs.mFoamTextureSize &&
			mDecalAmbientColor	== rhs.mDecalAmbientColor &&
			mDecalDiffuseColor	== rhs.mDecalDiffuseColor
		;
	}

	std::string	mShaderNameOverride;
	bool		mOverrideGlobalSettings;
	VuColor		mAmbientColor;
	VuColor		mDiffuseColor;
	VuColor		mFoamAmbientColor;
	VuColor		mFoamDiffuseColor;
	bool		mFogEnabled;
	float		mFoamTextureSize;
	std::string	mFoamTextureAsset;
	bool		mProceduralReflection;
	bool		mNormalMapEnabled;
	std::string	mReflectionCubeTextureAsset;
	VuColor		mDecalAmbientColor;
	VuColor		mDecalDiffuseColor;
	std::string	mDecalTextureAsset;
	bool		mReceiveShadows;
	float		mFresnelFactor;
	float		mFresnelMin;
	float		mFresnelMax;
};


//*****************************************************************************
// Water shader
//*****************************************************************************
class VuWaterShader : public VuRefObj
{
	friend class VuWater;

protected:
	VuWaterShader(const VuWaterShaderDesc &desc);
	~VuWaterShader();

public:
	//*************************************
	// internal interface
	//*************************************
	void					use(float fWaterZ, const VuMatrix &transform, const VuAabb &aabb) const;
	VuGfxSortMaterial		*getMaterial() const			{ return mpMaterial; }
	const VuWaterShaderDesc	&getDesc() const				{ return mDesc; }
	VuCubeTexture			*getReflectionCubeTexture() const;

private:
	typedef VuArray<float> HeightData;
	typedef VuArray<VuVector3> NormalData;

	VuTexture			*createFresnelTexture();

	VuWaterShaderDesc	mDesc;

	VuCompiledShaderAsset	*mpCompiledShaderAsset;
	VuTextureAsset			*mpFoamTextureAsset;
	VuCubeTextureAsset		*mpReflectionCubeTextureAsset;
	VuTextureAsset			*mpDecalTextureAsset;

	VuGfxSortMaterial	*mpMaterial;

	VuTexture			*mpFresnelTexture;

	// constants/samplers
	VUHANDLE			mhSpConstAmbientColor;
	VUHANDLE			mhSpConstDiffuseColor;
	VUHANDLE			mhSpConstFoamAmbientColor;
	VUHANDLE			mhSpConstFoamDiffuseColor;
	VUHANDLE			mhSpConstFoamTextureSize;
	VUHANDLE			mhSpConstFoamCenter;
	VUHANDLE			mhSpConstWaterZ;
	VUHANDLE			mhSpConstReflectionMapOffset;
	VUHANDLE			mhSpConstReflectionMapScale;
	VUHANDLE			mhSpConstFoamToNormalTextureScale;
	VUHANDLE			mhSpConstDecalAmbientColor;
	VUHANDLE			mhSpConstDecalDiffuseColor;
	VUINT				miSampFresnel;
	VUINT				miSampFoam;
	VUINT				miSampReflection;
	VUINT				miSampDecal;
	VUINT				miSampNormal;
};


//*****************************************************************************
// Water Shader Vertex
//*****************************************************************************
class VuWaterShaderVertex
{
public:
	VuPackedVector3	mPosition;
	VUUINT16		mNormalX;
	VUUINT16		mNormalY;
	VUUINT8			mFoam;
	VUUINT8			mShadow;
	VUUINT8			mDecal;
	VUUINT8			mPad0;
	VUUINT8			mLightR;
	VUUINT8			mLightG;
	VUUINT8			mLightB;
	VUUINT8			mPad1;
};
