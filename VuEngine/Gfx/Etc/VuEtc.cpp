//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ETC util
// 
//*****************************************************************************

#include "VuEtc.h"
#include "VuEngine/Libs/rg-etc1/rg_etc1.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuMath.h"


#if defined VUWIN32 && !VU_DISABLE_BAKING

static bool sEtc1Initialized = false;

int VuEtc::getStorageRequirements(int width, int height)
{
	// compute the storage requirements
	int blockcount = ( ( width + 3 )/4 ) * ( ( height + 3 )/4 );
	int blocksize = 8;
	return blockcount*blocksize;	
}

void VuEtc::compressEtc1(const VUUINT8 *rgba, int width, int height, void *blocks, const VuPackParams &params)
{
	// initialize
	if ( !sEtc1Initialized )
	{
		rg_etc1::pack_etc1_block_init();
		sEtc1Initialized = true;
	}

	rg_etc1::etc1_pack_params packParams;
	if ( params.mQuality == QUALITY_HIGH )        packParams.m_quality = rg_etc1::cHighQuality;
	else if ( params.mQuality == QUALITY_MEDIUM ) packParams.m_quality = rg_etc1::cMediumQuality;
	else if ( params.mQuality == QUALITY_LOW )    packParams.m_quality = rg_etc1::cLowQuality;
	packParams.m_dithering = params.mDithering;

	int blockCountX = (width+3)/4;
	int blockCountY = (height+3)/4;
	int blockCount = blockCountX*blockCountY;
	int blockSize = 8;

	#pragma omp parallel for
	for ( int b = 0; b < blockCount; b++ )
	{
		int blockX = b%blockCountX;
		int blockY = b/blockCountX;

		VUBYTE blockRGBA[16*4];
		int offsetX = blockX*4;
		int offsetY = blockY*4;
		int blockWidth = VuMin(4, width - offsetX);
		int blockHeight = VuMin(4, height - offsetY);

		for ( int dsty = 0; dsty < 4; dsty++ )
		{
			for ( int dstx = 0; dstx < 4; dstx++ )
			{
				int srcx = dstx%blockWidth;
				int srcy = dsty%blockHeight;

				blockRGBA[dsty*16 + dstx*4 + 0] = rgba[(offsetY + srcy)*width*4 + (offsetX + srcx)*4 + 0];
				blockRGBA[dsty*16 + dstx*4 + 1] = rgba[(offsetY + srcy)*width*4 + (offsetX + srcx)*4 + 1];
				blockRGBA[dsty*16 + dstx*4 + 2] = rgba[(offsetY + srcy)*width*4 + (offsetX + srcx)*4 + 2];
				blockRGBA[dsty*16 + dstx*4 + 3] = rgba[(offsetY + srcy)*width*4 + (offsetX + srcx)*4 + 3];
			}
		}

		void *block = ((VUBYTE *)blocks) + b*blockSize;

		int test = pack_etc1_block(block, (const unsigned int *)blockRGBA, packParams);
	}
}

#else // VUWIN32

int VuEtc::getStorageRequirements(int width, int height) { return 0; }
void VuEtc::compressEtc1(const VUUINT8 *rgba, int width, int height, void *blocks, const VuPackParams &params) {}

#endif // VUWIN32
