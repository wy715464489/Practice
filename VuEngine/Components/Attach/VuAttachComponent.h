//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AttachComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuMatrix.h"

class VuMotionComponent;


class VuAttachComponent : public VuComponent
{
	DECLARE_RTTI

public:
	VuAttachComponent(VuEntity *pOwner) : VuComponent(pOwner) {}

	virtual bool	attach(VuMotionComponent *pMotionComponent, const VuMatrix &transform, const char *node = "") = 0;
	virtual void	detach(VuMotionComponent *pMotionComponent) = 0;
	virtual void	update(const VuMatrix &transform, const VuVector3 &linVel = VuVector3(0,0,0), const VuVector3 &angVel = VuVector3(0,0,0)) = 0;
};
