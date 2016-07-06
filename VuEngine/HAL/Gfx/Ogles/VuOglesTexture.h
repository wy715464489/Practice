//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the texture interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"


class VuOglesTexture : public VuTexture
{
public:
	VuOglesTexture(int width, int height, int levelCount, const VuTextureState &state);
	~VuOglesTexture();

	virtual void		setData(int level, const void *pData, int size);

	static VuOglesTexture		*load(VuBinaryDataReader &reader, int skipLevels);
	static VuOglesTexture		*create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state);

	void				loadTextureDataIntoVRAM(VuTextureData &textureData);

	GLuint				mGlTexture;
	GLenum				mGlFormat;
	GLenum				mGlType;
	bool				mbCompressed;
	bool				mbDynamic;
	GLint				mMinFilter;
	GLint				mMagFilter;
	GLint				mWrapS;
	GLint				mWrapT;
};
