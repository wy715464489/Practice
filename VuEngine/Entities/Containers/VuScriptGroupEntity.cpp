//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Group Entity.
// 
//*****************************************************************************

#include "VuScriptGroupEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


//*****************************************************************************
// Script Group
//*****************************************************************************

IMPLEMENT_RTTI(VuScriptGroupEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuScriptGroupEntity);

//*****************************************************************************
VuScriptGroupEntity::VuScriptGroupEntity():
	VuEntity(CAN_HAVE_CHILDREN),
	mCollapsed(false),
	mWidth(250),
	mDepth(0)
{
	// properties
	addProperty(new VuBoolProperty("Collapsed", mCollapsed));
	addProperty(new VuIntProperty("Width", mWidth));
}

//*****************************************************************************
VuVector2 VuScriptGroupEntity::getPosition() const
{
	VuVector2 posMin(FLT_MAX, FLT_MAX), posMax(-FLT_MAX, -FLT_MAX);
	if ( getBoundsRecursive(this, posMin, posMax) )
		return 0.5f*(posMin + posMax);

	return VuVector2(0,0);
}

//*****************************************************************************
void VuScriptGroupEntity::setPosition(const VuVector2 &pos)
{
	moveRecursive(this, pos - getPosition());
}

//*****************************************************************************
void VuScriptGroupEntity::moveRecursive(const VuEntity *pEntity, const VuVector2 &delta) const
{
	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( VuScriptComponent *pScriptComponent = pChild->getComponent<VuScriptComponent>() )
			pScriptComponent->setPosition(pScriptComponent->getPosition() + delta);

		moveRecursive(pChild, delta);
	}
}

//*****************************************************************************
int VuScriptGroupEntity::countPlugsRecursive(const VuEntity *pEntity) const
{
	int count  = 0;

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( pChild->isDerivedFrom(VuScriptGroupConnectionEntity::msRTTI) )
			count++;
		else
			count += countPlugsRecursive(pChild);
	}

	return count;
}

//*****************************************************************************
VuScriptPlug *VuScriptGroupEntity::getPlugRecursive(const VuEntity *pEntity, int &index) const
{
	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( pChild->isDerivedFrom(VuScriptGroupConnectionEntity::msRTTI) )
		{
			if ( index == 0 )
			{
				if ( pChild->isDerivedFrom(VuScriptGroupInputEntity::msRTTI) )
					return static_cast<VuScriptGroupInputEntity *>(pChild)->getInputPlug();
				else if ( pChild->isDerivedFrom(VuScriptGroupOutputEntity::msRTTI) )
					return static_cast<VuScriptGroupOutputEntity *>(pChild)->getOutputPlug();
			}
			index--;
		}
		else
		{
			if ( VuScriptPlug *pPlug = getPlugRecursive(pChild, index) )
				return pPlug;
		}
	}

	return VUNULL;
}

//*****************************************************************************
int VuScriptGroupEntity::countNumPlugsOfTypeRecursive(const VuEntity *pEntity, bool bInput) const
{
	int count  = 0;

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( bInput && pChild->isDerivedFrom(VuScriptGroupInputEntity::msRTTI) )
			count++;
		else if ( !bInput && pChild->isDerivedFrom(VuScriptGroupOutputEntity::msRTTI) )
			count++;
		else
			count += countNumPlugsOfTypeRecursive(pChild, bInput);
	}

	return count;
}

//*****************************************************************************
bool VuScriptGroupEntity::getBoundsRecursive(const VuEntity *pEntity, VuVector2 &posMin, VuVector2 &posMax) const
{
	bool valid = false;

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( VuScriptComponent *pScriptComponent = pChild->getComponent<VuScriptComponent>() )
		{
			posMin = VuMin(posMin, pScriptComponent->getPosition());
			posMax = VuMax(posMax, pScriptComponent->getPosition());
			valid = true;
		}

		valid |= getBoundsRecursive(pChild, posMin, posMax);
	}

	return valid;
}


//*****************************************************************************
// Script Group Connection
//*****************************************************************************

IMPLEMENT_RTTI(VuScriptGroupConnectionEntity, VuEntity);

//*****************************************************************************
VuScriptGroupConnectionEntity::VuScriptGroupConnectionEntity()
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	mpInputPlug = ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuScriptGroupInputEntity, In);
	mpOutputPlug = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Out);
}

//*****************************************************************************
VuRetVal VuScriptGroupConnectionEntity::In(const VuParams &params)
{
	return mpScriptComponent->getPlug("Out")->execute(params);
}


//*****************************************************************************
// Script Group Input
//*****************************************************************************

IMPLEMENT_RTTI(VuScriptGroupInputEntity, VuScriptGroupConnectionEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuScriptGroupInputEntity);


//*****************************************************************************
// Script Group Output
//*****************************************************************************

IMPLEMENT_RTTI(VuScriptGroupOutputEntity, VuScriptGroupConnectionEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuScriptGroupOutputEntity);

