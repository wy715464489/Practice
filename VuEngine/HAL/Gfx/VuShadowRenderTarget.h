//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ShadowRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"

class VuTexture;


class VuShadowRenderTarget : public VuRefObj
{
public:
	VuShadowRenderTarget(int width, int height, int count) : mWidth(width), mHeight(height), mCount(count) {}

	int						getWidth() { return mWidth; }
	int						getHeight() { return mHeight; }
	int						getCount() { return mCount; }

	virtual void			resolve(int layer) {}
	virtual VuTexture		*getColorTexture(int layer) { return VUNULL; }

protected:
	int	mWidth;
	int	mHeight;
	int	mCount;
};
