//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Group Entity.
// 
//*****************************************************************************

#include "VuGroupEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


IMPLEMENT_RTTI(VuGroupEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuGroupEntity);


//*****************************************************************************
VuGroupEntity::VuGroupEntity():
	VuEntity(CAN_HAVE_CHILDREN),
	mCollapsed(false)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	// properties
	addProperty(new VuBoolProperty("Collapsed", mCollapsed));
}
