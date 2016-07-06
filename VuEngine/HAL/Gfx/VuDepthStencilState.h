//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  depth-stencil state interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuGfxTypes.h"


struct VuDepthStencilStateParams
{
	VuDepthStencilStateParams() { reset(); }

	void			reset() { mDepthCompFunc = VUGFX_COMP_LESS; mDepthWriteEnabled = true; }
	VUUINT32		calcHash() const { return VUUINT32(mDepthCompFunc) | (VUUINT32(mDepthWriteEnabled) << 4); }

	VuGfxCompFunc	mDepthCompFunc;
	bool			mDepthWriteEnabled;
};

class VuDepthStencilState : public VuRefObj
{
public:
	VuDepthStencilState(const VuDepthStencilStateParams &params) : mParams(params) {}

	VuDepthStencilStateParams	mParams;
};
