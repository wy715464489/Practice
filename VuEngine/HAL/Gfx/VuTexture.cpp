//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Texture interface class.
// 
//*****************************************************************************

#include "VuTexture.h"
#include "VuTextureData.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"

#if defined VUWIN32 && !VU_DISABLE_BAKING
	#include <d3d11.h>


	/*#include <texture_tool.h>
	#include <texture_tool/raw/surfacecount.h>
	#pragma comment(lib, "libSceTextureTool.lib")*/
#endif

// ogles texture baking util
#if defined VUWIN32 && !VU_DISABLE_BAKING
struct VuOglesParams
{
	VuOglesParams():
		mCompressed(false), mGlFormat(-1), mGlType(-1), mDataFormat(VuTextureData::FORMAT_INVALID), mNormalMap(false)
	{}
	VuOglesParams(bool compressed, VUUINT32 glFormat, VUUINT32 glType, VuTextureData::eFormat dataFormat):
		mCompressed(compressed), mGlFormat(glFormat), mGlType(glType), mDataFormat(dataFormat), mNormalMap(false)
	{}
	bool					mCompressed;
	VUUINT32				mGlFormat;
	VUUINT32				mGlType;
	VuTextureData::eFormat	mDataFormat;
	bool					mNormalMap;
};
#endif


IMPLEMENT_RTTI_BASE(VuBaseTexture);

IMPLEMENT_RTTI(VuTexture, VuBaseTexture);
IMPLEMENT_RTTI(VuCubeTexture, VuBaseTexture);


//*****************************************************************************
void VuTextureState::serialize(VuBinaryDataWriter &writer) const
{
	writer.writeValue(mAddressU);
	writer.writeValue(mAddressV);
	writer.writeValue(mMagFilter);
	writer.writeValue(mMinFilter);
	writer.writeValue(mMipFilter);
}

//*****************************************************************************
void VuTextureState::deserialize(VuBinaryDataReader &reader)
{
	reader.readValue(mAddressU);
	reader.readValue(mAddressV);
	reader.readValue(mMagFilter);
	reader.readValue(mMinFilter);
	reader.readValue(mMipFilter);
}

//*****************************************************************************
VuTexture::VuTexture(int width, int height, int levelCount):
	mWidth(width),
	mHeight(height),
	mLevelCount(levelCount)
{
}

//*****************************************************************************
VuCubeTexture::VuCubeTexture(int edgeLength, int levelCount):
	mEdgeLength(edgeLength),
	mLevelCount(levelCount)
{
}

//*****************************************************************************
bool VuTexture::bake(const std::string &platform, const std::string &strFileName, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
#if defined VUWIN32 && !VU_DISABLE_BAKING // texture baking is only supported on Win32 for now

	// load image
	VuTgaLoader tgaLoader;
	if ( tgaLoader.load(strFileName) != VuTgaLoader::OK )
		return false;

	if ( platform == "Win32" || platform == "Windows" || platform == "Xb1" )
	{
		// flip image
		tgaLoader.flipImg();

		return bakeD3d11(tgaLoader, type, compression, state, writer);
	}
	else if (platform == "Android" || platform == "Ios" || platform == "BB10")
	{
		return bakeOgles(platform, tgaLoader, type, compression, state, writer);
	}
	else if ( platform == "Ps4")
	{
		// flip image
		tgaLoader.flipImg();

		//return bakePs4(tgaLoader, type, compression, state, writer);
		return true;
	}
	else
	{
		VUASSERT(0, "VuTexture::bake() unsupported platform");
	}

#endif

	return false;
}

//*****************************************************************************
bool VuCubeTexture::bake(const std::string &platform, const std::string &strFileName, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
#if defined VUWIN32 && !VU_DISABLE_BAKING // texture baking is only supported on Win32 for now

	// load image
	VuTgaLoader tgaLoader;
	if ( tgaLoader.load(strFileName) != VuTgaLoader::OK )
		return false;

	if ( platform == "Win32" || platform == "Windows" || platform == "Xb1" )
	{
		// flip image
		tgaLoader.flipImg();

		return bakeD3d11(tgaLoader, type, compression, state, writer);
	}
	else if (platform == "Android" || platform == "Ios" || platform == "BB10" )
	{
		return bakeOgles(platform, tgaLoader, type, compression, state, writer);
	}
	else if ( platform == "Ps4")
	{
		// flip image
		tgaLoader.flipImg();

		//return bakePs4(tgaLoader, type, compression, state, writer);
		return true;
	}
	else
	{
		VUASSERT(0, "VuCubeTexture::bake() unsupported platform");
	}

#endif

	return false;
}

#if defined VUWIN32 && !VU_DISABLE_BAKING

//*****************************************************************************
bool VuTexture::bakeOgles(const std::string &platform, const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	VuOglesParams params;

	// determine format
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		params = VuOglesParams(false, 0x1907, 0x8363, VuTextureData::FORMAT_565); // GL_RGB, GL_UNSIGNED_SHORT_5_6_5
		params.mNormalMap = true;
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		params = VuOglesParams(false, 0x1909, 0x1401, VuTextureData::FORMAT_R); // GL_LUMINANCE, GL_UNSIGNED_BYTE
	}
	else // VUGFX_TEXTURE_TYPE_DEFAULT
	{
		if ( platform == "Ios" )
		{
			if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_32BIT )
			{
				if ( tgaLoader.getBPP() == 8 )	params = VuOglesParams(false, 0x1909, 0x1401, VuTextureData::FORMAT_R); // GL_LUMINANCE, GL_UNSIGNED_BYTE
				else							params = VuOglesParams(false, 0x1908, 0x1401, VuTextureData::FORMAT_RGBA); // GL_RGBA, GL_UNSIGNED_BYTE
			}
			else if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_S3TC )
			{
				if ( tgaLoader.getBPP() == 32 )	params = VuOglesParams(true, 0x83F3, -1, VuTextureData::FORMAT_DXT5); // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
				else							params = VuOglesParams(true, 0x83F0, -1, VuTextureData::FORMAT_DXT1); // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
			}
			else if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_PVRTC )
			{
				if ( tgaLoader.getBPP() == 32 )	params = VuOglesParams(true, 0x8C02, -1, VuTextureData::FORMAT_PVRTC_RGBA); // COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
				else							params = VuOglesParams(true, 0x8C00, -1, VuTextureData::FORMAT_PVRTC_RGB); // COMPRESSED_RGB_PVRTC_4BPPV1_IMG
			}
			else
			{
				return false;
			}
		}
		else // Android, BB10
		{
			if ( compression.mFormatOGLES == VUGFX_FORMAT_OGLES_32BIT )
			{
				if ( tgaLoader.getBPP() == 8 )	params = VuOglesParams(false, 0x1909, 0x1401, VuTextureData::FORMAT_R); // GL_LUMINANCE, GL_UNSIGNED_BYTE
				else							params = VuOglesParams(false, 0x1908, 0x1401, VuTextureData::FORMAT_RGBA); // GL_RGBA, GL_UNSIGNED_BYTE
			}
			else if ( compression.mFormatOGLES == VUGFX_FORMAT_OGLES_ETC1_DXT5 )
			{
				if ( tgaLoader.getBPP() <= 24 )	params = VuOglesParams(true, 0x8D64, -1, VuTextureData::FORMAT_ETC1); // ETC1_RGB8_OES
				else							params = VuOglesParams(true, 0x83F3, -1, VuTextureData::FORMAT_DXT5); // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
			}
			else
			{
				return false;
			}
		}
	}

	if ( !bakeOgles(tgaLoader, params, compression, state, writer) )
		return false;

	return true;
}

//*****************************************************************************
bool VuTexture::bakeOgles(const VuTgaLoader &tgaLoader, const VuOglesParams &params, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;

	// only power-of-2
	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	if ( createMipMaps && (VuBitCount(width) > 1 || VuBitCount(height) > 1) )
		return VUWARNING("Only support power-of-2 textures when mip mapping.");

	// convert tga to rgba
	VuArray<VUBYTE> rgba;
	if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
		return false;

	// force square pvrtc textures
	if ( params.mDataFormat == VuTextureData::FORMAT_PVRTC_RGB || params.mDataFormat == VuTextureData::FORMAT_PVRTC_RGBA )
	{
		if ( VuBitCount(width) > 1 || VuBitCount(height) > 1 )
			return VUWARNING("Only support power-of-2 textures when using PVRTC.");

		if ( tgaLoader.getWidth() != tgaLoader.getHeight() )
		{
			VuArray<VUBYTE> temp(0);
			VuImageUtil::makeSquare4(&rgba[0], width, height, temp);
			rgba.resize(temp.size());
			memcpy(&rgba[0], &temp[0], temp.size());
		}
	}

	// build texture data
	VuTextureData textureData;
	VuTextureData::VuBuildParams buildParams;
	if ( state.mAddressU == VUGFX_ADDRESS_WRAP && state.mAddressV == VUGFX_ADDRESS_WRAP )
		buildParams.mHints |= VuTextureData::HINT_ASSUME_TILES;
	buildParams.mEtcParams = compression.mEtcParams;
	if ( params.mNormalMap )
		buildParams.mHints |= VuTextureData::HINT_NORMAL_MAP;
	if ( !textureData.build(&rgba[0], width, height, params.mDataFormat, createMipMaps, buildParams) )
		return false;

	int levelCount = textureData.getLevelCount();

	// write
	state.serialize(writer);
	writer.writeValue(width);
	writer.writeValue(height);
	writer.writeValue(levelCount);
	writer.writeValue(params.mGlFormat);
	writer.writeValue(params.mGlType);
	writer.writeValue(params.mCompressed);

	textureData.save(writer);

	return true;
}

//*****************************************************************************
bool VuTexture::bakeD3d11(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;

	// determine format
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // default
	VuTextureData::eFormat dataFormat = VuTextureData::FORMAT_RGBA;
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		dxgiFormat = DXGI_FORMAT_R8G8_SNORM;
		dataFormat = VuTextureData::FORMAT_UV;
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		dxgiFormat = DXGI_FORMAT_R8_UNORM;
		dataFormat = VuTextureData::FORMAT_R;
	}
	else
	{
		if ( compression.mFormatDX == VUGFX_FORMAT_DX_32BIT )
		{
			dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			dataFormat = VuTextureData::FORMAT_RGBA;
		}
		else if ( compression.mFormatDX == VUGFX_FORMAT_DX_S3TC )
		{
			if ( tgaLoader.getBPP() == 32 )
			{
				dxgiFormat = DXGI_FORMAT_BC3_UNORM;
				dataFormat = VuTextureData::FORMAT_DXT5;
			}
			else
			{
				dxgiFormat = DXGI_FORMAT_BC1_UNORM;
				dataFormat = VuTextureData::FORMAT_DXT1;
			}
		}
		else
		{
			return false;
		}
	}

	// build texture data
	VuTextureData textureData;
	VuTextureData::VuBuildParams buildParams;
	if ( state.mAddressU == VUGFX_ADDRESS_WRAP && state.mAddressV == VUGFX_ADDRESS_WRAP )
		buildParams.mHints |= VuTextureData::HINT_ASSUME_TILES;
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
		buildParams.mHints |= VuTextureData::HINT_NORMAL_MAP;
	if ( !textureData.build(tgaLoader, dataFormat, createMipMaps, buildParams) )
		return false;

	int width = textureData.getWidth();
	int height = textureData.getHeight();
	int levelCount = textureData.getLevelCount();

	// write
	state.serialize(writer);
	writer.writeValue(width);
	writer.writeValue(height);
	writer.writeValue(levelCount);
	writer.writeValue(dxgiFormat);

	textureData.save(writer);

	return true;
}

//*****************************************************************************
/*
bool VuTexture::bakePs4(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	// convert tga to rgba
	VuArray<VUBYTE> rgba;
	if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();

	if ( width & 0xf )
	{
		// pad width out to multiple of 16
		int paddedWidth = VuAlign(width, 16);

		VuArray<VUBYTE> rgba_padded;
		rgba_padded.resize(paddedWidth*height*4);
		memset(&rgba_padded[0], 0, rgba_padded.size());

		for ( int y = 0; y < height; y++ )
		{
			void *src = &rgba[y*width*4];
			void *dst = &rgba_padded[y*paddedWidth*4];
			memmove(dst, src, width*4);
		}

		rgba.resize(rgba_padded.size());
		VU_MEMCPY(&rgba[0], rgba.size(), &rgba_padded[0], rgba_padded.size());
	}

	// create texture gen
	sce::TextureTool::Info info;
	info.imageType = sce::TextureTool::k2DImage;
	info.width = width;
	info.height = height;
	info.depth = 1;
	info.planes = 4;
	info.mips = 1;
	sce::TextureTool::GnmTextureGen *textureGen = sce::TextureTool::instantiateImage(info);

	// load image
	size_t size = textureGen->loadFromSurface(&rgba[0], sce::Gnm::kTileModeDisplay_LinearAligned, sce::Gnm::kDataFormatR8G8B8A8Unorm);
	if ( size != rgba.size() )
		return VUWARNING("Size problem with texture!\n");

	// determine format
	sce::Gnm::DataFormat gnmFormat = sce::Gnm::kDataFormatInvalid;
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		gnmFormat = sce::Gnm::kDataFormatR8G8Snorm;
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		gnmFormat = sce::Gnm::kDataFormatR8Unorm;
	}
	else
	{
		if ( compression.mFormatDX == VUGFX_FORMAT_DX_32BIT )
		{
			gnmFormat = sce::Gnm::kDataFormatR8G8B8A8Unorm;
		}
		else if ( compression.mFormatDX == VUGFX_FORMAT_DX_S3TC )
		{
			if ( tgaLoader.getBPP() == 32 )
			{
				gnmFormat = sce::Gnm::kDataFormatBc3Unorm;
			}
			else
			{
				gnmFormat = sce::Gnm::kDataFormatBc1Unorm;
			}
		}
		else
		{
			return false;
		}
	}

	// create mip maps?
	if ( state.mMipFilter != VUGFX_TEXF_NONE )
		sce::TextureTool::ImageHelper::convertToMippedImage(&textureGen, true);

	// write
	state.serialize(writer);

	sce::Gnf::GnfFile *gnfFile = sce::TextureTool::writeImage2GnfInMemory(textureGen, gnmFormat);
	writer.writeData(gnfFile, gnfFile->contents.m_streamSize);

	sce::TextureTool::toolsSystemRelease(gnfFile);
	delete textureGen;

	return true;
}*/

//*****************************************************************************
bool VuCubeTexture::bakeOgles(const std::string &platform, const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	VuOglesParams params;

	// determine format
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		VUASSERT(0, "Cube bump maps not supported yet.");
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		VUASSERT(0, "Cube SDF maps not supported yet.");
	}
	else // VUGFX_TEXTURE_TYPE_DEFAULT
	{
		if ( platform == "Ios" )
		{
			if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_32BIT )
			{
				if ( tgaLoader.getBPP() == 8 )	params = VuOglesParams(false, 0x1909, 0x1401, VuTextureData::FORMAT_R); // GL_LUMINANCE, GL_UNSIGNED_BYTE
				else							params = VuOglesParams(false, 0x1908, 0x1401, VuTextureData::FORMAT_RGBA); // GL_RGBA, GL_UNSIGNED_BYTE
			}
			else if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_S3TC )
			{
				if ( tgaLoader.getBPP() == 32 )	params = VuOglesParams(true, 0x83F3, -1, VuTextureData::FORMAT_DXT5); // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
				else							params = VuOglesParams(true, 0x83F0, -1, VuTextureData::FORMAT_DXT1); // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
			}
			else if ( compression.mFormatIOS == VUGFX_FORMAT_IOS_PVRTC )
			{
				if ( tgaLoader.getBPP() == 32 )	params = VuOglesParams(true, 0x8C02, -1, VuTextureData::FORMAT_PVRTC_RGBA); // COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
				else							params = VuOglesParams(true, 0x8C00, -1, VuTextureData::FORMAT_PVRTC_RGB); // COMPRESSED_RGB_PVRTC_4BPPV1_IMG
			}
			else
			{
				return false;
			}
		}
		else // Android, BB10
		{
			if ( compression.mFormatOGLES == VUGFX_FORMAT_OGLES_32BIT )
			{
				if ( tgaLoader.getBPP() == 8 )	params = VuOglesParams(false, 0x1909, 0x1401, VuTextureData::FORMAT_R); // GL_LUMINANCE, GL_UNSIGNED_BYTE
				else							params = VuOglesParams(false, 0x1908, 0x1401, VuTextureData::FORMAT_RGBA); // GL_RGBA, GL_UNSIGNED_BYTE
			}
			else if ( compression.mFormatOGLES == VUGFX_FORMAT_OGLES_ETC1_DXT5 )
			{
				if ( tgaLoader.getBPP() <= 24 )	params = VuOglesParams(true, 0x8D64, -1, VuTextureData::FORMAT_ETC1); // ETC1_RGB8_OES
				else							params = VuOglesParams(true, 0x83F3, -1, VuTextureData::FORMAT_DXT5); // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
			}
			else
			{
				return false;
			}
		}
	}

	if ( !bakeOgles(tgaLoader, params, compression, state, writer) )
		return false;

	return true;
}

//*****************************************************************************
bool VuCubeTexture::bakeOgles(const VuTgaLoader &tgaLoader, const VuOglesParams &params, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	// convert to rgba and extract each face
	int edgeLength = 0;
	VuArray<VUBYTE> rgbaFaces[6];
	{
		VuArray<VUBYTE> rgba;
		{
			// determine edge length
			edgeLength = VuMin((int)tgaLoader.getHeight(), (int)tgaLoader.getWidth()/6);

			// convert tga to rgba
			if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
				return false;
		}

		// extract each face from rgba image
		int rowSize = edgeLength*4;
		int faceSize = edgeLength*rowSize;
		for ( int iFace = 0; iFace < 6; iFace++ )
		{
			rgbaFaces[iFace].resize(faceSize);
			for ( int y = 0; y < edgeLength; y++ )
				memcpy(&rgbaFaces[iFace][y*rowSize], &rgba[y*rowSize*6 + iFace*rowSize], rowSize);
		}
	}

	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;

	// build texture data for each face
	VuTextureData textureData[6];
	VuTextureData::VuBuildParams buildParams;
	buildParams.mEtcParams = compression.mEtcParams;
	for ( int iFace = 0; iFace < 6; iFace++ )
		if ( !textureData[iFace].build(&rgbaFaces[iFace][0], edgeLength, edgeLength, params.mDataFormat, createMipMaps, buildParams) )
			return false;

	int levelCount = createMipMaps ? VuHighBit(edgeLength) + 1 : 1;

	// write
	state.serialize(writer);
	writer.writeValue(edgeLength);
	writer.writeValue(levelCount);
	writer.writeValue(params.mGlFormat);
	writer.writeValue(params.mGlType);
	writer.writeValue(params.mCompressed);

	for ( int iFace = 0; iFace < 6; iFace++ )
		textureData[iFace].save(writer);

	return true;
}

//*****************************************************************************
bool VuCubeTexture::bakeD3d11(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	// convert to rgba and extract each face
	int edgeLength = 0;
	VuArray<VUBYTE> rgbaFaces[6];
	{
		VuArray<VUBYTE> rgba;
		{
			// determine edge length
			edgeLength = VuMin((int)tgaLoader.getHeight(), (int)tgaLoader.getWidth()/6);

			// convert tga to rgba
			if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
				return false;
		}

		// extract each face from rgba image
		int rowSize = edgeLength*4;
		int faceSize = edgeLength*rowSize;
		for ( int iFace = 0; iFace < 6; iFace++ )
		{
			rgbaFaces[iFace].resize(faceSize);
			for ( int y = 0; y < edgeLength; y++ )
				memcpy(&rgbaFaces[iFace][y*rowSize], &rgba[y*rowSize*6 + iFace*rowSize], rowSize);
		}
	}

	// create mip maps?
	bool createMipMaps = state.mMipFilter != VUGFX_TEXF_NONE;

	// determine format
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // default
	VuTextureData::eFormat dataFormat = VuTextureData::FORMAT_RGBA;
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		VUASSERT(0, "Cube bump maps not supported yet.");
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		VUASSERT(0, "Cube SDF maps not supported yet.");
	}
	else
	{
		if ( compression.mFormatDX == VUGFX_FORMAT_DX_32BIT)
		{
			dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			dataFormat = VuTextureData::FORMAT_RGBA;
		}
		else if ( compression.mFormatDX == VUGFX_FORMAT_DX_S3TC )
		{
			if ( tgaLoader.getBPP() == 32 )
			{
				dxgiFormat = DXGI_FORMAT_BC3_UNORM;
				dataFormat = VuTextureData::FORMAT_DXT5;
			}
			else
			{
				dxgiFormat = DXGI_FORMAT_BC1_UNORM;
				dataFormat = VuTextureData::FORMAT_DXT1;
			}
		}
		else
		{
			return false;
		}
	}

	// build texture data for each face
	VuTextureData textureData[6];
	VuTextureData::VuBuildParams buildParams;
	for ( int iFace = 0; iFace < 6; iFace++ )
		if ( !textureData[iFace].build(&rgbaFaces[iFace][0], edgeLength, edgeLength, dataFormat, createMipMaps, buildParams) )
			return false;

	int levelCount = createMipMaps ? VuHighBit(edgeLength) + 1 : 1;

	// write
	state.serialize(writer);
	writer.writeValue(edgeLength);
	writer.writeValue(levelCount);
	writer.writeValue(dxgiFormat);

	for ( int iFace = 0; iFace < 6; iFace++ )
		textureData[iFace].save(writer);

	return true;
}

//*****************************************************************************
/*
bool VuCubeTexture::bakePs4(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer)
{
	static sce::Gnm::CubemapFace sFaceTextureTargets[6] =
	{
		sce::Gnm::kCubemapFacePositiveX,
		sce::Gnm::kCubemapFaceNegativeX,
		sce::Gnm::kCubemapFacePositiveY,
		sce::Gnm::kCubemapFaceNegativeY,
		sce::Gnm::kCubemapFacePositiveZ,
		sce::Gnm::kCubemapFaceNegativeZ,
	};

	// convert to rgba and extract each face
	int edgeLength = 0;
	VuArray<VUBYTE> rgbaFaces[6];
	{
		VuArray<VUBYTE> rgba;
		{
			// determine edge length
			edgeLength = VuMin((int)tgaLoader.getHeight(), (int)tgaLoader.getWidth()/6);

			// convert tga to rgba
			if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
				return false;
		}

		// extract each face from rgba image
		int rowSize = edgeLength*4;
		int faceSize = edgeLength*rowSize;
		for ( int iFace = 0; iFace < 6; iFace++ )
		{
			rgbaFaces[iFace].resize(faceSize);
			for ( int y = 0; y < edgeLength; y++ )
				memcpy(&rgbaFaces[iFace][y*rowSize], &rgba[y*rowSize*6 + iFace*rowSize], rowSize);
		}
	}

	// create texture gen
	sce::TextureTool::Info info;
	info.imageType = sce::TextureTool::kCubeImage;
	info.width = edgeLength;
	info.height = edgeLength;
	info.depth = 1;
	info.planes = 4;
	info.mips = 1;
	sce::TextureTool::GnmTextureGen *textureGen = sce::TextureTool::instantiateImage(info);

	// load images
	for ( int iFace = 0; iFace < 6; iFace++ )
	{
		sce::TextureTool::GnmTextureGen::ImageIdentifier imageId;
		imageId.m_face = sFaceTextureTargets[iFace];
		imageId.m_mip = 0;
		imageId.m_image = VUNULL;

		sce::TextureTool::Image *image = textureGen->findSubImage(imageId);
		image->loadFromSurface(&rgbaFaces[iFace][0], sce::Gnm::kTileModeDisplay_LinearAligned, sce::Gnm::kDataFormatR8G8B8A8Unorm);
	}

	// create mip maps?
	if ( state.mMipFilter != VUGFX_TEXF_NONE )
		sce::TextureTool::ImageHelper::convertToMippedImage(&textureGen, true);

	// determine format
	sce::Gnm::DataFormat gnmFormat = sce::Gnm::kDataFormatInvalid;
	if ( type == VUGFX_TEXTURE_TYPE_BUMP )
	{
		VUASSERT(0, "Cube bump maps not supported yet.");
	}
	else if ( type == VUGFX_TEXTURE_TYPE_SDF )
	{
		VUASSERT(0, "Cube SDF maps not supported yet.");
	}
	else
	{
		if ( compression.mFormatDX == VUGFX_FORMAT_DX_32BIT )
		{
			gnmFormat = sce::Gnm::kDataFormatR8G8B8A8Unorm;
		}
		else if ( compression.mFormatDX == VUGFX_FORMAT_DX_S3TC )
		{
			if ( tgaLoader.getBPP() == 32 )
			{
				gnmFormat = sce::Gnm::kDataFormatBc3Unorm;
			}
			else
			{
				gnmFormat = sce::Gnm::kDataFormatBc1Unorm;
			}
		}
		else
		{
			return false;
		}
	}

	// write
	state.serialize(writer);

	sce::Gnf::GnfFile *gnfFile = sce::TextureTool::writeImage2GnfInMemory(textureGen, gnmFormat);
	writer.writeData(gnfFile, gnfFile->contents.m_streamSize);

	sce::TextureTool::toolsSystemRelease(gnfFile);
	delete textureGen;

	return true;
}
*/

#endif // VUWIN32
