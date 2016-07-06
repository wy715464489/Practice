//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawComponent inline implementation
// 
//*****************************************************************************

//*****************************************************************************
template<class T>
void Vu3dDrawComponent::setDrawMethod(T *pObj,void (T::*method)(const VuGfxDrawParams &params))
{
	mpDrawMethod = new VuMethod1<T, void, const VuGfxDrawParams &>(pObj, method);
}

//*****************************************************************************
template<class T>
void Vu3dDrawComponent::setDrawShadowMethod(T *pObj,void (T::*method)(const VuGfxDrawShadowParams &params))
{
	mpDrawShadowMethod = new VuMethod1<T, void, const VuGfxDrawShadowParams &>(pObj, method);
}

//*****************************************************************************
template<class T>
void Vu3dDrawComponent::setDrawPrefetchMethod(T *pObj,void (T::*method)())
{
	mpDrawPrefetchMethod = new VuMethod0<T, void>(pObj, method);

	if ( Vu3dDrawManager::IF() )
		Vu3dDrawManager::IF()->addPrefetchMethod(mpDrawPrefetchMethod);
}
