//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the cube texture interface class.
// 
//*****************************************************************************

#include <gnf.h>
#include "VuPs4CubeTexture.h"
#include "VuPs4Sampler.h"
#include "VuPs4Gfx.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

using namespace sce;


//*****************************************************************************
VuPs4CubeTexture::VuPs4CubeTexture(int edgeLength, int levelCount, const VuTextureState &state) :
	VuCubeTexture(edgeLength, levelCount),
	mpGnmMemory(VUNULL)
{
	VuPs4Sampler::createSampler(state, mGnmSampler);
}

//*****************************************************************************
VuPs4CubeTexture::~VuPs4CubeTexture()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmMemory);
}

//*****************************************************************************
VuPs4CubeTexture *VuPs4CubeTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	Gnf::GnfFile *gnfFile = (Gnf::GnfFile *)reader.cur();
	reader.skip(gnfFile->contents.m_streamSize);

	int edgeLength = gnfFile->contents.m_textures[0].getWidth();
	int levelCount = gnfFile->contents.m_textures[0].getLastMipLevel() + 1;

	// allocate
	Gnm::SizeAlign sizeAlign = Gnf::getTexturePixelsSize(&gnfFile->contents, 0);
	void *dstTexels = VuPs4Gfx::IF()->garlicAllocator().allocate(sizeAlign);

	// copy
	int texelOffset = Gnf::getTexturePixelsByteOffset(&gnfFile->contents, 0);
	void *srcTexels = ((VUBYTE *)gnfFile) + sizeof(Gnf::Header) + gnfFile->header.m_contentsSize + texelOffset;
	memcpy(dstTexels, srcTexels, sizeAlign.m_size);

	VuPs4CubeTexture *pTexture = new VuPs4CubeTexture(edgeLength, levelCount, state);

	// patch
	pTexture->mpGnmMemory = (VUBYTE *)dstTexels;
	pTexture->mGnmTexture = *Gnf::patchTextures(&gnfFile->contents, 0, 1, &dstTexels);

	return pTexture;
}
