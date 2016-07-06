//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio utility functionality.
// 
//*****************************************************************************

#pragma once

class VuVector3;

namespace VuAudioUtil
{
	void playSfx(const char *sfx);
	void playSfx(const char *sfx, const VuVector3 &pos);
}
