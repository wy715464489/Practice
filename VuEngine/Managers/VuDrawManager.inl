//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DrawManager inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void VuDrawManager::registerHandler(T *pObj, void (T::*method)())
{
	// create draw handler
	VuMethodInterface0<void> *pHandler = new VuMethod0<T, void>(pObj, method);

	// register
	registerHandler(pHandler);
}
