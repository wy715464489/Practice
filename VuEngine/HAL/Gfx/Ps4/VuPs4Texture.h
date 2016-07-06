//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the texture interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>

#include "VuEngine/HAL/Gfx/VuTexture.h"

using namespace sce;

class VuPs4Texture : public VuTexture
{
public:
	VuPs4Texture(int width, int height, int levelCount, const VuTextureState &state);
	~VuPs4Texture();

	virtual void	setData(int level, const void *pData, int size);

	static VuPs4Texture	*load(VuBinaryDataReader &reader, int skipLevels);
	static VuPs4Texture	*create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state);

	Gnm::Texture	mGnmTexture;
	Gnm::Sampler	mGnmSampler;
	void			*mpGnmMemory;
};
