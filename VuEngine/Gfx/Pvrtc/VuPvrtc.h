//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PVRTC util
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


namespace VuPvrtc
{

	void compressImage(const VUUINT8 *rgba, int width, int height, VuArray<VUBYTE> &output, bool createMipMaps, bool alphaOn, bool assumeTiles);

} // namespace VuDxt
