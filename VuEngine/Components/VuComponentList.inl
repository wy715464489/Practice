//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ComponentList inline implementation
// 
//*****************************************************************************

#include "VuComponent.h"


template<class T>
T *VuComponentList::get() const
{
	for ( VuComponent *pComponent = mpHead; pComponent; pComponent = pComponent->mpNextComponent )
		if ( pComponent->isDerivedFrom(T::msRTTI) )
			return static_cast<T *>(pComponent);

	return VUNULL;
}
