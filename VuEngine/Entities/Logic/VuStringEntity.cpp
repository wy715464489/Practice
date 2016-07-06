//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"


class VuStringEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuStringEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			Clear(const VuParams &params)	{ change(""); return VuRetVal(); }
	VuRetVal			Get(const VuParams &params)		{ return VuRetVal(mValue.c_str()); }

	void				change(const char *value);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mValue;
};

IMPLEMENT_RTTI(VuStringEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuStringEntity);


//*****************************************************************************
VuStringEntity::VuStringEntity()
{
	addProperty(new VuStringProperty("Value", mValue));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuStringEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::String));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuStringEntity, Clear, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuStringEntity, Get, VuRetVal::String, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnChanged, VuRetVal::Void, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuStringEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	change(accessor.getString());
	
	return VuRetVal();
}

//*****************************************************************************
void VuStringEntity::change(const char *value)
{
	if ( mValue != value )
	{
		mValue = value;

		mpScriptComponent->getPlug("OnChanged")->execute();
	}
}
