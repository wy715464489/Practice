//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxDrawParams
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Util/VuColor.h"

class VuCamera;
class VuShadowClip;
class VuShadowVolume;
class VuMatrix;
class VuAabb;


class VuGfxDrawParams
{
public:
	explicit VuGfxDrawParams(const VuCamera &camera);

	bool			isVisible(const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const;

	VuVector3		mEyePos;
	const VuCamera	&mCamera;
	float			mRejectionScale;
	bool			mbDrawReflection;
	VuVector4		mReflectionPlane;
	bool			mbDrawSSAO;
	VUUINT32		mZoneMask;
};

class VuGfxDrawShadowParams
{
public:
	explicit VuGfxDrawShadowParams(const VuCamera &camera, const VuShadowClip &combinedShadowClip);

	bool					isVisible(const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const;
	bool					isVisible(int volume, const VuAabb &aabb, const VuMatrix &matWorld, float rejectionScaleModifier) const;

	VuVector3				mEyePos;
	const VuCamera			&mCamera;
	const VuShadowClip		&mCombinedShadowClip;
	int						mShadowVolumeCount;
	const VuShadowVolume	*mpShadowVolumes;
	float					mRejectionScale;
	bool					mbDrawReflection;
	VuVector4				mReflectionPlane;
	bool					mbDrawCollision;
	VUUINT32				mZoneMask;
};

class VuGfxDrawInfoParams
{
public:
	explicit VuGfxDrawInfoParams(const VuCamera &pCamera);

	enum {
		INSTANCE_NAMES		= (1<<0),
		MESH_NAMES			= (1<<1),
		MESH_BOUNDS			= (1<<2),
		MESH_PART_BOUNDS	= (1<<3),
		SCENE_INFO			= (1<<4),
		BONES				= (1<<5),
		BONE_NAMES			= (1<<6),
		ANIMATION_TIMELINE	= (1<<7),
	};

	VUUINT32		mFlags;
	const VuCamera	&mCamera;
	VuColor			mDevTextColor;
	VuColor			mDevLineColor;
	float			mBoneSize;
};
