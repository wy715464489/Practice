//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Ref class
// 
//*****************************************************************************

#include "VuScriptRef.h"
#include "VuScriptComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuScriptRef::VuScriptRef(const char *strName, const VuRTTI &refType, VuScriptComponent *pOwnerScriptComponent):
	mstrName(strName),
	mRefType(refType),
	mpOwnerScriptComponent(pOwnerScriptComponent),
	mpRefScriptComponent(VUNULL),
	mbTemplatedRef(false),
	mpWatcher(VUNULL)
{
}

//*****************************************************************************
VuScriptRef::~VuScriptRef()
{
	disconnect();
	delete mpWatcher;
}

//*****************************************************************************
bool VuScriptRef::isCompatibleWith(const VuEntity *pEntity) const
{
	// check if already connected
	if ( mpRefScriptComponent )
		return false;

	if ( getOwnerScriptComponent()->getOwnerEntity() == pEntity )
		return false;

	if ( !pEntity->isDerivedFrom(mRefType) )
		return false;

	return true;
}

//*****************************************************************************
void VuScriptRef::connect(VuScriptComponent &script)
{
	// check for compatibility
	if ( isCompatibleWith(script.getOwnerEntity()) )
	{
		disconnect();
		mpRefScriptComponent = &script;
		mpRefScriptComponent->addRefConnection(this);

		if ( mpWatcher )
			mpWatcher->execute();
	}
}

//*****************************************************************************
void VuScriptRef::disconnect()
{
	if ( mpRefScriptComponent )
	{
		mpRefScriptComponent->removeRefConnection(this);
		mpRefScriptComponent = VUNULL;

		if ( mpWatcher )
			mpWatcher->execute();
	}
}

//*****************************************************************************
VuEntity *VuScriptRef::getRefEntity() const
{
	if ( VuScriptComponent *pScript = getRefScript() )
		return pScript->getOwnerEntity();

	return VUNULL;
}

//*****************************************************************************
void VuScriptRef::load(const VuJsonContainer &data)
{
	const std::string &strRef = data[getName()].asString();

	// find entity
	if ( VuEntity *pEntity = getOwnerScriptComponent()->getOwnerEntity()->getRootEntity()->findEntity(strRef) )
	{
		// make sure type matches
		if ( isCompatibleWith(pEntity) )
		{
			// get script component
			if ( VuScriptComponent *pScriptComponent = pEntity->getComponent<VuScriptComponent>() )
			{
				mpRefScriptComponent = pScriptComponent;
				mpRefScriptComponent->addRefConnection(this);
			}
		}
	}
}

//*****************************************************************************
void VuScriptRef::save(VuJsonContainer &data) const
{
	if ( mpRefScriptComponent && !mbTemplatedRef )
	{
		data[getName()].putValue(mpRefScriptComponent->getOwnerEntity()->getLongName());
	}
}
