//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  A scratch pad rovides some raw memory which can be used for temporary
//  storage of calculations, data, and other work in progress.
//
//  Thread-safe and lock-free.
// 
//*****************************************************************************

#pragma once


namespace VuScratchPad
{
	enum { SIZE = 256*1024 };
	enum { SIMULATION, GRAPHICS, WATER, COUNT };

	void *get(int which = SIMULATION);
};