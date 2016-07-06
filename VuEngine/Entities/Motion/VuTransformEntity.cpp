//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Transform entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Components/Attach/VuOffsetAttachComponent.h"


class VuTransformEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuTransformEntity();

protected:
	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	VuScriptComponent	*mpScriptComponent;
	VuMotionComponent	*mpMotionComponent;
	VuAttachComponent	*mpAttachComponent;
};


IMPLEMENT_RTTI(VuTransformEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTransformEntity);


//*****************************************************************************
VuTransformEntity::VuTransformEntity()
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));
	addComponent(mpAttachComponent = new VuOffsetAttachComponent(this));
}

//*****************************************************************************
void VuTransformEntity::onMotionUpdate()
{
	mpAttachComponent->update(mpMotionComponent->getWorldTransform(), mpMotionComponent->getWorldLinearVelocity(), mpMotionComponent->getWorldAngularVelocity());
}
