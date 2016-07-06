//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Properties class.
// 
//*****************************************************************************

#include "VuProperties.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Json/VuJsonContainer.h"



//*****************************************************************************
VuProperties::~VuProperties()
{
	clear();
}

//*****************************************************************************
void VuProperties::load(const VuFastContainer &data)
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		pProperty->load(data);
}

//*****************************************************************************
void VuProperties::load(const VuJsonContainer &data)
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		pProperty->load(data);
}

//*****************************************************************************
void VuProperties::save(VuJsonContainer &data) const
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		pProperty->save(data);
}

//*****************************************************************************
void VuProperties::updateDefaults()
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		pProperty->updateDefault();
}

//*****************************************************************************
void VuProperties::reset()
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		pProperty->reset();
}

//*****************************************************************************
VuProperty *VuProperties::add(VuProperty *pProperty)
{
	// add to tail
	if ( mpHead == VUNULL )
	{
		mpHead = pProperty;
	}
	else
	{
		VuProperty *p = mpHead;
		while ( p->mpNextProperty )
			p = p->mpNextProperty;
		p->mpNextProperty = pProperty;
	}

	return pProperty;
}

//*****************************************************************************
void VuProperties::remove(VuProperty *pProperty)
{
	if ( mpHead == pProperty )
	{
		mpHead = pProperty->mpNextProperty;
	}
	else
	{
		VuProperty *p1 = mpHead;
		VuProperty *p2 = mpHead->mpNextProperty;
		while ( p2 )
		{
			if ( p2 == pProperty )
			{
				p1->mpNextProperty = pProperty->mpNextProperty;
				break;
			}

			p1 = p2;
			p2 = p2->mpNextProperty;
		}
	}

	delete pProperty;
}

//*****************************************************************************
void VuProperties::clear()
{
	while ( mpHead )
	{
		VuProperty *pProperty = mpHead;
		mpHead = pProperty->mpNextProperty;
		delete pProperty;
	}
}
