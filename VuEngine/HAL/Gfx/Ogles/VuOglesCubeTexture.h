//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpgnGL ES implementation of the cube texture interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"


class VuOglesCubeTexture : public VuCubeTexture
{
public:
	VuOglesCubeTexture(int edgeLength, int levelCount, const VuTextureState &state);
	~VuOglesCubeTexture();

	static VuOglesCubeTexture	*load(VuBinaryDataReader &reader, int skipLevels);

	void				loadTextureDataIntoVRAM(VuTextureData *pTextureDataArray);

	GLuint				mGlTexture;
	GLenum				mGlFormat;
	GLenum				mGlType;
	bool				mbCompressed;
	GLint				mMinFilter;
	GLint				mMagFilter;
	GLint				mWrapS;
	GLint				mWrapT;
};
