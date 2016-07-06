//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Trail Pattern
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"

class VuPfxTrailParticle;


class VuPfxTrailPattern : public VuPfxPattern
{
	DECLARE_RTTI
	DECLARE_PFX_PATTERN

public:
	VuPfxTrailPattern();

	enum eTrailType { TT_2D_TRAIL, TT_3D_RIBBON, TRAIL_TYPE_COUNT };
	enum eBlendMode { BM_ADDITIVE, BM_MODULATE, BLEND_MODE_COUNT };
	enum eSorting { SORT_ABOVE_WATER, SORT_BELOW_WATER, SORT_UI, SORTING_COUNT };

	// properties
	int			mTrailType;
	float		mLifespan;
	float		mFadeInTime;
	float		mFadeOutStartTime;
	VuColor		mColor;
	float		mWidth;
	VuVector3	mLinearVelocity;
	float		mSpawnDistance;
	std::string	mTextureAssetName;
	float		mTexCoordRate;
	int			mBlendMode;
	int			mSorting;

	// property references
	VuAssetProperty<VuTextureAsset>	*mpTextureAssetProperty;
};

class VuPfxTrailPatternInstance : public VuPfxPatternInstance
{
public:
	VuPfxTrailPatternInstance() : mSpawnAccum(0), mAliveTime(0), mpRootParticle(VUNULL) {}

	virtual void		start();
	virtual void		tick(float fdt, bool ui);
	virtual void		draw(const VuGfxDrawParams &params);

	float				mSpawnAccum;
	float				mAliveTime;
	VuPfxTrailParticle	*mpRootParticle;
};
