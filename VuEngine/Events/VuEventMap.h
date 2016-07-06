//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Event map class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Method/VuParams.h"


class VuEventMap
{
public:
	VuEventMap();
	~VuEventMap();

	// send an event
	void		handle(const char *name, const VuParams &params = VuParams()) const;
	void		handle(VUUINT32 key, const VuParams &params = VuParams()) const;

	// allows registration of an event handler
	template<class T>
	void		registerHandler(T *pObj, const char *name, void (T::*method)(const VuParams &params));

	// unregister a single handler (match object, event name, and parameter definition)
	void		unregisterHandler(const char *name);

	// unregister all handlers for a specific object (all events, all parameter definitions)
	void		unregisterHandlers();

protected:
	friend class VuEventManager;

	typedef VuMethodInterface1<void, const VuParams &>	Handler;
	typedef std::map<VUUINT32, Handler *>				HandlerMap;

	bool			unregisterHandler(VUUINT32 key);

	// non-templated internal registration of an event handler
	void			registerHandler(VuMethodInterface1<void, const VuParams &> *pHandler, const char *name);

	HandlerMap		mHandlers;
};


//*****************************************************************************
// Macro used to declare an event map.
#define DECLARE_EVENT_MAP	\
public:						\
	VuEventMap	mEventMap;	\
private:

//*****************************************************************************
#define REG_EVENT_HANDLER(classname, handler) mEventMap.registerHandler<classname>(this, #handler, &classname::handler)


#include "VuEventMap.inl"
