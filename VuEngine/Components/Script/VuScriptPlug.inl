//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Plug inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template <class T>
VuScriptInputPlug::VuScriptInputPlug(const char *strName, T *pObj, VuRetVal (T::*method)(const VuParams &params), VuRetVal::eType retType, const VuParamDecl &paramDecl)
:	VuScriptPlug(strName, retType, paramDecl)
{
	// create templated native method
	mpMethod = new VuMethod1<T, VuRetVal, const VuParams &>(pObj, method);
}

//*****************************************************************************
VuScriptInputPlug::~VuScriptInputPlug()
{
	delete mpMethod;
}
