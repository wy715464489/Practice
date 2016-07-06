//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ETC util
// 
//*****************************************************************************

#pragma once


namespace VuEtc
{

	enum eQuality { QUALITY_LOW, QUALITY_MEDIUM, QUALITY_HIGH, QUALITY_COUNT };

	struct VuPackParams
	{
		VuPackParams() : mQuality(QUALITY_LOW), mDithering(false) {}
		eQuality	mQuality;
		bool		mDithering;
	};

	int getStorageRequirements(int width, int height);
	void compressEtc1(const VUUINT8 *rgba, int width, int height, void *blocks, const VuPackParams &params);

} // namespace VuEtc
