//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicLight class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Util/VuColor.h"

class VuDbvtNode;


class VuRenderLight
{
public:
	inline bool	testBounds(const VuMatrix &transform, const VuAabb &aabb) const;
	inline bool	testBounds(const VuAabb &aabb) const;
	inline bool	testBounds(const VuVector3 &point) const;
	void		debugDraw(const VuMatrix &viewProjMatrix) const;

	VuVector4	mPosition;
	VuVector4	mDirection;
	VuVector4	mDiffuseColor;
	VuVector4	mSpecularColor;
	VuVector4	mRange;
	VUUINT32	mGroup;
};

class VuShaderLights
{
public:
	enum { MAX_DYNAMIC_LIGHT_COUNT = 3 };

	VuVector4	mDirections[MAX_DYNAMIC_LIGHT_COUNT];
	VuVector4	mDiffuseColors[MAX_DYNAMIC_LIGHT_COUNT];
};

class VuDynamicLight
{
public:
	VuDynamicLight();
	virtual ~VuDynamicLight();

	// adding/removing from scene
	void				turnOn();
	void				turnOff();

	// call this after making changes
	void				update();

	const VuRenderLight	&getRenderLight() const { return mRenderLight; }

	VuVector3			mPosition;
	VuVector3			mDirection;
	VuColor				mDiffuseColor;
	VuColor				mSpecularColor;
	float				mFactor;
	float				mFalloffRangeMin;
	float				mFalloffRangeMax;
	float				mConeAngle;
	float				mPenumbraAngle;
	float				mDrawDistance;
	bool				mbReflecting;
	VUUINT32			mViewportMask;
	VUUINT32			mGroup;

protected:
	friend class VuLightManager;
	VuDbvtNode			*getDbvtNode()					{ return mpDbvtNode; }
	void				setDbvtNode(VuDbvtNode *pNode)	{ mpDbvtNode = pNode; }
	const VuAabb		&getAabb()						{ return mAabb; }

private:
	bool				mbRegistered;
	VuDbvtNode			*mpDbvtNode;
	VuAabb				mAabb;
	VuRenderLight		mRenderLight;
};

#include "VuDynamicLight.inl"
