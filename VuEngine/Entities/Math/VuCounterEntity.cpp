//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Counter entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuCounterEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCounterEntity();

private:
	VuRetVal			Inc(const VuParams &params)			{ change( 1, params); return VuRetVal(); }
	VuRetVal			Dec(const VuParams &params)			{ change(-1, params); return VuRetVal(); }
	VuRetVal			GetCount(const VuParams &params)	{ return VuRetVal(mCount); }

	void				change(int value, const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mCount;
	int					mTarget;
};

IMPLEMENT_RTTI(VuCounterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCounterEntity);


//*****************************************************************************
VuCounterEntity::VuCounterEntity():
	mCount(0),
	mTarget(0)
{
	addProperty(new VuIntProperty("Initial Count", mCount));
	addProperty(new VuIntProperty("Target Count", mTarget));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCounterEntity, Inc);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCounterEntity, Dec);
	ADD_SCRIPT_INPUT(mpScriptComponent, VuCounterEntity, GetCount, VuRetVal::Int, VuParamDecl());
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Trigger);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnChanged);
}

//*****************************************************************************
void VuCounterEntity::change(int value, const VuParams &params)
{
	mCount += value;
	if ( mCount == mTarget )
	{
		mpScriptComponent->getPlug("Trigger")->execute(params);
	}
	mpScriptComponent->getPlug("OnChanged")->execute(params);
}
