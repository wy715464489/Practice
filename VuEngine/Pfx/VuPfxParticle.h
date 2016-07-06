//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Particle
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Containers/VuList.h"


class VuPfxParticle : public VuListElement<VuPfxParticle>
{
public:
	VuPfxParticle() : mPosition(0,0,0), mLinearVelocity(0,0,0), mColor(1,1,1,1), mScale(1), mAge(0), mLifespan(0) {}

	VuVector3	mPosition;
	VuVector3	mLinearVelocity;
	VuVector4	mColor;
	float		mScale;
	float		mAge;
	float		mLifespan;
};
