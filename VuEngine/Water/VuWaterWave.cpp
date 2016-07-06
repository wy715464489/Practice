//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterWave.h"
#include "VuWater.h"


IMPLEMENT_RTTI_BASE(VuWaterWave);


//*****************************************************************************
VuWaterWave::VuWaterWave(VUUINT32 flags):
	mBoundingAabb(VuVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX), VuVector3(FLT_MAX, FLT_MAX, FLT_MAX)),
	mBoundingDiskCenter(0,0),
	mBoundingDiskRadius(FLT_MAX),
	mpWaterTreeNode(VUNULL),
	mpNextWaveInNode(VUNULL),
	mFlags(flags),
	mTimeFactor(1.0f)
{
}

//*****************************************************************************
VuWaterWave::~VuWaterWave()
{
	VuWater::IF()->removeWave(this);
}
