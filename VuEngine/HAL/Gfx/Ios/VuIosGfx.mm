//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios OpenGL ES interface class for Gfx.
//
//*****************************************************************************

#include "VuIosOglesGfx.h"
#include "VuMetalGfx.h"


VuGfx *VuGfx::mpInterface = VUNULL;
VuGfx *CreateVuGfxInterface()
{
	VuGfx *pIF = VUNULL;
	
	if ( VuMetalGfx::checkForMetal() )
		pIF = new VuMetalGfx;
	else
		pIF = new VuIosOglesGfx;
	
	VuGfx::mpInterface = pIF;
	
	return pIF;
}
