//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OffsetAttachComponent class
// 
//*****************************************************************************

#pragma once

#include "VuAttachComponent.h"
#include "VuEngine/Containers/VuObjectArray.h"


class VuOffsetAttachComponent : public VuAttachComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(OffsetAttach)
	DECLARE_RTTI

public:
	VuOffsetAttachComponent(VuEntity *pOwner);
	~VuOffsetAttachComponent();

	virtual bool	attach(VuMotionComponent *pMotionComponent, const VuMatrix &transform, const char *node);
	virtual void	detach(VuMotionComponent *pMotionComponent);
	virtual void	update(const VuMatrix &transform, const VuVector3 &linVel, const VuVector3 &angVel);

private:
	struct VuAttachment
	{
		VuMatrix			mTransform;
		VuMotionComponent	*mpMotionComponent;
	};
	typedef VuObjectArray<VuAttachment> Attachments;

	Attachments		mAttachments;
};
