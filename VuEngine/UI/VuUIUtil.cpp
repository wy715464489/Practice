//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Util
// 
//*****************************************************************************

#include "VuUIUtil.h"
#include "VuEngine/Components/VuTransitionComponent.h"
#include "VuEngine/Entities/VuEntity.h"


//*****************************************************************************
void VuUIUtil::startTransitionIn(VuEntity *pEntity)
{
	if ( VuTransitionBaseComponent *pTransComp = pEntity->getComponent<VuTransitionBaseComponent>() )
		pTransComp->transitionIn();

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
		startTransitionIn(pEntity->getChildEntity(i));
}

//*****************************************************************************
void VuUIUtil::startTransitionOut(VuEntity *pEntity)
{
	if ( VuTransitionBaseComponent *pTransComp = pEntity->getComponent<VuTransitionBaseComponent>() )
		pTransComp->transitionOut();

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
		startTransitionOut(pEntity->getChildEntity(i));
}

//*****************************************************************************
bool VuUIUtil::tickTransition(VuEntity *pEntity, float fdt)
{
	bool done = true;

	if ( VuTransitionBaseComponent *pTransComp = pEntity->getComponent<VuTransitionBaseComponent>() )
	{
		pTransComp->tick(fdt);
		done &= !pTransComp->isTransitioning();
	}

	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
		done &= tickTransition(pEntity->getChildEntity(i), fdt);

	return done;
}
