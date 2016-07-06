//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  JPEG
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


namespace VuJpeg
{
	bool	decompress(const VuArray<VUBYTE> &jpeg, VuArray<VUBYTE> &rgb, int &width, int &height);
	bool	compress(VuArray<VUBYTE> &jpeg, const VuArray<VUBYTE> &rgb, int width, int height, int quality); // quality 0-100
}
