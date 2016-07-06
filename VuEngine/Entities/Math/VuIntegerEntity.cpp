//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Integer entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuIntegerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuIntegerEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			Get(const VuParams &params)	{ return VuRetVal(mValue); }

	void				change(int value);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mValue;
};

IMPLEMENT_RTTI(VuIntegerEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuIntegerEntity);


//*****************************************************************************
VuIntegerEntity::VuIntegerEntity():
	mValue(0)
{
	addProperty(new VuIntProperty("Value", mValue));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuIntegerEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Int));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuIntegerEntity, Get, VuRetVal::Int, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnChanged, VuRetVal::Void, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuIntegerEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	change(accessor.getInt());
	
	return VuRetVal();
}

//*****************************************************************************
void VuIntegerEntity::change(int value)
{
	if ( value != mValue )
	{
		mValue = value;

		mpScriptComponent->getPlug("OnChanged")->execute();
	}
}
