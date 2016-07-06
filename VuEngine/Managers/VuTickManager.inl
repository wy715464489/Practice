//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TickManager inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void VuTickManager::registerHandler(T *pObj, void (T::*method)(float fdt), const char *strPhase)
{
	// create tick handler
	VuMethodInterface1<void, float> *pHandler = new VuMethod1<T, void, float>(pObj, method);

	// register
	registerHandler(pHandler, strPhase);
}
