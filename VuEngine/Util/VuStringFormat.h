//*****************************************************************************
//
//  Copyright (c) 2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String format utility.
// 
//*****************************************************************************

#pragma once

class VuStringFormat
{
public:
	VuStringFormat();

	operator int() const;

	enum eAlign { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_TOP, ALIGN_BOTTOM, ALIGN_CENTER, ALIGN_BASELINE, ALIGN_RIGHT_EA_LEFT };

	int			mAlignH;	// ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_RIGHT_EA_LEFT
	int			mAlignV;	// ALIGN_TOP, ALIGN_CENTER, ALIGN_BOTTOM, ALIGN_BASELINE
	bool		mClip;
	bool		mWordbreak;
	bool		mShrinkToFit;
};
