//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  texture data container
// 
//*****************************************************************************

#include "VuTextureData.h"
#include "VuEngine/Gfx/Etc/VuEtc.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Gfx/Pvrtc/VuPvrtc.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


// static variables
bool VuTextureData::smVisualizeMipLevels = false;


//*****************************************************************************
VuTextureData::VuTextureData():
	mFormat(FORMAT_INVALID), mWidth(0), mHeight(0), mLevelCount(0),
	mData(0)
{
}

//*****************************************************************************
void VuTextureData::create(int width, int height, eFormat format, bool createMipMaps)
{
	mFormat = format;
	mWidth = width;
	mHeight = height;
	mLevelCount = createMipMaps ? VuHighBit(VuMax(mWidth, mHeight)) + 1 : 1;

	mData.resize(getTotalSize());
}

//*****************************************************************************
bool VuTextureData::build(const VuTgaLoader &tgaLoader, eFormat format, bool createMipMaps, const VuBuildParams &buildParams)
{
	// convert tga to rgba
	VuArray<VUBYTE> rgba;
	if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
		return false;

	return build(&rgba[0], tgaLoader.getWidth(), tgaLoader.getHeight(), format, createMipMaps, buildParams);
}

//*****************************************************************************
bool VuTextureData::build(const VUBYTE *rgba, int width, int height, eFormat format, bool createMipMaps, const VuBuildParams &buildParams)
{
	create(width, height, format, createMipMaps);

	if ( format == FORMAT_PVRTC_RGB || format == FORMAT_PVRTC_RGBA )
	{
		if ( width != height )
			return VUWARNING("PVRTC only supports square textures.");

		bool alphaOn = format == FORMAT_PVRTC_RGBA;
		bool assumeTiles = (buildParams.mHints & HINT_ASSUME_TILES) ? true : false;
		VuPvrtc::compressImage(rgba, width, height, mData, createMipMaps, alphaOn, assumeTiles);
	}
	else
	{
		// build level 0
		if ( !buildLevel(0, rgba, buildParams) )
			return false;

		// convert to floating point
		VuArray<float> frgba(0);
		frgba.resize(width*height*4);
		VuImageUtil::convertRGBAtoFRGBA(rgba, width, height, &frgba[0]);

		// build mip levels
		if ( !buildMipLevelRecursive(1, &frgba[0], buildParams) )
			return false;
	}

	return true;
}

//*****************************************************************************
void VuTextureData::buildMipLevels()
{
	for ( int level = 1; level < mLevelCount; level++ )
	{
		int srcWidth = getLevelWidth(level - 1);
		int srcHeight = getLevelHeight(level - 1);
		const VUBYTE *src = getLevelData(level - 1);
		VUBYTE *dst = getLevelData(level);

		if ( mFormat == FORMAT_RGBA || mFormat == FORMAT_ARGB )
			VuImageUtil::generateMipLevelRGBA(srcWidth, srcHeight, src, dst);
		else if ( mFormat == FORMAT_RGB )
			VuImageUtil::generateMipLevelRGB(srcWidth, srcHeight, src, dst);
		else if ( mFormat == FORMAT_RG )
			VuImageUtil::generateMipLevelRG(srcWidth, srcHeight, src, dst);
		else if ( mFormat == FORMAT_R )
			VuImageUtil::generateMipLevelR(srcWidth, srcHeight, src, dst);
		else
			VUASSERT(0, "format not supported");
	}
}

//*****************************************************************************
void VuTextureData::flipEndianness()
{
	for ( int level = 0; level < mLevelCount; level++ )
	{
		int width = getLevelWidth(level);
		int height = getLevelHeight(level);
		VUBYTE *data = getLevelData(level);

		if ( mFormat == FORMAT_RGBA || mFormat == FORMAT_ARGB )
			VuImageUtil::endianFlip4(data, width, height);
		else if ( mFormat == FORMAT_RG || mFormat == FORMAT_UV || mFormat == FORMAT_VU )
			VuImageUtil::endianFlip2(data, width, height);
		else if ( mFormat == FORMAT_565 || mFormat == FORMAT_5551 || mFormat == FORMAT_4444 )
			VuImageUtil::endianFlip2(data, width, height);
	}
}

//*****************************************************************************
void VuTextureData::load(VuBinaryDataReader &reader, int skipMipLevels)
{
	reader.readValue(mFormat);
	reader.readValue(mWidth);
	reader.readValue(mHeight);
	reader.readValue(mLevelCount);

	if ( skipMipLevels && mLevelCount > skipMipLevels )
	{
		int size;
		reader.readValue(size);

		int skipSize = 0;
		for ( int i = 0; i < skipMipLevels; i++ )
			skipSize += getLevelSize(i);

		int remainingSize = size - skipSize;

		reader.skip(skipSize);
		mData.resize(remainingSize);
		reader.readData(&mData[0], remainingSize);

		mWidth >>= skipMipLevels;
		mHeight >>= skipMipLevels;
		mLevelCount = mLevelCount - skipMipLevels;
	}
	else
	{
		reader.readArray(mData);
	}

	// debug
	if ( smVisualizeMipLevels )
		visualizeMipLevels();
}

//*****************************************************************************
void VuTextureData::save(VuBinaryDataWriter &writer)
{
	writer.writeValue(mFormat);
	writer.writeValue(mWidth);
	writer.writeValue(mHeight);
	writer.writeValue(mLevelCount);
	writer.writeArray(mData);
}

//*****************************************************************************
VUBYTE *VuTextureData::getLevelData(int level)
{
	VUASSERT(level < mLevelCount, "VuTextureData::getData() invalid level");

	int levelOffset = 0;
	for ( int i = 0; i < level; i++ )
		levelOffset += getLevelSize(i);

	return &mData[levelOffset];
}

//*****************************************************************************
int VuTextureData::getLevelWidth(int level)
{
	return VuMax(mWidth>>level, 1);
}

//*****************************************************************************
int VuTextureData::getLevelHeight(int level)
{
	return VuMax(mHeight>>level, 1);
}

//*****************************************************************************
int VuTextureData::getLevelBlockCount(int level)
{
	return ( ( getLevelWidth(level) + 3 )/4 ) * ( ( getLevelHeight(level) + 3 )/4 );
}

//*****************************************************************************
int VuTextureData::getLevelSize(int level)
{
	int levelWidth = getLevelWidth(level);
	int levelHeight = getLevelHeight(level);
	int levelBlockCount = getLevelBlockCount(level);

	int levelSize = 0;
	if ( mFormat == FORMAT_RGBA || mFormat == FORMAT_ARGB )
	{
		levelSize = levelWidth*levelHeight*4;
	}
	else if ( mFormat == FORMAT_RGB )
	{
		levelSize = levelWidth*levelHeight*3;
	}
	else if ( mFormat == FORMAT_RG || mFormat == FORMAT_UV || mFormat == FORMAT_VU )
	{
		levelSize = levelWidth*levelHeight*2;
	}
	else if ( mFormat == FORMAT_R )
	{
		levelSize = levelWidth*levelHeight*1;
	}
	else if ( mFormat == FORMAT_ETC1 )
	{
		levelSize = levelBlockCount*8;
	}
	else if ( mFormat == FORMAT_DXT1 )
	{
		levelSize = levelBlockCount*8;
	}
	else if ( mFormat == FORMAT_DXT5 )
	{
		levelSize = levelBlockCount*16;
	}
	else if ( mFormat == FORMAT_PVRTC_RGB || mFormat == FORMAT_PVRTC_RGBA )
	{
		levelSize = ( VuMax(levelWidth, 8) * VuMax(levelHeight, 8) * 4 + 7) / 8;
	}
	else if ( mFormat == FORMAT_565 || mFormat == FORMAT_5551 || mFormat == FORMAT_4444 )
	{
		levelSize = levelWidth*levelHeight*2;
	}
	else
	{
		VUASSERT(0, "format not supported");
	}
	
	return levelSize;
}

//*****************************************************************************
int VuTextureData::getLevelPitch(int level)
{
	int levelWidth = getLevelWidth(level);

	int pitch = 0;
	if ( mFormat == FORMAT_RGBA || mFormat == FORMAT_ARGB )
	{
		pitch = levelWidth*4;
	}
	else if ( mFormat == FORMAT_RGB )
	{
		pitch = levelWidth*3;
	}
	else if ( mFormat == FORMAT_RG || mFormat == FORMAT_UV || mFormat == FORMAT_VU )
	{
		pitch = levelWidth*2;
	}
	else if ( mFormat == FORMAT_R )
	{
		pitch = levelWidth*1;
	}
	else if ( mFormat == FORMAT_ETC1 )
	{
		pitch = ((levelWidth + 3) / 4) * 8;
	}
	else if ( mFormat == FORMAT_DXT1 )
	{
		pitch = ((levelWidth + 3) / 4) * 8;
	}
	else if ( mFormat == FORMAT_DXT5 )
	{
		pitch = ((levelWidth + 3) / 4) * 16;
	}
	else if ( mFormat == FORMAT_PVRTC_RGB || mFormat == FORMAT_PVRTC_RGBA )
	{
		pitch = ((levelWidth + 3) / 4) * 8;
	}
	else if ( mFormat == FORMAT_565 || mFormat == FORMAT_5551 || mFormat == FORMAT_4444 )
	{
		pitch = levelWidth*2;
	}
	else
	{
		VUASSERT(0, "format not supported");
	}
	
	return pitch;
}

//*****************************************************************************
int VuTextureData::getTotalSize()
{
	int totalSize = 0;
	for ( int level = 0; level < mLevelCount; level++ )
		totalSize += getLevelSize(level);

	return totalSize;
}

//*****************************************************************************
bool VuTextureData::buildLevel(int level, const VUBYTE *src, const VuBuildParams &buildParams)
{
	int levelWidth = getLevelWidth(level);
	int levelHeight = getLevelHeight(level);
	int levelSize = getLevelSize(level);
	
	(void)levelSize; // to suppress warning for unused variable on some platforms

	VUBYTE *dst = getLevelData(level);

	if ( mFormat == FORMAT_RGBA )
	{
		VU_MEMCPY(dst, levelSize, src, levelWidth*levelHeight*4);
	}
	else if ( mFormat == FORMAT_ARGB )
	{
		VuImageUtil::convertRGBAtoARGB(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_RGB )
	{
		VuImageUtil::convertRGBAtoRGB(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_RG )
	{
		VuImageUtil::convertRGBAtoRG(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_UV )
	{
		VuImageUtil::convertRGBAtoUV(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_VU )
	{
		VuImageUtil::convertRGBAtoVU(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_R )
	{
		VuImageUtil::convertRGBAtoR(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_ETC1 )
	{
		VuEtc::compressEtc1(src, levelWidth, levelHeight, dst, buildParams.mEtcParams);
	}
	else if ( mFormat == FORMAT_DXT1 )
	{
		VuDxt::compressImage(src, levelWidth, levelHeight, dst, VuDxt::DXT1);
	}
	else if ( mFormat == FORMAT_DXT5 )
	{
		VuDxt::compressImage(src, levelWidth, levelHeight, dst, VuDxt::DXT5);
	}
	else if ( mFormat == FORMAT_565 )
	{
		VuImageUtil::convertRGBAto565(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_5551 )
	{
		VuImageUtil::convertRGBAto5551(src, levelWidth, levelHeight, dst);
	}
	else if ( mFormat == FORMAT_4444 )
	{
		VuImageUtil::convertRGBAto4444(src, levelWidth, levelHeight, dst);
	}
	else
	{
		VUASSERT(0, "format not supported");
	}

	return true;
}

//*****************************************************************************
bool VuTextureData::buildMipLevelRecursive(int level, const float *src, const VuBuildParams &buildParams)
{
	if ( level == mLevelCount )
		return true;

	int srcWidth = getLevelWidth(level - 1);
	int srcHeight = getLevelHeight(level - 1);

	int dstWidth = getLevelWidth(level);
	int dstHeight = getLevelHeight(level);

	VuArray<float> frgba(0);
	frgba.resize(dstWidth*dstHeight*4);
	VuImageUtil::generateMipLevelFRGBA(srcWidth, srcHeight, src, &frgba[0]);

	VuArray<VUBYTE> rgba(0);
	rgba.resize(dstWidth*dstHeight*4);
	VuImageUtil::convertFRGBAtoRGBA(&frgba[0], dstWidth, dstHeight, &rgba[0]);

	if ( !buildLevel(level, &rgba[0], buildParams) )
		return false;

	return buildMipLevelRecursive(level + 1, &frgba[0], buildParams);
}

//*****************************************************************************
void VuTextureData::visualizeMipLevels()
{
	if ( mFormat == FORMAT_RGBA || mFormat == FORMAT_ARGB || mFormat == FORMAT_RGB || mFormat == FORMAT_DXT1 || mFormat == FORMAT_DXT5 )
	{
		for ( int level = 1; level < mLevelCount; level++ )
		{
			int levelWidth = getLevelWidth(level);
			int levelHeight = getLevelHeight(level);
			VUBYTE *levelData = getLevelData(level);

			// convert level data to temp RGBA
			VuArray<VUBYTE> rgba(0);
			rgba.resize(levelWidth*levelHeight*4);
			VUBYTE *tempData = &rgba[0];

			if ( mFormat == FORMAT_RGBA )
			{
				VU_MEMCPY(tempData, rgba.size(), levelData, levelWidth*levelHeight*4);
			}
			else if ( mFormat == FORMAT_ARGB )
			{
				VuImageUtil::convertARGBtoRGBA(levelData, levelWidth, levelHeight, tempData);
			}
			else if ( mFormat == FORMAT_RGB )
			{
				VuImageUtil::convertRGBtoRGBA(levelData, levelWidth, levelHeight, tempData);
			}
			else if ( mFormat == FORMAT_DXT1 )
			{
				VuDxt::decompressImage(tempData, levelWidth, levelHeight, levelData, VuDxt::DXT1);
			}
			else if ( mFormat == FORMAT_DXT5 )
			{
				VuDxt::decompressImage(tempData, levelWidth, levelHeight, levelData, VuDxt::DXT5);
			}
			else
			{
				VUASSERT(0, "format not supported");
			}

			// fill temp RGBA with solid color
			{
				VUBYTE color[3] = {0,0,0};
				if ( level%3 == 1 ) color[0] = 255;
				if ( level%3 == 2 ) color[1] = 255;
				if ( level%3 == 0 ) color[2] = 255;

				VUBYTE *dst = tempData;
				for ( int i = 0; i < levelWidth*levelHeight; i++ )
				{
					dst[0] = color[0];
					dst[1] = color[1];
					dst[2] = color[2];
					dst += 4;
				}
			}

			// convert temp RGBA back to level data
			if ( mFormat == FORMAT_RGBA )
			{
				VU_MEMCPY(levelData, levelWidth*levelHeight*4, tempData, levelWidth*levelHeight*4);
			}
			else if ( mFormat == FORMAT_ARGB )
			{
				VuImageUtil::convertRGBAtoARGB(tempData, levelWidth, levelHeight, levelData);
			}
			else if ( mFormat == FORMAT_RGB )
			{
				VuImageUtil::convertRGBAtoRGB(tempData, levelWidth, levelHeight, levelData);
			}
			else if ( mFormat == FORMAT_DXT1 )
			{
				VuDxt::compressImage(tempData, levelWidth, levelHeight, levelData, VuDxt::DXT1);
			}
			else if ( mFormat == FORMAT_DXT5 )
			{
				VuDxt::compressImage(tempData, levelWidth, levelHeight, levelData, VuDxt::DXT5);
			}
		}
	}
}