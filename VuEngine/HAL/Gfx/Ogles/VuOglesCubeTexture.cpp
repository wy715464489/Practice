//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the cube texture interface class.
// 
//*****************************************************************************

#include "VuOglesCubeTexture.h"
#include "VuOglesGfxTypes.h"
#include "VuOglesGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"


static GLenum sFaceTextureTargets[6] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};


//*****************************************************************************
VuOglesCubeTexture::VuOglesCubeTexture(int edgeLength, int levelCount, const VuTextureState &state):
	VuCubeTexture(edgeLength, levelCount),
	mGlFormat(-1),
	mGlType(-1),
	mbCompressed(false)
{
	mMinFilter = VuOglesGfxTypes::convert(state.mMinFilter, state.mMipFilter);
	mMagFilter = VuOglesGfxTypes::convert(state.mMagFilter);
	mWrapS = VuOglesGfxTypes::convert(state.mAddressU);
	mWrapT = VuOglesGfxTypes::convert(state.mAddressV);

	// create texture
	glGenTextures(1, &mGlTexture);

	// set texture parameters
	glBindTexture(GL_TEXTURE_CUBE_MAP, mGlTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mMagFilter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mWrapS);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mWrapT);
}

//*****************************************************************************
VuOglesCubeTexture::~VuOglesCubeTexture()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteTextures(1, &mGlTexture);
}

//*****************************************************************************
VuOglesCubeTexture *VuOglesCubeTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	int edgeLength, levelCount;
	reader.readValue(edgeLength);
	reader.readValue(levelCount);

	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		edgeLength = VuMax(edgeLength>>skipLevels, 1);
		levelCount -= skipLevels;
	}
	
	VuOglesCubeTexture *pCubeTexture = new VuOglesCubeTexture(edgeLength, levelCount, state);

	reader.readValue(pCubeTexture->mGlFormat);
	reader.readValue(pCubeTexture->mGlType);
	reader.readValue(pCubeTexture->mbCompressed);

	VuTextureData textureData[6];
	for ( int iFace = 0; iFace < 6; iFace++ )
		textureData[iFace].load(reader, skipLevels);
	pCubeTexture->loadTextureDataIntoVRAM(textureData);

	return pCubeTexture;
}

//*****************************************************************************
void VuOglesCubeTexture::loadTextureDataIntoVRAM(VuTextureData *pTextureDataArray)
{
	// load texture levels for each face
	glBindTexture(GL_TEXTURE_CUBE_MAP, mGlTexture);

	bool isDxt = (mGlFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) || (mGlFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);

	for ( int iFace = 0; iFace < 6; iFace++ )
	{
		VuTextureData *pTextureData = &pTextureDataArray[iFace];
		if ( isDxt && !VuOglesGfx::IF()->getDxtCompression() )
		{
			// if DXT compression is not supported, uncompress

			VuArray<VUBYTE> texData(0);
			texData.resize(mEdgeLength*mEdgeLength*4);

			for ( int level = 0; level < mLevelCount; level++ )
			{
				int levelWidth = pTextureData->getLevelWidth(level);
				int levelHeight = pTextureData->getLevelHeight(level);
				const void *levelData = pTextureData->getLevelData(level);

				if ( mGlFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) // 565
				{
					VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT1);
					VuImageUtil::convertRGBAto565(&texData.begin(), levelWidth, levelHeight, &texData.begin());
					glTexImage2D(sFaceTextureTargets[iFace], level, GL_RGB, levelWidth, levelHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, &texData.begin());
				}
				else if ( mGlFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) // 8888
				{
					VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT5);
					glTexImage2D(sFaceTextureTargets[iFace], level, GL_RGBA, levelWidth, levelHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texData.begin());
				}
			}
		}
		else
		{
			for ( int level = 0; level < mLevelCount; level++ )
			{
				int levelWidth = pTextureData->getLevelWidth(level);
				int levelHeight = pTextureData->getLevelHeight(level);
				int levelSize = pTextureData->getLevelSize(level);
				const void *levelData = pTextureData->getLevelData(level);

				if ( mbCompressed )
					glCompressedTexImage2D(sFaceTextureTargets[iFace], level, mGlFormat, levelWidth, levelHeight, 0, levelSize, levelData);
				else
					glTexImage2D(sFaceTextureTargets[iFace], level, mGlFormat, levelWidth, levelHeight, 0, mGlFormat, mGlType, levelData);
			}
		}
	}
}
