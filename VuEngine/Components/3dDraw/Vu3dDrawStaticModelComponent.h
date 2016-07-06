//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawStaticModelComponent class
// 
//*****************************************************************************

#pragma once

#include "Vu3dDrawComponent.h"
#include "VuEngine/Gfx/Model/VuStaticModelInstance.h"

class Vu3dLayoutDrawParams;


class Vu3dDrawStaticModelComponent : public Vu3dDrawComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(StaticModel)
	DECLARE_RTTI

public:
	Vu3dDrawStaticModelComponent(VuEntity *pOwnerEntity);
	~Vu3dDrawStaticModelComponent();

	virtual void	onPostLoad() { modified(); }

	void			updateVisibility(const VuMatrix &mat);

	void			drawLayout(const Vu3dLayoutDrawParams &params);
	bool			collideLayout(const VuVector3 &v0, VuVector3 &v1);

	VuColor			getColor() { return VuLerp(mAmbientColor, mColor, mShadowValue); }
	void			setShadowValue(float shadowValue) { mShadowValue = shadowValue; }

	VuStaticModelInstance	&modelInstance() { return mModelInstance; }
	VuStaticModelInstance	&lod1ModelInstance() { return mLod1ModelInstance; }
	VuStaticModelInstance	&lod2ModelInstance() { return mLod2ModelInstance; }
	VuStaticModelInstance	&reflectionModelInstance() { return mReflectionModelInstance; }
	VuStaticModelInstance	&ultraModelInstance() { return mUltraModelInstance; }

	VuAabb					&getModelAabb() { return mModelAabb; }

	bool					castBakedShadow() { return mCastBakedShadow; }

protected:
	void					draw(const VuGfxDrawParams &params);
	void					drawShadow(const VuGfxDrawShadowParams &params);
	void					drawPrefetch();

	VuStaticModelInstance	*chooseModelToDraw(const VuVector3 &eyePos, bool bDrawReflection);
	void					modified();

	// properties
	std::string				mModelAssetName;
	std::string				mLod1ModelAssetName;
	std::string				mLod2ModelAssetName;
	std::string				mReflectionModelAssetName;
	std::string				mUltraModelAssetName;
	VuColor					mColor;
	VuColor					mAmbientColor;
	float					mDrawDist;
	float					mLod0DrawDist;
	float					mLod1DrawDist;
	float					mRejectionScaleModifier;
	bool					mUseLod1LowSpec;
	bool					mCastBakedShadow;

	VuStaticModelInstance	mModelInstance;
	VuStaticModelInstance	mLod1ModelInstance;
	VuStaticModelInstance	mLod2ModelInstance;
	VuStaticModelInstance	mReflectionModelInstance;
	VuStaticModelInstance	mUltraModelInstance;

	VuMatrix				mMatrix;
	float					mShadowValue;
	VuAabb					mModelAabb;
};
