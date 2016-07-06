//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EntityRepository class
// 
//*****************************************************************************

#include "VuEventManager.h"
#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Util/VuHash.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuEventManager, VuEventManager);


//*****************************************************************************
bool VuEventManager::init()
{
	VuTickManager::IF()->registerHandler(this, &VuEventManager::tick, "Decision");

	mCriticalSection = VuThread::IF()->createCriticalSection();

	return true;
}

//*****************************************************************************
void VuEventManager::release()
{
	// make sure that all event maps have been released
	for ( HandlerMap::iterator iter = mHandlerMap.begin(); iter != mHandlerMap.end(); iter++ )
		VUASSERT(iter->second.size() == 0, "VuEventManager::~VuEventManager() handler leak" );

	VuTickManager::IF()->unregisterHandlers(this);

	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuEventManager::broadcast(const char *name, const VuParams &params)
{
	// generate key
	VUUINT32 key = VuHash::fnv32String(name);

	broadcast(key, params);
}

//*****************************************************************************
void VuEventManager::broadcast(const VUUINT32 key, const VuParams &params)
{
	HandlerMap::iterator itHandlers = mHandlerMap.find(key);
	if ( itHandlers != mHandlerMap.end() )
	{
		Handlers &handlers = itHandlers->second;
		for ( Handlers::iterator iter = handlers.begin(); iter != handlers.end(); iter++ )
			(*iter)->execute(params);
	}
}

//*****************************************************************************
void VuEventManager::broadcastDelayed(float delay, bool realTime, const char *name, const VuParams &params)
{
	VUUINT32 key = VuHash::fnv32String(name);

	VuThread::IF()->enterCriticalSection(mCriticalSection);

	mPendingDelayedEvents.resize(mPendingDelayedEvents.size() + 1);
	DelayedEvent &delayedEvent = mPendingDelayedEvents.back();

	delayedEvent.mTimer = delay;
	delayedEvent.mRealTime = realTime;
	delayedEvent.mKey = key;
	delayedEvent.mParams = params;

	VuThread::IF()->leaveCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuEventManager::registerHandler(VUUINT32 key, Handler *pHandler)
{
	mHandlerMap[key].insert(pHandler);
}

//*****************************************************************************
void VuEventManager::unregisterHandler(VUUINT32 key, Handler *pHandler)
{
	HandlerMap::iterator itHandlers = mHandlerMap.find(key);
	if ( itHandlers != mHandlerMap.end() )
	{
		Handlers &handlers = itHandlers->second;
		Handlers::iterator iter = handlers.find(pHandler);
		if ( iter != handlers.end() )
			handlers.erase(iter);
	}
}

//*****************************************************************************
void VuEventManager::tick(float fdt)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);

	float fdtReal = VuTickManager::IF()->getRealDeltaTime();

	for ( VUUINT i = 0; i < mPendingDelayedEvents.size(); i++ )
	{
		DelayedEvent &delayedEvent = mPendingDelayedEvents[i];
		delayedEvent.mTimer -= delayedEvent.mRealTime ? fdtReal : fdt;
		if ( delayedEvent.mTimer <= 0 )
		{
			mExecuteDelayedEvents.push_back(delayedEvent);

			mPendingDelayedEvents.erase(mPendingDelayedEvents.begin() + i);
			i--;
		}
	}

	VuThread::IF()->leaveCriticalSection(mCriticalSection);

	for ( DelayedEvents::const_iterator iter = mExecuteDelayedEvents.begin(); iter != mExecuteDelayedEvents.end(); iter++ )
		broadcast(iter->mKey, iter->mParams);
	mExecuteDelayedEvents.clear();
}
