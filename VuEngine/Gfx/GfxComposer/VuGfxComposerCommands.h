//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxComposerCommands implementation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuRect.h"

class VuRenderTarget;
class VuShadowRenderTarget;
class VuTexture;
class VuVector4;
class VuColor;


namespace VuGfxComposerSceneCommands
{
	void submitClear(VuRenderTarget *pRenderTarget);
	void submitReflectionClip(const VuVector4 &vPlane);
	void submitShadow(VuShadowRenderTarget *pShadowRenderTarget, int layer);
	void submitBeginEndScene(VUHANDLE context);
};

namespace VuGfxComposerPostProcessCommands
{
	void copy(VuTexture *pTexture, VuRenderTarget *pRenderTarget, int sequenceNo = 0);
	void radialBlur(VuTexture *pTexture, VuRenderTarget *pRenderTarget, float amount, int sequenceNo = 0);
	void colorCorrection(VuTexture *pTexture, VuRenderTarget *pRenderTarget, const VuColor &contrast, const VuColor &tint, float gammaMin, float gammaMax, float gammaCurve, int sequenceNo = 0);
	void antiAlias(VuTexture *pTexture, VuRenderTarget *pRenderTarget);
	void blur(VuRenderTarget *pRenderTarget0, VuRenderTarget *pRenderTarget1, float amount, int sequenceNo = 0);

	struct CopyMultiParams
	{
		CopyMultiParams() : mCount(0) {}
		void add(VuTexture *pTexture, const VuRect &dstRect)
		{
			VUASSERT(mCount < 8, "CopyMultiParams count exceeded");
			mTextures[mCount] = pTexture;
			mDstRects[mCount] = dstRect;
			mCount++;
		}
		int			mCount;
		VuTexture	*mTextures[8];
		VuRect		mDstRects[8];
	};
	void copyMulti(VuRenderTarget *pRenderTarget, const CopyMultiParams &params, int sequenceNo = 0);
};
