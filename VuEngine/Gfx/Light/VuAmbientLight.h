//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AmbientLight class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Util/VuColor.h"


class VuAmbientLight
{
public:
	VuAmbientLight() { reset(); }

	void reset()
	{
		mFoliageColor.set(128,128,128);
		mColor.set(51,51,51);
	}

	VuColor		mColor;
	VuColor		mFoliageColor;
};