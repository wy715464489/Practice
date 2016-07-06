//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TickManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Method/VuMethod.h"

class VuEngine;

class VuTickManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTickManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init() = 0;
	virtual void	tick() = 0;

public:

	// clamps delta time to a max value
	virtual void	setMaxClockDelta(float fdt) = 0;

	// register a tick handler for a specific phase
	template<class T>
	void			registerHandler(T *pObj, void (T::*method)(float fdt), const char *strPhase);

	// unregister the handler for a specific phase
	virtual void	unregisterHandler(void *pObj, const char *strPhase) = 0;

	// unregister all handlers for this object
	virtual void	unregisterHandlers(void *pObj) = 0;

	// pause
	virtual void	pushPauseRequest() = 0;
	virtual void	popPauseRequest() = 0;
	virtual void	resetPauseRequests() = 0;
	virtual bool	isPaused() = 0;

	// real delta time
	virtual float	getRealDeltaTime() = 0;
	virtual float	getUnclampedRealDeltaTime() = 0;

	// game time (since app launch)
	virtual double	getGameTime() = 0;

private:

	virtual void	registerHandler(VuMethodInterface1<void, float> *pHandler, const char *strPhase) = 0;
};

#include "VuTickManager.inl"
