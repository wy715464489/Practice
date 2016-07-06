//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String format utility.
// 
//*****************************************************************************

#include "VuStringFormat.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/DB/VuStringDB.h"


//*****************************************************************************
VuStringFormat::VuStringFormat():
	mAlignH(ALIGN_LEFT), mAlignV(ALIGN_TOP), mClip(false), mWordbreak(false), mShrinkToFit(false)
{
}

//*****************************************************************************
VuStringFormat::operator int() const
{
	int format = 0;

	if ( mAlignH == ALIGN_CENTER )		format |= VUGFX_TEXT_DRAW_HCENTER;
	else if ( mAlignH == ALIGN_RIGHT )	format |= VUGFX_TEXT_DRAW_RIGHT;
	else if ( mAlignH == ALIGN_RIGHT_EA_LEFT )
	{
		if ( !(VuStringDB::IF() && VuStringDB::IF()->isCurrentLanguageEastAsian()) )
			format |= VUGFX_TEXT_DRAW_RIGHT;
	}

	if ( mAlignV == ALIGN_CENTER )			format |= VUGFX_TEXT_DRAW_VCENTER;
	else if ( mAlignV == ALIGN_BOTTOM )		format |= VUGFX_TEXT_DRAW_BOTTOM;
	else if ( mAlignV == ALIGN_BASELINE )	format |= VUGFX_TEXT_DRAW_BASELINE;

	if ( mClip )		format |= VUGFX_TEXT_DRAW_CLIP;
	if ( mWordbreak )	format |= VUGFX_TEXT_DRAW_WORDBREAK;

	return format;
}
