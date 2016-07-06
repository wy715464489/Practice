//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Trail Particle
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxParticle.h"


class VuPfxTrailParticle : public VuPfxParticle
{
public:
	VuPfxTrailParticle() : mTexCoord(0) {}

	VuVector3	mAxis;
	float		mTexCoord;
};
