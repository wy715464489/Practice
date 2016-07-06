//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Boolean entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuBooleanEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuBooleanEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			SetTrue(const VuParams &params)		{ change(true); return VuRetVal(); }
	VuRetVal			SetFalse(const VuParams &params)	{ change(false); return VuRetVal(); }
	VuRetVal			Toggle(const VuParams &params)		{ change(!mbValue); return VuRetVal(); }
	VuRetVal			Get(const VuParams &params)			{ return VuRetVal(mbValue); }

	void				change(bool bValue);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mbValue;
};

IMPLEMENT_RTTI(VuBooleanEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuBooleanEntity);


//*****************************************************************************
VuBooleanEntity::VuBooleanEntity():
	mbValue(false)
{
	addProperty(new VuBoolProperty("Value", mbValue));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuBooleanEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Bool));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuBooleanEntity, SetTrue, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuBooleanEntity, SetFalse, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuBooleanEntity, Toggle, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuBooleanEntity, Get, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnChanged, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnChangedTrue, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnChangedFalse, VuRetVal::Void, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuBooleanEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	change(accessor.getBool());

	return VuRetVal();
}

//*****************************************************************************
void VuBooleanEntity::change(bool bValue)
{
	if ( bValue != mbValue )
	{
		mbValue = bValue;

		mpScriptComponent->getPlug("OnChanged")->execute();
		if ( bValue )
			mpScriptComponent->getPlug("OnChangedTrue")->execute();
		else
			mpScriptComponent->getPlug("OnChangedFalse")->execute();
	}
}
