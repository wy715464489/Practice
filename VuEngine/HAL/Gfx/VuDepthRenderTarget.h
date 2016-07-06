//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DepthRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"

class VuTexture;


class VuDepthRenderTarget : public VuRefObj
{
public:
	VuDepthRenderTarget(int width, int height) : mWidth(width), mHeight(height) {}

	int					getWidth() { return mWidth; }
	int					getHeight() { return mHeight; }

	virtual VuTexture	*getTexture() = 0;

protected:
	int				mWidth;
	int				mHeight;
};
