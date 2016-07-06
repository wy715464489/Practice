//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the texture interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuTexture.h"

class VuMetalSamplerState;


class VuMetalTexture : public VuTexture
{
public:
	VuMetalTexture(int width, int height, int levelCount);
	~VuMetalTexture();
	
	virtual void		setData(int level, const void *pData, int size);
	
	static VuMetalTexture	*load(VuBinaryDataReader &reader, int skipLevels);
	static VuMetalTexture	*create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state);
	
	id<MTLTexture>		mMTLTexture;
	VuMetalSamplerState	*mpSamplerState;
};
