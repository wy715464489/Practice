//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ShadowVolume class
// 
//*****************************************************************************

#pragma once

#include "VuShadowClip.h"
#include "VuEngine/Math/VuMatrix.h"

class VuShadowVolume
{
public:
	bool isSphereVisible(const VuVector3 &vPosWorld, float fRadius) const { return mShadowClip.isSphereVisible(vPosWorld, fRadius); }

	VuMatrix		mCropMatrix;
	VuShadowClip	mShadowClip;
};
