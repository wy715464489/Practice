//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Set Value entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"


//*****************************************************************************
// SetBoolean
//*****************************************************************************
class VuSetBooleanEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSetBooleanEntity():
		mValue(false)
	{
		addProperty(new VuBoolProperty("Value", mValue));
		addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
		ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSetBooleanEntity, Trigger);
		ADD_SCRIPT_OUTPUT(mpScriptComponent, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Bool));
	}

private:
	VuRetVal Trigger(const VuParams &params)
	{
		VuParams outParams;
		outParams.addBool(mValue);
		mpScriptComponent->getPlug("Set")->execute(outParams);
		return VuRetVal();
	}

	VuScriptComponent	*mpScriptComponent;
	bool				mValue;
};

IMPLEMENT_RTTI(VuSetBooleanEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSetBooleanEntity);


//*****************************************************************************
// SetInteger
//*****************************************************************************
class VuSetIntegerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSetIntegerEntity():
		mValue(0)
	{
		addProperty(new VuIntProperty("Value", mValue));
		addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
		ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSetIntegerEntity, Trigger);
		ADD_SCRIPT_OUTPUT(mpScriptComponent, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Int));
	}

private:
	VuRetVal Trigger(const VuParams &params)
	{
		VuParams outParams;
		outParams.addInt(mValue);
		mpScriptComponent->getPlug("Set")->execute(outParams);
		return VuRetVal();
	}

	VuScriptComponent	*mpScriptComponent;
	int					mValue;
};

IMPLEMENT_RTTI(VuSetIntegerEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSetIntegerEntity);


//*****************************************************************************
// SetString
//*****************************************************************************
class VuSetStringEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSetStringEntity()
	{
		addProperty(new VuStringProperty("Value", mValue));
		addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
		ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSetStringEntity, Trigger);
		ADD_SCRIPT_OUTPUT(mpScriptComponent, Set, VuRetVal::Void, VuParamDecl(1, VuParams::String));
	}

private:
	VuRetVal Trigger(const VuParams &params)
	{
		VuParams outParams;
		outParams.addString(mValue.c_str());
		mpScriptComponent->getPlug("Set")->execute(outParams);
		return VuRetVal();
	}

	VuScriptComponent	*mpScriptComponent;
	std::string			mValue;
};

IMPLEMENT_RTTI(VuSetStringEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSetStringEntity);
