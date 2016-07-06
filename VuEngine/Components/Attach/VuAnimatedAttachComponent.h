//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AnimatedAttachComponent class
// 
//*****************************************************************************

#pragma once

#include "VuAttachComponent.h"
#include "VuEngine/Containers/VuObjectArray.h"

class VuAnimatedModelInstance;


class VuAnimatedAttachComponent : public VuAttachComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(AnimatedAttach)
	DECLARE_RTTI

public:
	VuAnimatedAttachComponent(VuEntity *pOwner, const VuAnimatedModelInstance *pAnimatedModelInstance);
	~VuAnimatedAttachComponent();

	virtual bool	attach(VuMotionComponent *pMotionComponent, const VuMatrix &transform, const char *node);
	virtual void	detach(VuMotionComponent *pMotionComponent);
	virtual void	update(const VuMatrix &transform, const VuVector3 &linVel, const VuVector3 &angVel);

private:
	struct VuAttachment
	{
		VuMatrix			mTransform;
		VuMotionComponent	*mpMotionComponent;
		int					mBoneIndex;
	};
	typedef VuObjectArray<VuAttachment> Attachments;

	const VuAnimatedModelInstance	*mpAnimatedModelInstance;
	Attachments						mAttachments;
};
