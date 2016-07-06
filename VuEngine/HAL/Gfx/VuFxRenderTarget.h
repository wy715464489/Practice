//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  FxRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuGfxTypes.h"

class VuTexture;


class VuFxRenderTarget : public VuRefObj
{
public:
	VuFxRenderTarget(int width, int height, VuGfxFormat format) : mWidth(width), mHeight(height), mFormat(format) {}

	int					getWidth() { return mWidth; }
	int					getHeight() { return mHeight; }

	virtual VuTexture	*getTexture() { return VUNULL; }

protected:
	int				mWidth;
	int				mHeight;
	VuGfxFormat		mFormat;
};
