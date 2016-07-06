//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuComponent class
// 
//*****************************************************************************

#include "VuComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"


IMPLEMENT_RTTI_BASE(VuComponent);


//*****************************************************************************
void VuComponent::load(const VuJsonContainer &data)
{
	mProperties.load(data["Properties"]);

	onLoad(data);
}

//*****************************************************************************
void VuComponent::postLoad()
{
	onPostLoad();
}

//*****************************************************************************
void VuComponent::save(VuJsonContainer &data) const
{
	if ( mProperties.hasProperties() )
		mProperties.save(data["Properties"]);

	onSave(data);
}

//*****************************************************************************
void VuComponent::bake()
{
	onBake();
}

//*****************************************************************************
void VuComponent::clearBaked()
{
	onClearBaked();
}

//*****************************************************************************
void VuComponent::applyTemplate()
{
	mProperties.updateDefaults();

	onApplyTemplate();
}

//*****************************************************************************
void VuComponent::gameInitialize()
{
	onGameInitialize();
}

//*****************************************************************************
void VuComponent::gameRelease()
{
	onGameRelease();
}

//*****************************************************************************
void VuComponent::gameReset()
{
	mProperties.reset();

	onGameReset();
}
