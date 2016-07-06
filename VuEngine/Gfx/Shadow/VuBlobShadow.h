//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Blob Shadow
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"

class VuGfxSortMaterial;
class VuGfxDrawParams;
class btSphereShape;


class VuBlobShadow
{
public:
	struct VuParams
	{
		VuParams();

		VUUINT32	mCollisionMask;
		float		mAlpha;
		float		mScale;
		float		mDistanceFadeNear;
		float		mDistanceFadeFar;
		float		mHeightFadeNear;
		float		mHeightFadeFar;
		float		mOffsetZ;
	};

	VuBlobShadow();
	~VuBlobShadow();

	void			reset();

	void			setTexture(const std::string &assetName);

	VuParams		&params()       { return mParams; }
	const VuParams	&params() const { return mParams; }

	void			calculate(const VuMatrix &transform, const VuAabb &aabb);
	void			draw(const VuGfxDrawParams &params);

private:
	typedef VuArray<VuVector3> Verts;

	VuParams			mParams;
	VuGfxSortMaterial	*mpGfxSortMaterial;

	Verts				mVerts;
	VuMatrix			mTransform;
	VuAabb				mAabb;
	float				mHeightFade;
};
