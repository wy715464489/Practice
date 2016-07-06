//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the texture interface class.
//
//*****************************************************************************

#include <gnf.h>
#include "VuPs4Texture.h"
#include "VuPs4Sampler.h"
#include "VuPs4Gfx.h"
#include "VuPs4GfxTypes.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Math/VuMath.h"


//*****************************************************************************
VuPs4Texture::VuPs4Texture(int width, int height, int levelCount, const VuTextureState &state) :
	VuTexture(width, height, levelCount),
	mpGnmMemory(VUNULL)
{
	VuPs4Sampler::createSampler(state, mGnmSampler);
}

//*****************************************************************************
VuPs4Texture::~VuPs4Texture()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmMemory);
}

//*****************************************************************************
void VuPs4Texture::setData(int level, const void *pData, int size)
{
	uint64_t mipOffset, mipSize;
	if ( GpuAddress::computeTextureSurfaceOffsetAndSize(&mipOffset, &mipSize, &mGnmTexture, level, 0) == GpuAddress::kStatusSuccess )
	{
		GpuAddress::TilingParameters tp;
		if ( tp.initFromTexture(&mGnmTexture, level, 0) == GpuAddress::kStatusSuccess )
		{
			uint64_t outSize;
			Gnm::AlignmentType outAlign;
			if ( GpuAddress::computeUntiledSurfaceSize(&outSize, &outAlign, &tp) == GpuAddress::kStatusSuccess )
			{
				VUASSERT(outSize == size, "VuPs4Texture::setData() size issue");
				GpuAddress::tileSurface((VUBYTE *)mpGnmMemory + mipOffset, pData, &tp);
			}
		}
	}
}

//*****************************************************************************
VuPs4Texture *VuPs4Texture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	Gnf::GnfFile *gnfFile = (Gnf::GnfFile *)reader.cur();
	reader.skip(gnfFile->contents.m_streamSize);

	int width = gnfFile->contents.m_textures[0].getWidth();
	int height = gnfFile->contents.m_textures[0].getHeight();
	int levelCount = gnfFile->contents.m_textures[0].getLastMipLevel() + 1;

	// allocate
	Gnm::SizeAlign sizeAlign = Gnf::getTexturePixelsSize(&gnfFile->contents, 0);
	void *dstTexels = VuPs4Gfx::IF()->garlicAllocator().allocate(sizeAlign);

	// copy
	int texelOffset = Gnf::getTexturePixelsByteOffset(&gnfFile->contents, 0);
	void *srcTexels = ((VUBYTE *)gnfFile) + sizeof(Gnf::Header) + gnfFile->header.m_contentsSize + texelOffset;
	memcpy(dstTexels, srcTexels, sizeAlign.m_size);

	VuPs4Texture *pTexture = new VuPs4Texture(width, height, levelCount, state);

	// patch
	pTexture->mpGnmMemory = dstTexels;
	pTexture->mGnmTexture = *Gnf::patchTextures(&gnfFile->contents, 0, 1, &dstTexels);

	return pTexture;
}

//*****************************************************************************
VuPs4Texture *VuPs4Texture::create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	bool createMipMaps = (state.mMipFilter != VUGFX_TEXF_NONE);
	int levelCount = createMipMaps ? VuHighBit(VuMax(width, height)) + 1 : 1;

	Gnm::DataFormat ps4Format = VuPs4GfxTypes::convert(format);

	Gnm::TileMode tileMode;
	GpuAddress::computeSurfaceTileMode(&tileMode, GpuAddress::kSurfaceTypeTextureFlat, ps4Format, 1);

	VuPs4Texture *pTexture = new VuPs4Texture(width, height, levelCount, state);

	Gnm::SizeAlign sizeAlign = pTexture->mGnmTexture.initAs2d(width, height, levelCount, ps4Format,tileMode, Gnm::kNumFragments1);
	pTexture->mpGnmMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(sizeAlign);
	pTexture->mGnmTexture.setBaseAddress(pTexture->mpGnmMemory);

	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
		pTexture->mGnmTexture.setResourceMemoryType(Gnm::kResourceMemoryTypeUC);

	return pTexture;
}