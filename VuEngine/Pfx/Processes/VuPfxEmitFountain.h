//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Fountain Emitter
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Util/VuColor.h"

class VuPfxParticle;


//*****************************************************************************
// base fountain
//*****************************************************************************
class VuPfxEmitFountain : public VuPfxProcess
{
	DECLARE_RTTI

public:
	VuPfxEmitFountain();

	float		mSpawnPerSecond;
	int			mMaxSpawnCount;
	float		mMinLifespan;
	float		mMaxLifespan;
	VuColor		mMinColor;
	VuColor		mMaxColor;
	float		mMinScale;
	float		mMaxScale;
	VuVector3	mMinLinearVelocity;
	VuVector3	mMaxLinearVelocity;
	VuVector3	mMinPosition;
	VuVector3	mMaxPosition;
	bool		mSpawnAtWaterSurface;
	float		mSpawnDistance;
};

class VuPfxEmitFountainInstance : public VuPfxProcessInstance
{
public:
	VuPfxEmitFountainInstance() : mSpawnCount(0), mSpawnAccum(0), mMaxSpawnCountMultiplier(1.0f), mSpawnPerSecondMultiplier(1.0f), mAlphaMultiplier(1.0f) {}

	virtual void		start();
	virtual void		tick(float fdt, bool ui);
	virtual void		onEmit(VuPfxParticle *pParticle) = 0;

	int					mSpawnCount;
	float				mSpawnAccum;
	float				mMaxSpawnCountMultiplier;
	float				mSpawnPerSecondMultiplier;
	float				mAlphaMultiplier;
};


//*****************************************************************************
// quad fountain
//*****************************************************************************
class VuPfxEmitQuadFountain : public VuPfxEmitFountain
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitQuadFountain();

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

class VuPfxEmitQuadFountainInstance : public VuPfxEmitFountainInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};


//*****************************************************************************
// geom fountain
//*****************************************************************************
class VuPfxEmitGeomFountain : public VuPfxEmitFountain
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitGeomFountain();

	VuVector3	mMinRotation;
	VuVector3	mMaxRotation;
	VuVector3	mMinAngularVelocity;
	VuVector3	mMaxAngularVelocity;
};

class VuPfxEmitGeomFountainInstance : public VuPfxEmitFountainInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};


//*****************************************************************************
// recursive fountain
//*****************************************************************************
class VuPfxEmitRecursiveFountain : public VuPfxEmitFountain
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitRecursiveFountain();

	VuVector3	mMinRotation;
	VuVector3	mMaxRotation;
	VuVector3	mMinAngularVelocity;
	VuVector3	mMaxAngularVelocity;
};

class VuPfxEmitRecursiveFountainInstance : public VuPfxEmitFountainInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};


//*****************************************************************************
// directional quad fountain
//*****************************************************************************
class VuPfxEmitDirectionalQuadFountain : public VuPfxEmitQuadFountain
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS
};

class VuPfxEmitDirectionalQuadFountainInstance : public VuPfxEmitQuadFountainInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};
