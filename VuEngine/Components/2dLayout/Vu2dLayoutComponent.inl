//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  2dLayoutComponent inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
Vu2dLayoutComponent::Vu2dLayoutComponent(T *pOwner, void (T::*drawMethod)(bool bSelected)) : VuComponent(pOwner)
{
	mpDrawMethod = new VuMethod1<T, void, bool>(pOwner, drawMethod);
}
