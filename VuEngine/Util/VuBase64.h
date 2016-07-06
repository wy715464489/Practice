//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Base64 library
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


namespace VuBase64
{
	void encode(const VUBYTE *bytes, int len, std::string &str);
	void encode(const VuArray<VUBYTE> &bytes, std::string &str);
	bool decode(const std::string &str, VuArray<VUBYTE> &bytes);
}