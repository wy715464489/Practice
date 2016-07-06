//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Math functions
// 
//*****************************************************************************

#include "VuMath.h"

//*****************************************************************************
float VuAngClamp(float ang)
{
	// get diff into -PI -> +PI range
	ang += VU_PI;
	ang /= 2.0f*VU_PI;
	ang -= VuFloor(ang);
	ang *= 2.0f*VU_PI;
	ang -= VU_PI;

	return ang;
}

//*****************************************************************************
float VuAngDiff(float ang0, float ang1)
{
	return VuAngClamp(ang1 - ang0);
}

//*****************************************************************************
float VuAngLerp(float ang0, float ang1, float factor)
{
	float diff = VuAngDiff(ang0, ang1);

	return ang0 + factor*diff;
}
