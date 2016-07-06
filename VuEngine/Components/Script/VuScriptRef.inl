//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Ref inline implementation
// 
//*****************************************************************************

#include "VuScriptComponent.h"


//*****************************************************************************
template<class T>
T *VuScriptRef::getRefEntity() const
{
	if ( mRefType.isDerivedFrom(T::msRTTI) )
		return static_cast<T *>(getRefEntity());

	return VUNULL;
}
