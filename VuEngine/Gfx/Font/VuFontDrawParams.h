//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuFontDrawParams class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Math/VuRect.h"

#define FONT_DRAW_SCALE_Y (720.0f)

class VuFontDrawParams
{
public:
	VuFontDrawParams() :
		mFlags(0), mSize(16), mWeight(100), mSoftness(5), mColor(255,255,255),
		mOutlineWeight(0), mOutlineSoftness(5), mOutlineColor(0,0,0),
		mSlant(0), mTabSize(8), mStretch(1.0f), mClip(false)
	{}

	enum eFlags
	{
		FORCE_UPPER_CASE = 1<<0,
		FORCE_LOWER_CASE = 1<<1,
	};

	int		mFlags;
	float	mSize;				// 720p pixels (FONT_DRAW_SCALE_Y)
	float	mWeight;			// % of natural weight
	float	mSoftness;			// % of size
	VuColor	mColor;				
	float	mOutlineWeight;		// % of size
	float	mOutlineSoftness;	// % of size
	VuColor	mOutlineColor;
	float	mSlant;				// x/y slope (must be positive)
	int		mTabSize;			// number of spaces
	float	mStretch;			// scaling in x-direction

	bool	mClip;
	VuRect	mClipRect;			// 0-1
};
