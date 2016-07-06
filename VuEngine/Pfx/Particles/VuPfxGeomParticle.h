//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Geom Particle
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxParticle.h"


class VuPfxGeomParticle : public VuPfxParticle
{
public:
	VuPfxGeomParticle() : mRotation(0,0,0), mAngularVelocity(0,0,0) {}

	VuVector3	mRotation;
	VuVector3	mAngularVelocity;
};
