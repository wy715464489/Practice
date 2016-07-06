//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Spring Constraint Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Math/VuVector3.h"


class VuPfxSpringConstraint : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxSpringConstraint();

	float		mSpringCoeff;
	float		mDampingCoeff;
	float		mStartDelay;
	VuVector3	mTarget;
};

class VuPfxSpringConstraintInstance : public VuPfxProcessInstance
{
public:
	VuPfxSpringConstraintInstance() : mTargetOffset(0,0,0) {}

	virtual void	tick(float fdt, bool ui);

	VuVector3	mTargetOffset;
};
