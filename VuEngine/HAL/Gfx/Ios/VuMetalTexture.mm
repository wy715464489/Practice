//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the texture interface class.
//
//*****************************************************************************

#include "VuMetalTexture.h"
#include "VuMetalSamplerState.h"
#include "VuMetalGfx.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuMetalTexture::VuMetalTexture(int width, int height, int levelCount):
	VuTexture(width, height, levelCount)
{
}

//*****************************************************************************
VuMetalTexture::~VuMetalTexture()
{
	mpSamplerState->removeRef();
}

//*****************************************************************************
void VuMetalTexture::setData(int level, const void *pData, int size)
{
	VUASSERT(level < mLevelCount, "VuMetalTexture::setData() invalid level");

	int levelWidth = VuMax(mWidth >> level, 1);
	int levelHeight = VuMax(mHeight >> level, 1);

	int bytesPerRow = size/levelHeight;

	[mMTLTexture replaceRegion:MTLRegionMake2D(0, 0, levelWidth, levelHeight) mipmapLevel:level withBytes:pData bytesPerRow:bytesPerRow];
}

//*****************************************************************************
VuMetalTexture *VuMetalTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	// read texture state
	VuTextureState state;
	state.deserialize(reader);
	
	// read size/levelCount
	int width, height, levelCount;
	reader.readValue(width);
	reader.readValue(height);
	reader.readValue(levelCount);
	
	// handle mip level skip
	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		width = VuMax(width>>skipLevels, 1);
		height = VuMax(height>>skipLevels, 1);
		levelCount -= skipLevels;
	}
	
	// skip Ogles stuff
	VUUINT32 oglFormat;
	VUUINT32 oglType;
	bool oglCompressed;
	reader.readValue(oglFormat);
	reader.readValue(oglType);
	reader.readValue(oglCompressed);
	
	// read texture data
	VuTextureData textureData;
	textureData.load(reader, skipLevels);
	
	// create texture
	VuMetalTexture *pTexture = new VuMetalTexture(width, height, levelCount);
	
	VuTextureData::eFormat format = textureData.getFormat();
	if ( (format == VuTextureData::FORMAT_DXT1) || (format == VuTextureData::FORMAT_DXT5) )
	{
		MTLPixelFormat pixelFormat = MTLPixelFormatRGBA8Unorm;
		
		// create metal texture
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat width:width height:height mipmapped:levelCount > 1];
		descriptor.mipmapLevelCount = levelCount;
		
		pTexture->mMTLTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:descriptor];
		
		// uncompress if DXT
		VuArray<VUBYTE> texData(0);
		texData.resize(width*height*4);
		
		// decompress and load levels
		for ( int level = 0; level < levelCount; level++ )
		{
			int levelWidth = textureData.getLevelWidth(level);
			int levelHeight = textureData.getLevelHeight(level);
			const void *levelData = textureData.getLevelData(level);
			
			if ( format == VuTextureData::FORMAT_DXT1 )
				VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT1);
			else if ( format == VuTextureData::FORMAT_DXT5 )
				VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT5);

			[pTexture->mMTLTexture replaceRegion:MTLRegionMake2D(0, 0, levelWidth, levelHeight) mipmapLevel:level withBytes:&texData.begin() bytesPerRow:levelWidth*4];
		}
	}
	else
	{
		// determine format
		MTLPixelFormat pixelFormat = MTLPixelFormatInvalid;
		bool pvrtc = false;
		
		if ( format == VuTextureData::FORMAT_R )
		{
			pixelFormat = MTLPixelFormatR8Unorm;
		}
		else if ( format == VuTextureData::FORMAT_RGBA )
		{
			pixelFormat = MTLPixelFormatRGBA8Unorm;
		}
		else if ( format == VuTextureData::FORMAT_565 )
		{
			pixelFormat = MTLPixelFormatB5G6R5Unorm;
		}
		else if ( format == VuTextureData::FORMAT_PVRTC_RGB )
		{
			pixelFormat = MTLPixelFormatPVRTC_RGB_4BPP;
			pvrtc = true;
		}
		else if ( format == VuTextureData::FORMAT_PVRTC_RGBA )
		{
			pixelFormat = MTLPixelFormatPVRTC_RGBA_4BPP;
			pvrtc = true;
		}
		else
		{
			VUASSERT(0, "Unsupported texture format.");
		}
		
		// create metal texture
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat width:width height:height mipmapped:levelCount > 1];
		descriptor.mipmapLevelCount = levelCount;
		
		pTexture->mMTLTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:descriptor];
		
		// load levels
		for ( int level = 0; level < levelCount; level++ )
		{
			int levelWidth = textureData.getLevelWidth(level);
			int levelHeight = textureData.getLevelHeight(level);
			const void *levelData = textureData.getLevelData(level);
			int levelPitch = pvrtc ? 0 : textureData.getLevelPitch(level);
			
			[pTexture->mMTLTexture replaceRegion:MTLRegionMake2D(0, 0, levelWidth, levelHeight) mipmapLevel:level withBytes:levelData bytesPerRow:levelPitch];
		}
	}
	
	pTexture->mpSamplerState = VuMetalSamplerState::create(state);
	
	return pTexture;
}

//*****************************************************************************
VuMetalTexture *VuMetalTexture::create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;
	int levelCount = createMipMaps ? VuHighBit(VuMax(width, height)) + 1 : 1;

	// when mipmapping, only support power of two textures
	VUASSERT(levelCount == 1 || (VuBitCount(width) == 1 && VuBitCount(height) == 1), "Only support power-of-2 textures when mip mapping.");
	
	// create texture
	VuMetalTexture *pTexture = new VuMetalTexture(width, height, levelCount);
	
	// determine format
	MTLPixelFormat pixelFormat = MTLPixelFormatInvalid;
	
	if ( format == VUGFX_FORMAT_LIN_R8 )
		pixelFormat = MTLPixelFormatR8Unorm;
	else if ( format == VUGFX_FORMAT_A8R8G8B8 )
		pixelFormat = MTLPixelFormatRGBA8Unorm;
	else if ( format == VUGFX_FORMAT_LIN_L8A8 )
		pixelFormat = MTLPixelFormatRG8Unorm;
	else
		VUASSERT(0, "Unsupported texture format.");

	// create metal texture
	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat width:width height:height mipmapped:levelCount > 1];
	descriptor.mipmapLevelCount = levelCount;
	
	pTexture->mMTLTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:descriptor];
	pTexture->mpSamplerState = VuMetalSamplerState::create(state);
	
	return pTexture;
}
