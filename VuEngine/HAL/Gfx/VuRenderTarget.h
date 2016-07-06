//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuColor.h"
#include "VuGfxTypes.h"

class VuTexture;
class VuRenderTarget;


struct VuSetRenderTargetParams
{
	explicit VuSetRenderTargetParams(VuRenderTarget *pRenderTarget):
		mpRenderTarget(pRenderTarget),
		mColorLoadAction(LoadActionDontCare), mClearColor(0,0,0),
		mDepthLoadAction(LoadActionDontCare), mClearDepth(1.0f)
	{}

	enum eLoadAction { LoadActionDontCare, LoadActionLoad, LoadActionClear };

	VuRenderTarget	*mpRenderTarget;
	eLoadAction		mColorLoadAction;
	VuColor			mClearColor;
	eLoadAction		mDepthLoadAction;
	float			mClearDepth;
};

class VuRenderTarget : public VuRefObj
{
public:
	VuRenderTarget(int width, int height) : mWidth(width), mHeight(height) {}

	int						getWidth() { return mWidth; }
	int						getHeight() { return mHeight; }

	virtual VuTexture		*getColorTexture() = 0;
	virtual void			readPixels(VuArray<VUBYTE> &rgb) = 0;

protected:
	int						mWidth;
	int						mHeight;
};
