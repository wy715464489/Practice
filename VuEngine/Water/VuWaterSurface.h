//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water surface class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Util/VuColor.h"


class VuWaterSurfaceDesc;
class VuDbrt;
class VuDbrtNode;
class VuWaterMapAsset;
class VuLightMapAsset;
class VuEntity;


//*****************************************************************************
// Description used for surface creation
//*****************************************************************************
class VuWaterSurfaceDesc
{
public:
	VuWaterSurfaceDesc() :
		mPos(0,0,0),
		mRotZ(0.0f),
		mPowSizeX(0),
		mPowSizeY(0),
		mMaxWaveDepth(0.0f),
		mMaxWaveHeight(0.0f),
		mReflectionHeight(0.0f),
		mReflectionOffset(0.0f),
		mMinRecursionDepth(0),
		mProceduralReflection(false),
		mpWaterMapAsset(VUNULL),
		mFlags(0)
	{
	}

	VuVector3		mPos;
	float			mRotZ;
	VUUINT			mPowSizeX;
	VUUINT			mPowSizeY;
	float			mMaxWaveDepth;
	float			mMaxWaveHeight;
	float			mReflectionHeight;
	float			mReflectionOffset;
	int				mMinRecursionDepth;
	bool			mProceduralReflection;
	VuWaterMapAsset	*mpWaterMapAsset;
	VuLightMapAsset	*mpLightMapAsset;
	VUUINT32		mFlags;
};


//*****************************************************************************
// Water surface interface
//*****************************************************************************
class VuWaterSurface : public VuRefObj
{
	friend class VuWater;

protected:
	VuWaterSurface(const VuWaterSurfaceDesc &desc, VuEntity *pOwnerEntity);
	~VuWaterSurface();

public:
	//*************************************
	// public interface
	//*************************************
	void						modify(const VuWaterSurfaceDesc &desc);
	const VuWaterSurfaceDesc	&getDesc() const { return mDesc; }
	VUUINT8						getShadow(const VuVector3 &vPos) const;
	VuColor						getLight(const VuVector3 &vPos) const;
	VuEntity					*getOwnerEntity() const	{ return mpOwnerEntity; }

	//*************************************
	// internal interface
	//*************************************
	float				calcDistance2d(const VuVector3 &vPos) const	{ return VuSqrt(calcDistance2dSquared(vPos)); }
	float				calcDistance2dSquared(const VuVector3 &vPos) const;

	float				calcDistance3d(const VuVector3 &vPos) const	{ return VuSqrt(calcDistance3dSquared(vPos)); }
	float				calcDistance3dSquared(const VuVector3 &vPos) const;

	float				calcReflectionDistance3dSquared(const VuVector3 &vPos) const;

	VuWaterSurfaceDesc	mDesc;
	VuEntity			*mpOwnerEntity;
	int					mSizeX;
	int					mSizeY;
	VuMatrix			mTransform;
	VuMatrix			mInverseTransform;
	VuAabb				mWorldAabb;
	VuVector2			mExtent;
	VuDbrtNode			*mpDbrtNode;
	VuDbrt				*mpWaveDbrt;
	VuWaterMapAsset		*mpWaterMap;
	VuLightMapAsset		*mpLightMap;
};
