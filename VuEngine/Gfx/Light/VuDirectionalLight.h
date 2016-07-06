//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DirectionalLight class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Util/VuColor.h"


class VuDirectionalLight
{
public:
	VuDirectionalLight() { reset(); }

	void reset()
	{
		mPosition.set(0,0,500);
		mDirection.set(0,0,-1);
		mFrontColor.set(204,204,204);
		mBackColor.set(64,64,64);
		mSpecularColor.set(255,255,255);
		mFoliageColor.set(192,192,192);
	}

	VuVector3	mPosition;
	VuVector3	mDirection;
	VuColor		mFrontColor;
	VuColor		mBackColor;
	VuColor		mSpecularColor;
	VuColor		mFoliageColor;
};