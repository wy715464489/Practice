//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Corona
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Util/VuColor.h"

class VuMatrix;
class VuGfxDrawParams;
class VuGfxSortMaterial;


class VuCorona
{
public:
	VuCorona();
	~VuCorona();

	void		setTextureAsset(const std::string &assetName);
	void		updateVisibility(const VuVector3 &position);
	void		draw(const VuMatrix &transform, const VuGfxDrawParams &params);

	float		mDrawDist;
	float		mFadeDist;
	float		mQueryRadius;
	float		mConeAngle;
	float		mPenumbraAngle;
	bool		mbEnableBackLight;
	VuColor		mTextureColor;
	bool		mbTextureSizeScreenSpace;
	float		mTextureSize;
	float		mRotationOffset;
	float		mRotationAmount;
	VUUINT32	mCollisionMask;

private:
	VuGfxSortMaterial	*mpGfxSortMaterial;

	class VuQuery
	{
	public:
		VuQuery() : mbTestVisibility(false), mVisibility(0.0f) {}
		bool	mbTestVisibility;
		float	mVisibility;
	};
	VuQuery				mQueries[VuViewportManager::MAX_VIEWPORTS];

	struct DrawCallbackData
	{
		VuVector3	mPosition;
		VuColor		mColor;
		float		mSize;
		float		mRotationOffset;
		float		mRotationAmount;
	};
	static void			drawCallback(void *data);
};