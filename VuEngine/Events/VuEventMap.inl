//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Event Map inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void VuEventMap::registerHandler(T *pObj, const char *name, void (T::*method)(const VuParams &params))
{
	// create event handler
	VuMethodInterface1<void, const VuParams &> *pHandler = new VuMethod1<T, void, const VuParams &>(pObj, method);

	// register
	registerHandler(pHandler, name);
}
