//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EventManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Method/VuParams.h"

class VuEventMap;


class VuEventManager : VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuEventManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();
	virtual void release();

public:
	void		broadcast(const char *name, const VuParams &params = VuParams());
	void		broadcast(const VUUINT32 key, const VuParams &params = VuParams());
	void		broadcastDelayed(float delay, bool realTime, const char *name, const VuParams &params = VuParams());

protected:
	typedef VuMethodInterface1<void, const VuParams &> Handler;

	// called by VuEventMap
	friend class VuEventMap;
	void		registerHandler(VUUINT32 key, Handler *pHandler);
	void		unregisterHandler(VUUINT32 key, Handler *pHandler);

private:
	void		tick(float fdt);

	typedef std::set<Handler *> Handlers;
	typedef std::hash_map<VUUINT32, Handlers> HandlerMap;
	struct DelayedEvent
	{
		float			mTimer;
		bool			mRealTime;
		VUUINT32		mKey;
		VuParams		mParams;
	};
	typedef std::vector<DelayedEvent> DelayedEvents;

	HandlerMap			mHandlerMap;
	DelayedEvents		mPendingDelayedEvents;
	DelayedEvents		mExecuteDelayedEvents;
	VUHANDLE			mCriticalSection;
};
