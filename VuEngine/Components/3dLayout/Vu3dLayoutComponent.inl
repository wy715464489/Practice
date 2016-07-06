//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dLayoutComponent inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void Vu3dLayoutComponent::setDrawMethod(T *pObj,void (T::*method)(const Vu3dLayoutDrawParams &params))
{
	mpDrawMethod = new VuMethod1<T, void, const Vu3dLayoutDrawParams &>(pObj, method);
}

//*****************************************************************************
template<class T>
void Vu3dLayoutComponent::setCollideMethod(T *pOwner, bool (T::*collideMethod)(const VuVector3 &, VuVector3 &))
{
	mpCollideMethod = new VuMethod2<T, bool, const VuVector3 &, VuVector3 &>(pOwner, collideMethod);
}
