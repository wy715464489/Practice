//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ComponentList class
// 
//*****************************************************************************

#include "VuComponentList.h"
#include "VuComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuComponentList::~VuComponentList()
{
	while ( mpHead )
	{
		VuComponent *pComponent = mpHead;
		mpHead = pComponent->mpNextComponent;
		delete pComponent;
	}
}

//*****************************************************************************
void VuComponentList::load(const VuJsonContainer &data)
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->load(data[pComponent->getType()]);
}

//*****************************************************************************
void VuComponentList::postLoad()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->postLoad();
}

//*****************************************************************************
void VuComponentList::save(VuJsonContainer &data) const
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->save(data[pComponent->getType()]);
}

//*****************************************************************************
void VuComponentList::bake()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->bake();
}

//*****************************************************************************
void VuComponentList::clearBaked()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->clearBaked();
}

//*****************************************************************************
void VuComponentList::applyTemplate()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->applyTemplate();
}

//*****************************************************************************
void VuComponentList::gameInitialize()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->gameInitialize();
}

//*****************************************************************************
void VuComponentList::gameRelease()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->gameRelease();
}

//*****************************************************************************
void VuComponentList::gameReset()
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		pComponent->gameReset();
}

//*****************************************************************************
void VuComponentList::add(VuComponent *pComponent)
{
	// for now we only support one component per type
#ifdef VUDEBUG
	for ( VuComponent *pTest = mpHead; pTest; pTest = pTest->mpNextComponent )
	{
		if ( pTest->isDerivedFrom(pComponent->getRTTI()) || pComponent->isDerivedFrom(pTest->getRTTI()) )
		{
			VUASSERT(0, "The engine currently only supports one component per type");
			return;
		}
	}
#endif

	// add to tail
	if ( mpHead == VUNULL )
	{
		mpHead = pComponent;
	}
	else
	{
		VuComponent *p = mpHead;
		while ( p->mpNextComponent )
			p = p->mpNextComponent;
		p->mpNextComponent = pComponent;
	}
}

//*****************************************************************************
int VuComponentList::getCount() const
{
	int count = 0;

	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		count++;

	return count;
}

//*****************************************************************************
VuComponent *VuComponentList::getByIndex(int index) const
{
	int count = 0;

	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
	{
		if ( count == index )
			return pComponent;
		count++;
	}

	return VUNULL;
}
