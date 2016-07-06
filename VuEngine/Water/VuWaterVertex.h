//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water vertex definitions
// 
//*****************************************************************************

#pragma once


#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuPackedVector.h"


//*****************************************************************************
// Vertex declarations
//*****************************************************************************
class VuWaterPhysicsVertex
{
public:
	VuVector3	mPosition;	// input(xy)
	VuVector3	mDxyzDt;	// output
	float		mHeight;	// output
};

class VuWaterRenderVertex
{
public:
	VuPackedVector3	mPosition;	// input(xyz)/output(z)
	VuPackedVector2	mDzDxy;		// output
	float			mFoam;		// output
};

