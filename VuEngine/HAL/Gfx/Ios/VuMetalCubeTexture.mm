//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the cube texture interface class.
//
//*****************************************************************************

#include "VuMetalCubeTexture.h"
#include "VuMetalSamplerState.h"
#include "VuMetalGfx.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


static int sFaceTextureTargets[6] =
{
	0,
	1,
	3,
	2,
	4,
	5,
};


//*****************************************************************************
VuMetalCubeTexture::VuMetalCubeTexture(int edgeLength, int levelCount, const VuTextureState &state):
	VuCubeTexture(edgeLength, levelCount)
{
}

//*****************************************************************************
VuMetalCubeTexture::~VuMetalCubeTexture()
{
	mpSamplerState->removeRef();
}

//*****************************************************************************
VuMetalCubeTexture *VuMetalCubeTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	// read texture state
	VuTextureState state;
	state.deserialize(reader);
	
	// read size/levelCount
	int edgeLength, levelCount;
	reader.readValue(edgeLength);
	reader.readValue(levelCount);
	
	// handle mip level skip
	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		edgeLength = VuMax(edgeLength>>skipLevels, 1);
		levelCount -= skipLevels;
	}
	
	// skip Ogles stuff
	VUUINT32 oglFormat;
	VUUINT32 oglType;
	bool oglCompressed;
	reader.readValue(oglFormat);
	reader.readValue(oglType);
	reader.readValue(oglCompressed);
	
	// load texture data
	VuTextureData textureData[6];
	for ( int iFace = 0; iFace < 6; iFace++ )
		textureData[iFace].load(reader, skipLevels);
	
	VuMetalCubeTexture *pCubeTexture = new VuMetalCubeTexture(edgeLength, levelCount, state);
	
	VuTextureData::eFormat format = textureData[0].getFormat();
	if ( (format == VuTextureData::FORMAT_DXT1) || (format == VuTextureData::FORMAT_DXT5) )
	{
		MTLPixelFormat pixelFormat = MTLPixelFormatRGBA8Unorm;

		// create metal cube texture
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixelFormat size:edgeLength mipmapped:levelCount > 1];
		descriptor.mipmapLevelCount = levelCount;
		
		pCubeTexture->mMTLCubeTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:descriptor];

		// uncompress if DXT
		VuArray<VUBYTE> texData(0);
		texData.resize(edgeLength*edgeLength*4);
		
		for ( int iFace = 0; iFace < 6; iFace++ )
		{
			VuTextureData *pTextureData = &textureData[iFace];

			for ( int level = 0; level < levelCount; level++ )
			{
				int levelWidth = pTextureData->getLevelWidth(level);
				int levelHeight = pTextureData->getLevelHeight(level);
				const void *levelData = pTextureData->getLevelData(level);
				
				if ( format == VuTextureData::FORMAT_DXT1 )
					VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT1);
				else if ( format == VuTextureData::FORMAT_DXT5 )
					VuDxt::decompressImage(&texData.begin(), levelWidth, levelHeight, levelData, VuDxt::DXT5);

				VUASSERT(levelWidth == levelHeight, "Cube Texture level width != height!");
				int levelEdgeLength = levelWidth;
				
				int slice = sFaceTextureTargets[iFace];
				[pCubeTexture->mMTLCubeTexture replaceRegion:MTLRegionMake2D(0, 0, levelEdgeLength, levelEdgeLength) mipmapLevel:level slice:slice withBytes:&texData.begin() bytesPerRow:levelWidth*4 bytesPerImage:0];
			}
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

		// create metal cube texture
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixelFormat size:edgeLength mipmapped:levelCount > 1];
		descriptor.mipmapLevelCount = levelCount;
		
		pCubeTexture->mMTLCubeTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:descriptor];

		for ( int iFace = 0; iFace < 6; iFace++ )
		{
			VuTextureData *pTextureData = &textureData[iFace];
			for ( int level = 0; level < levelCount; level++ )
			{
				int levelWidth = pTextureData->getLevelWidth(level);
				int levelHeight = pTextureData->getLevelHeight(level);
				const void *levelData = pTextureData->getLevelData(level);
				int levelPitch = pvrtc ? 0 : pTextureData->getLevelPitch(level);
				
				VUASSERT(levelWidth == levelHeight, "Cube Texture level width != height!");
				int levelEdgeLength = levelWidth;
				
				int slice = sFaceTextureTargets[iFace];
				[pCubeTexture->mMTLCubeTexture replaceRegion:MTLRegionMake2D(0, 0, levelEdgeLength, levelEdgeLength) mipmapLevel:level slice:slice withBytes:levelData bytesPerRow:levelPitch bytesPerImage:0];
			}
		}
	}
	
	pCubeTexture->mpSamplerState = VuMetalSamplerState::create(state);

	return pCubeTexture;
}
