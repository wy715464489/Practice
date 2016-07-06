//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Recursive Particle
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxParticle.h"


class VuPfxRecursiveParticle : public VuPfxParticle
{
public:
	VuPfxRecursiveParticle() : mRotation(0,0,0), mAngularVelocity(0,0,0), mpChildPfx(VUNULL) {}

	VuVector3			mRotation;
	VuVector3			mAngularVelocity;
	VuPfxSystemInstance	*mpChildPfx;
};
