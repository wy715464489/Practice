//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TransformComponent inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void VuTransformComponent::setWatcher(void (T::*method)())
{
	mpWatcher = new VuMethod0<T, void>(static_cast<T *>(getOwnerEntity()), method);
}
