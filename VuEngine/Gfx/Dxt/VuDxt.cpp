//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DXT util
// 
//*****************************************************************************

#include "VuDxt.h"
#include "VuEngine/Libs/squish/squish.h"


#if VU_DISABLE_DXT_COMPRESSION

int VuDxt::getStorageRequirements(int width, int height, eType type) { return 0; }
void VuDxt::compressImage(VUUINT8 const *rgba, int width, int height, void *blocks, eType type, VUUINT32 flags) {}
void VuDxt::decompressImage(VUUINT8 *rgba, int width, int height, void const *blocks, eType type, VUUINT32 flags) {}

#else

static int SquishFlags(VuDxt::eType vuType, VUUINT32 vuFlags = 0)
{
	int squishFlags = 0;

	if ( vuType == VuDxt::DXT1 )                     squishFlags |= squish::kDxt1;
	if ( vuType == VuDxt::DXT3 )                     squishFlags |= squish::kDxt3;
	if ( vuType == VuDxt::DXT5 )                     squishFlags |= squish::kDxt5;
	if ( vuFlags & VuDxt::ITERATIVE_CLUSTER_FIT )    squishFlags |= squish::kColourIterativeClusterFit;
	if ( vuFlags & VuDxt::COLOUR_CLUSTER_FIT )       squishFlags |= squish::kColourClusterFit;
	if ( vuFlags & VuDxt::COLOUR_RANGE_FIT )         squishFlags |= squish::kColourRangeFit;
	if ( vuFlags & VuDxt::COLOUR_METRIC_PERCEPTUAL ) squishFlags |= squish::kColourMetricPerceptual;
	if ( vuFlags & VuDxt::COLOUR_METRIC_UNIFORM )    squishFlags |= squish::kColourMetricUniform;
	if ( vuFlags & VuDxt::WEIGHT_COLOUR_BY_ALPHA )   squishFlags |= squish::kWeightColourByAlpha;

	return squishFlags;
}


//*****************************************************************************
int VuDxt::getStorageRequirements(int width, int height, eType type)
{
	return squish::GetStorageRequirements(width, height, SquishFlags(type));
}

//*****************************************************************************
void VuDxt::compressImage(const VUUINT8 *rgba, int width, int height, void *blocks, eType type, VUUINT32 flags)
{
	return squish::CompressImage(rgba, width, height, blocks, SquishFlags(type, flags));
}

//*****************************************************************************
void VuDxt::decompressImage(VUUINT8 *rgba, int width, int height, const void *blocks, eType type, VUUINT32 flags)
{
	return squish::DecompressImage(rgba, width, height, blocks, SquishFlags(type, flags));
}

#endif // VU_DISABLE_DXT_COMPRESSION