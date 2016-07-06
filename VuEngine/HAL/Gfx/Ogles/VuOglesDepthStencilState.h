//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"
#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"


class VuOglesDepthStencilState : public VuDepthStencilState
{
public:
	VuOglesDepthStencilState(const VuDepthStencilStateParams &params);
	~VuOglesDepthStencilState();

	static VuOglesDepthStencilState	*create(const VuDepthStencilStateParams &params);

	VUUINT32	mHash;
	GLenum		mOglesDepthFunc;
};
