//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Math entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


//*****************************************************************************
// Add a constant to an integer
//*****************************************************************************
class VuAddIntegerConstantEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAddIntegerConstantEntity();

private:
	VuRetVal			Result(const VuParams &params);
	VuRetVal			In(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mConstant;
};

IMPLEMENT_RTTI(VuAddIntegerConstantEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAddIntegerConstantEntity);

//*****************************************************************************
VuAddIntegerConstantEntity::VuAddIntegerConstantEntity():
	mConstant(0)
{
	addProperty(new VuIntProperty("C", mConstant));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A + C", this, &VuAddIntegerConstantEntity::Result, VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptInputPlug("In", this, &VuAddIntegerConstantEntity::In));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("A", VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("Out", VuRetVal::Void, VuParamDecl(1, VuParams::Int)));
}

//*****************************************************************************
VuRetVal VuAddIntegerConstantEntity::Result(const VuParams &params)
{
	int result = mConstant;

	VuRetVal a = mpScriptComponent->getPlug("A")->execute();
	if ( a.getType() == VuRetVal::Int )
		result += a.asInt();

	return VuRetVal(result);
}

//*****************************************************************************
VuRetVal VuAddIntegerConstantEntity::In(const VuParams &params)
{
	VuParams outParams;
	outParams.addInt(Result(params).asInt());
	mpScriptComponent->getPlug("Out")->execute(outParams);
	return VuRetVal();
}


//*****************************************************************************
// Add two integers
//*****************************************************************************
class VuAddIntegersEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAddIntegersEntity();

private:
	VuRetVal			Result(const VuParams &params);
	VuRetVal			In(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuAddIntegersEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAddIntegersEntity);

//*****************************************************************************
VuAddIntegersEntity::VuAddIntegersEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A + B", this, &VuAddIntegersEntity::Result, VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptInputPlug("In", this, &VuAddIntegersEntity::In));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("A", VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("B", VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("Out", VuRetVal::Void, VuParamDecl(1, VuParams::Int)));
}

//*****************************************************************************
VuRetVal VuAddIntegersEntity::Result(const VuParams &params)
{
	int result = 0;

	VuRetVal a = mpScriptComponent->getPlug("A")->execute();
	VuRetVal b = mpScriptComponent->getPlug("B")->execute();

	if ( a.getType() == VuRetVal::Int )	result += a.asInt();
	if ( b.getType() == VuRetVal::Int )	result += b.asInt();

	return VuRetVal(result);
}

//*****************************************************************************
VuRetVal VuAddIntegersEntity::In(const VuParams &params)
{
	VuParams outParams;
	outParams.addInt(Result(params).asInt());
	mpScriptComponent->getPlug("Out")->execute(outParams);
	return VuRetVal();
}


//*****************************************************************************
// Subtract two integers
//*****************************************************************************
class VuSubtractIntegersEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSubtractIntegersEntity();

private:
	VuRetVal			Result(const VuParams &params);
	VuRetVal			In(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuSubtractIntegersEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSubtractIntegersEntity);

//*****************************************************************************
VuSubtractIntegersEntity::VuSubtractIntegersEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A - B", this, &VuSubtractIntegersEntity::Result, VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptInputPlug("In", this, &VuSubtractIntegersEntity::In));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("A", VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("B", VuRetVal::Int));
	mpScriptComponent->addPlug(new VuScriptOutputPlug("Out", VuRetVal::Void, VuParamDecl(1, VuParams::Int)));
}

//*****************************************************************************
VuRetVal VuSubtractIntegersEntity::Result(const VuParams &params)
{
	int result = 0;

	VuRetVal a = mpScriptComponent->getPlug("A")->execute();
	VuRetVal b = mpScriptComponent->getPlug("B")->execute();

	if ( a.getType() == VuRetVal::Int )	result += a.asInt();
	if ( b.getType() == VuRetVal::Int )	result -= b.asInt();

	return VuRetVal(result);
}

//*****************************************************************************
VuRetVal VuSubtractIntegersEntity::In(const VuParams &params)
{
	VuParams outParams;
	outParams.addInt(Result(params).asInt());
	mpScriptComponent->getPlug("Out")->execute(outParams);
	return VuRetVal();
}
