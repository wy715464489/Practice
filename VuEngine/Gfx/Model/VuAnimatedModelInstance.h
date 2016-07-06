//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Model Instance
// 
//*****************************************************************************

#pragma once

#include "VuModelInstance.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuMatrix.h"

class VuAnimatedModelAsset;
class VuSkeleton;
class VuGfxAnimatedScene;
class VuAnimatedSkeleton;
class VuRagdoll;


class VuAnimatedModelInstance : public VuModelInstance
{
public:
	VuAnimatedModelInstance();
	~VuAnimatedModelInstance();

	void					reset();

	// the container can be set either by specifying the model asset or directly
	void					setModelAsset(const std::string &assetName);
	void					setModel(VuSkeleton *pSkeleton, VuGfxAnimatedScene *pGfxAnimatedScene);

	VuAnimatedModelAsset	*getModelAsset() const			{ return mpModelAsset; }
	VuSkeleton				*getSkeleton() const			{ return mpSkeleton; }
	VuGfxAnimatedScene		*getGfxAnimatedScene() const	{ return mpGfxAnimatedScene; }
	const VuAabb			&getLocalAabb() const			{ return mLocalAabb; }
	const VuMatrix			&getRootTransform() const		{ return mRootTransform; }

	// pose
	void					setPose(const VuAnimatedSkeleton *pAnimatedSkeleton);
	void					setPose(const VuMatrix &modelMat, const VuRagdoll *pRagdoll);
	void					copyPose(const VuAnimatedModelInstance *pFrom);
	void					finalizePose();
	VuMatrix				*getModelMatrices() const { return mpModelMatrices; }

	// draw
	void					draw(const VuMatrix &modelMat, const VuGfxDrawParams &params) const;
	void					drawShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const;
	void					drawDropShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const;
	void					drawPrefetch() const;
	void					drawInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const;

private:
	void					drawBoneInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const;

	VuAnimatedModelAsset	*mpModelAsset;
	VuSkeleton				*mpSkeleton;
	VuGfxAnimatedScene		*mpGfxAnimatedScene;
	VuMatrix				*mpModelMatrices;
	VuMatrix				*mpRenderMatrices[2];
	int						mCurSimRenderMatrices;
	int						mCurGfxRenderMatrices;
	VuAabb					mLocalAabb;
	float					mExtraAabbExtent;
	VuMatrix				mRootTransform;
};
