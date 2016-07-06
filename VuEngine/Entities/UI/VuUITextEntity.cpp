//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Text class
// 
//*****************************************************************************

#include "VuUITextEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/DB/VuStringDB.h"
#include "VuEngine/Properties/VuBasicProperty.h"

IMPLEMENT_RTTI(VuUITextEntity, VuUITextBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUITextEntity);


//*****************************************************************************
VuUITextEntity::VuUITextEntity()
{
	// properties
	addProperty(new VuStringProperty("String ID", mStringID));

	// scripting
	ADD_SCRIPT_INPUT(mpScriptComponent, VuUITextEntity, SetStringID, VuRetVal::Void, VuParamDecl(1, VuParams::String));
}

//*****************************************************************************
const char *VuUITextEntity::getText()
{
	return VuStringDB::IF()->getString(mStringID).c_str();
}

//*****************************************************************************
VuRetVal VuUITextEntity::SetStringID(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	mStringID = accessor.getString();

	return VuRetVal();
}
