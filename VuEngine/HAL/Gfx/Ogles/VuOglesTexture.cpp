//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the texture interface class.
//
//*****************************************************************************

#include "VuOglesTexture.h"
#include "VuOglesGfxTypes.h"
#include "VuOglesGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"


//*****************************************************************************
VuOglesTexture::VuOglesTexture(int width, int height, int levelCount, const VuTextureState &state):
	VuTexture(width, height, levelCount),
	mGlFormat(-1),
	mGlType(-1),
	mbCompressed(false),
	mbDynamic(false)
{
	// when mipmapping, only support power of two textures
	VUASSERT(levelCount == 1 || (VuBitCount(width) == 1 && VuBitCount(height) == 1), "Only support power-of-2 textures when mip mapping.");

	mMinFilter = VuOglesGfxTypes::convert(state.mMinFilter, state.mMipFilter);
	mMagFilter = VuOglesGfxTypes::convert(state.mMagFilter);
	mWrapS = VuOglesGfxTypes::convert(state.mAddressU);
	mWrapT = VuOglesGfxTypes::convert(state.mAddressV);

	// create texture
	glGenTextures(1, &mGlTexture);

	// set texture parameters
	glBindTexture(GL_TEXTURE_2D, mGlTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);
}

//*****************************************************************************
VuOglesTexture::~VuOglesTexture()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteTextures(1, &mGlTexture);
}

//*****************************************************************************
void VuOglesTexture::setData(int level, const void *pData, int size)
{
	int levelWidth = VuMax(mWidth >> level, 1);
	int levelHeight = VuMax(mHeight >> level, 1);

	VUASSERT(level < mLevelCount, "VuOglesTexture::setData() invalid level");

	glBindTexture(GL_TEXTURE_2D, mGlTexture);
	glTexImage2D(GL_TEXTURE_2D, level, mGlFormat, levelWidth, levelHeight, 0, mGlFormat, mGlType, pData);
}

//*****************************************************************************
VuOglesTexture *VuOglesTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	int width, height, levelCount;
	reader.readValue(width);
	reader.readValue(height);
	reader.readValue(levelCount);
	
	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		width = VuMax(width>>skipLevels, 1);
		height = VuMax(height>>skipLevels, 1);
		levelCount -= skipLevels;
	}

	VuOglesTexture *pTexture = new VuOglesTexture(width, height, levelCount, state);

	reader.readValue(pTexture->mGlFormat);
	reader.readValue(pTexture->mGlType);
	reader.readValue(pTexture->mbCompressed);

	VuTextureData textureData;
	textureData.load(reader, skipLevels);
	pTexture->loadTextureDataIntoVRAM(textureData);

	return pTexture;
}

//*****************************************************************************
VuOglesTexture *VuOglesTexture::create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;

	int levels = createMipMaps ? VuHighBit(VuMax(width, height)) + 1 : 1;
	VuOglesTexture *pTexture = new VuOglesTexture(width, height, levels, state);

	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
		pTexture->mbDynamic = true;

	// determine Ogl format
	if ( format == VUGFX_FORMAT_LIN_R8 )
	{
		pTexture->mGlFormat = GL_LUMINANCE;
		pTexture->mGlType = GL_UNSIGNED_BYTE;
	}
	else if ( format == VUGFX_FORMAT_A8R8G8B8 )
	{
		pTexture->mGlFormat = GL_RGBA;
		pTexture->mGlType = GL_UNSIGNED_BYTE;
	}
	else if ( format == VUGFX_FORMAT_LIN_L8A8 )
	{
		pTexture->mGlFormat = GL_LUMINANCE_ALPHA;
		pTexture->mGlType = GL_UNSIGNED_BYTE;
	}
	else
	{
		VUASSERT(0, "Unsupported texture format.");
	}

	return pTexture;
}

//*****************************************************************************
void VuOglesTexture::loadTextureDataIntoVRAM(VuTextureData &textureData)
{
	glBindTexture(GL_TEXTURE_2D, mGlTexture);

	bool isDxt = (mGlFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) || (mGlFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
	if ( isDxt && !VuOglesGfx::IF()->getDxtCompression() )
	{
		// if DXT compression is not supported, uncompress

		VuArray<VUBYTE> texData(0);
		texData.resize(mWidth*mHeight*4);

		for ( int level = 0; level < mLevelCount; level++ )
		{
			int levelWidth = textureData.getLevelWidth(level);
			int levelHeight = textureData.getLevelHeight(level);
			const void *levelData = textureData.getLevelData(level);

			if ( mGlFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) // 565
			{
				VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT1);
				VuImageUtil::convertRGBAto565(&texData.begin(), levelWidth, levelHeight, &texData.begin());
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, levelWidth, levelHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, &texData.begin());
			}
			else if ( mGlFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) // 8888
			{
				VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT5);
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, levelWidth, levelHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texData.begin());
			}
		}
	}
	else
	{
		for ( int level = 0; level < mLevelCount; level++ )
		{
			int levelWidth = textureData.getLevelWidth(level);
			int levelHeight = textureData.getLevelHeight(level);
			int levelSize = textureData.getLevelSize(level);
			const void *levelData = textureData.getLevelData(level);

			if ( mbCompressed )
				glCompressedTexImage2D(GL_TEXTURE_2D, level, mGlFormat, levelWidth, levelHeight, 0, levelSize, levelData);
			else
				glTexImage2D(GL_TEXTURE_2D, level, mGlFormat, levelWidth, levelHeight, 0, mGlFormat, mGlType, levelData);
		}
	}
}
