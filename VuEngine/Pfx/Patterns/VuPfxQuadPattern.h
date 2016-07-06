//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Quad Pattern
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Math/VuMatrix.h"


class VuPfxQuadPattern : public VuPfxPattern
{
	DECLARE_RTTI
	DECLARE_PFX_PATTERN

public:
	VuPfxQuadPattern();

	enum eBlendMode { BM_ADDITIVE, BM_MODULATE, BLEND_MODE_COUNT };
	enum eSorting { SORT_ABOVE_WATER, SORT_BELOW_WATER, SORT_UI, SORTING_COUNT };

	// properties
	std::string						mTextureAssetName;
	std::string						mTileTextureAssetName;
	int								mBlendMode;
	int								mSorting;
	float							mClipThreshold;
	float							mNearFadeMin;
	float							mNearFadeMax;
	float							mTileScrollSpeedU;
	float							mTileScrollSpeedV;
	float							mTileScrollLoopTime;
	float							mTileScale;
	float							mMaxStretch;
	bool							mFogEnabled;
	VuVector2						mCenterOffset;

	// property references
	VuAssetProperty<VuTextureAsset>	*mpTextureAssetProperty;
	VuAssetProperty<VuTextureAsset>	*mpTileTextureAssetProperty;
};

class VuPfxQuadPatternInstance : public VuPfxPatternInstance
{
public:
	virtual void	start();
	virtual void	tick(float fdt, bool ui);
	virtual void	draw(const VuGfxDrawParams &params);
};

class VuPfxOrbitQuadPattern : public VuPfxQuadPattern
{
	DECLARE_RTTI
	DECLARE_PFX_PATTERN

public:
	VuPfxOrbitQuadPattern();

	// properties
	float		mOrbitalRadius;
	VuVector3	mOrbitalCenter;
	VuVector3	mOrbitalRotation;
	float		mOrbitalVelocity;
	float		mMinLifespan;
	float		mMaxLifespan;
	VuColor		mMinColor;
	VuColor		mMaxColor;
	float		mMinScale;
	float		mMaxScale;
	float		mMinRotation;
	float		mMaxRotation;
	float		mMinAngularVelocity;
	float		mMaxAngularVelocity;
	float		mMinWorldScaleZ;
	float		mMaxWorldScaleZ;
	float		mMinDirStretch;
	float		mMaxDirStretch;
	float		mMinTileOffsetU;
	float		mMaxTileOffsetU;
	float		mMinTileOffsetV;
	float		mMaxTileOffsetV;
};

class VuPfxOrbitQuadPatternInstance : public VuPfxQuadPatternInstance
{
public:
	virtual void	tick(float fdt, bool ui);
	void			createParticles();
};
