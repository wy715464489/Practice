//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Quad Particle
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxParticle.h"


class VuPfxQuadParticle : public VuPfxParticle
{
public:
	VuPfxQuadParticle() : mRotation(0), mAngularVelocity(0), mWorldScaleZ(1), mDirStretch(0), mTileOffsetU(0), mTileOffsetV(0) {}

	float		mRotation;
	float		mAngularVelocity;
	float		mWorldScaleZ;
	float		mDirStretch;
	float		mTileOffsetU;
	float		mTileOffsetV;
};

class VuPfxOrbitQuadParticle : public VuPfxQuadParticle
{
public:
	VuPfxOrbitQuadParticle() : mOrbitalPosition(0) {}

	float		mOrbitalPosition;
};
