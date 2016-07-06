//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Container Entities defined by the engine.
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/VuTransitionComponent.h"


//*****************************************************************************
//
// VuContainerEntity
//
//*****************************************************************************
class VuContainerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuContainerEntity();
};

IMPLEMENT_RTTI(VuContainerEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuContainerEntity);

//*****************************************************************************
VuContainerEntity::VuContainerEntity() : VuEntity(CAN_HAVE_CHILDREN)
{
}


//*****************************************************************************
//
// VuTransitionEntity
//
//*****************************************************************************
class VuTransitionEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuTransitionEntity();

protected:
	// event handlers
	void				OnUITick(const VuParams &params);

	// components
	VuTransitionComponent	*mpTransitionComponent;
};

IMPLEMENT_RTTI(VuTransitionEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTransitionEntity);

//*****************************************************************************
VuTransitionEntity::VuTransitionEntity() : VuEntity(CAN_HAVE_CHILDREN)
{
	// components
	addComponent(mpTransitionComponent = new VuTransitionComponent(this));

	// event handlers
	REG_EVENT_HANDLER(VuTransitionEntity, OnUITick);
}

//*****************************************************************************
void VuTransitionEntity::OnUITick(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float fdt = accessor.getFloat();

	mpTransitionComponent->tick(fdt);
}