//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class to provide some raw memory which can be used for temporary storage
//  of calculations, data, and other work in progress
// 
//*****************************************************************************

#include "VuScratchPad.h"


// static variables
static VUUINT32 sScratchPadMemory[VuScratchPad::COUNT][VuScratchPad::SIZE/sizeof(VUUINT32)];


//*****************************************************************************
void *VuScratchPad::get(int which)
{
	return sScratchPadMemory[which];
}
