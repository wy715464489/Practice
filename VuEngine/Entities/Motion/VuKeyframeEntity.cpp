//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Keyframe entity
// 
//*****************************************************************************

#include "VuKeyframeEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


IMPLEMENT_RTTI(VuKeyframeEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuKeyframeEntity);


//*****************************************************************************
VuKeyframeEntity::VuKeyframeEntity():
	mTime(0.0f)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dLayoutComponent->setDrawMethod(this, &VuKeyframeEntity::drawLayout);

	// properties
	addProperty(new VuFloatProperty("Time", mTime));
}

//*****************************************************************************
void VuKeyframeEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
		if ( VuEntity *pParent = getParentEntity() )
			if ( Vu3dLayoutComponent *pParent3dLayoutComponent = pParent->getComponent<Vu3dLayoutComponent>() )
				pParent3dLayoutComponent->draw(params);
}
