//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Event map class
// 
//*****************************************************************************

#include "VuEventMap.h"
#include "VuEventManager.h"
#include "VuEngine/Util/VuHash.h"


//*****************************************************************************
VuEventMap::VuEventMap()
{
}

//*****************************************************************************
VuEventMap::~VuEventMap()
{
	unregisterHandlers();
}

//*****************************************************************************
void VuEventMap::handle(const char *name, const VuParams &params) const
{
	// generate key
	VUUINT32 key = VuHash::fnv32String(name);

	handle(key, params);
}

//*****************************************************************************
void VuEventMap::handle(VUUINT32 key, const VuParams &params) const
{
	// find handler for key
	HandlerMap::const_iterator iter = mHandlers.find(key);
	if ( iter != mHandlers.end() )
		iter->second->execute(params);
}

//*****************************************************************************
void VuEventMap::unregisterHandler(const char *name)
{
	// generate key
	VUUINT32 key = VuHash::fnv32String(name);

	unregisterHandler(key);
}

//*****************************************************************************
void VuEventMap::unregisterHandlers()
{
	for ( HandlerMap::iterator iter = mHandlers.begin(); iter != mHandlers.end(); iter++ )
	{
		VuEventManager::IF()->unregisterHandler(iter->first, iter->second);
		delete iter->second;
	}
	mHandlers.clear();
}

//*****************************************************************************
bool VuEventMap::unregisterHandler(VUUINT32 key)
{
	// find handler for key
	HandlerMap::iterator iter = mHandlers.find(key);
	if ( iter != mHandlers.end() )
	{
		VuEventManager::IF()->unregisterHandler(key, iter->second);
		delete iter->second;
		mHandlers.erase(iter);
		return true;
	}

	return false;
}

//*****************************************************************************
void VuEventMap::registerHandler(VuMethodInterface1<void, const VuParams &> *pHandler, const char *name)
{
	// generate key
	VUUINT32 key = VuHash::fnv32String(name);

	VUASSERT(mHandlers.find(key) == mHandlers.end(), "Duplicate event handler");

	// allocate and add handler
	mHandlers[key] = pHandler;

	VuEventManager::IF()->registerHandler(key, pHandler);
}
