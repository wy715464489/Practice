//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  InstigatorComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuVector3.h"


class VuInstigatorComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Instigator)
	DECLARE_RTTI

public:
	VuInstigatorComponent(VuEntity *pOwnerEntity);
	~VuInstigatorComponent();

	// parameters
	void			setMask(VUUINT32 mask);
	void			setOffset(const VuVector3 &vOffset)	{ mOffset = vOffset; }
	void			setRadius(float fRadius)			{ mRadius = fRadius; }

	VUUINT32		getMask()		{ return mMask; }
	const VuVector3	&getOffset()	{ return mOffset; }
	float			getRadius()		{ return mRadius; }

	// adding/removing from scene
	void			enable();
	void			disable();

	// snap to current location
	void			snap();

private:
	VUUINT32		mMask;
	VuVector3		mOffset;
	float			mRadius;
	bool			mbRegistered;
};
