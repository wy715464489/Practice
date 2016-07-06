//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawAnimatedModelComponent class
// 
//*****************************************************************************

#pragma once

#include "Vu3dDrawComponent.h"
#include "VuEngine/Method/VuParams.h"
#include "VuEngine/Gfx/Model/VuAnimatedModelInstance.h"

class Vu3dLayoutDrawParams;


class Vu3dDrawAnimatedModelComponent : public Vu3dDrawComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(AnimatedModel)
	DECLARE_RTTI

public:
	Vu3dDrawAnimatedModelComponent(VuEntity *pOwnerEntity);
	~Vu3dDrawAnimatedModelComponent();

	virtual void			onPostLoad() { modified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

	void					updateVisibility(const VuMatrix &mat);
	void					drawLayout(const Vu3dLayoutDrawParams &params);
	void					setAlpha(float alpha) { mAlpha = alpha; }
	void					setAdditiveAlpha(float alpha) { mAdditiveAlpha = alpha; }

	const std::string		&getModelAssetName() { return mModelAssetName; }
	VuAnimatedModelInstance	&getModelInstance() { return mModelInstance; }
	VuAnimatedSkeleton		*getAnimatedSkeleton() { return mpAnimatedSkeleton; }

protected:
	void					draw(const VuGfxDrawParams &params);
	void					drawShadow(const VuGfxDrawShadowParams &params);
	void					drawPrefetch();

	void					modified();

	// properties
	std::string				mModelAssetName;
	VuColor					mColor;
	float					mAlpha;
	float					mAdditiveAlpha;
	float					mDrawDist;

	VuAnimatedModelInstance	mModelInstance;
	VuAnimatedSkeleton		*mpAnimatedSkeleton;

	VuMatrix				mMatrix;
};
