//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DXT util
// 
//*****************************************************************************

#pragma once


namespace VuDxt
{

	enum eType { DXT1, DXT3, DXT5 };
	enum eOptions
	{
		ITERATIVE_CLUSTER_FIT    = 1 << 0, // Use a very slow but very high quality colour compressor.
		COLOUR_CLUSTER_FIT       = 1 << 1, // Use a slow but high quality colour compressor (the default).
		COLOUR_RANGE_FIT         = 1 << 2, // Use a fast but low quality colour compressor.
		COLOUR_METRIC_PERCEPTUAL = 1 << 3, // Use a perceptual metric for colour error (the default).
		COLOUR_METRIC_UNIFORM    = 1 << 4, // Use a uniform metric for colour error.
		WEIGHT_COLOUR_BY_ALPHA   = 1 << 5, // Weight the colour by alpha during cluster fit (disabled by default).
	};

	int getStorageRequirements(int width, int height, eType type);
	void compressImage(const VUUINT8 *rgba, int width, int height, void *blocks, eType type, VUUINT32 flags = 0);
	void decompressImage(VUUINT8 *rgba, int width, int height, const void *blocks, eType type, VUUINT32 flags = 0);

} // namespace VuDxt
