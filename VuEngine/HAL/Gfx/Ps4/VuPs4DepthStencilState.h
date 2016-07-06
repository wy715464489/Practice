//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"

using namespace sce;


class VuPs4DepthStencilState : public VuDepthStencilState
{
public:
	VuPs4DepthStencilState(const VuDepthStencilStateParams &params);
	~VuPs4DepthStencilState();

	static VuPs4DepthStencilState	*create(const VuDepthStencilStateParams &params);

	VUUINT32					mHash;
	Gnm::DepthStencilControl	mGnmDSC;
};
