//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Drop Shadow
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"

class VuGfxSortMaterial;
class VuGfxDrawParams;
class VuRenderTarget;
class VuGfxDrawShadowParams;
class VuColor;
class btSphereShape;


class VuDropShadow
{
public:
	struct VuParams
	{
		VuParams();

		VUUINT32	mCollisionMask;
		float		mAlpha;
		float		mDistanceFadeNear;
		float		mDistanceFadeFar;
		float		mHeightFadeNear;
		float		mHeightFadeFar;
		float		mOffsetZ;
	};

	VuDropShadow(int textureSize);
	~VuDropShadow();

	VuParams		&params()       { return mParams; }
	const VuParams	&params() const { return mParams; }

	void			calculate(const VuMatrix &transform, const VuAabb &aabb);

	class Callback { public: virtual void onDropShadowDraw(const VuGfxDrawShadowParams &params) = 0; };
	void			draw(const VuGfxDrawParams &params, Callback *pCB);

private:
	void			submitClearCommand();
	void			submitShadow(const VuMatrix &textureMatrix, const VuColor &color);

	void			calcLightMatrix(const VuVector3 &lightPos, const VuVector3 &lightDir, VuMatrix &lightMatrix);
	void			calcLightAabb(const VuMatrix &lightMatrix, VuAabb &lightAabb);
	void			calcLightCropMatrix(const VuAabb &lightAabb, const VuMatrix &lightMatrix, VuMatrix &lightCropMatrix);
	void			calcTextScaleBiasMatrix(VuMatrix &texScaleBiasMatrix);

	typedef VuArray<VuVector3> Verts;

	VuParams			mParams;
	int					mTextureSize;
	VuGfxSortMaterial	*mpGfxSortMaterial;
	VuRenderTarget		*mpRenderTarget;

	Verts				mVerts;

	VuVector3			mObjectPos;
	float				mObjectRadius;
	float				mHeightFade;
};
