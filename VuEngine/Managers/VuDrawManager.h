//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DrawManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Method/VuMethod.h"

class VuEngine;
class VuRect;

class VuDrawManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuDrawManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init() = 0;
	virtual void	draw() = 0;

public:

	// register/unregister draw handlers
	template<class T> void	registerHandler(T *pObj, void (T::*method)());
	virtual           void	unregisterHandler(void *pObj) = 0;

private:

	virtual void	registerHandler(VuMethodInterface0<void> *pHandler) = 0;
};

#include "VuDrawManager.inl"
