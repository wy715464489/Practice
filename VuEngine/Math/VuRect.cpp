//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rect class
// 
//*****************************************************************************

#include <float.h>
#include "VuRect.h"


//*****************************************************************************
bool VuRect::intersects(const VuVector2 &p0, const VuVector2 &p1) const
{
	VuVector2 t0 = p0, t1 = p1;

	// clip left/right
	if ( t0.mX > t1.mX )
		VuSwap<VuVector2>(t0, t1);

	// left
	if ( t0.mX < getLeft() )
	{
		if ( t1.mX < getLeft() )
			return false;
		t0 += (t1 - t0)*(getLeft() - t0.mX)/(t1.mX - t0.mX);
	}

	// right
	if ( t1.mX > getRight() )
	{
		if ( t0.mX > getRight() )
			return false;
		t1 -= (t1 - t0)*(t1.mX - getRight())/(t1.mX - t0.mX);
	}

	// clip top/bottom
	if ( t0.mY > t1.mY )
		VuSwap<VuVector2>(t0, t1);

	// top
	if ( t0.mY < getTop() )
	{
		if ( t1.mY < getTop() )
			return false;
		t0 += (t1 - t0)*(getTop() - t0.mY)/(t1.mY - t0.mY);
	}

	// bottom
	if ( t1.mY > getBottom() )
	{
		if ( t0.mY > getBottom() )
			return false;
		t1 -= (t1 - t0)*(t1.mY - getBottom())/(t1.mY - t0.mY);
	}

	return true;
}
